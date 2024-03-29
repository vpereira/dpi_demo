/* This program is covered by the license described in LICENSE.TXT
 * Copyright, 2007-2009, NetLogic Microsystems, Inc. */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/capability.h>
#include <linux/pid.h>
#include <linux/sort.h>
#include <linux/compat.h>

#include <asm/uaccess.h>

#include "netl7driver.h"

#define DRV_NAME "NetL7_V2.2"

/* PARAMETERS
   Please see comments in the README and device_driver_instructions.txt file
   for more information about using the driver with fixed address */
static ulong fixed_phys_base = 0;
static ulong fixed_phys_size = 0;
static ulong dmamem_size = 0;
static int   verbose = 0;

module_param (fixed_phys_base, ulong, S_IRUGO);
module_param (fixed_phys_size, ulong, S_IRUGO);
module_param (dmamem_size, ulong, S_IRUGO);
module_param (verbose, int, S_IRUGO);

#define NLM_INFO(f, args...) printk (KERN_INFO DRV_NAME " "f,##args)
#define NLM_VERB(f, args...) do { if (verbose) printk (KERN_INFO DRV_NAME " "f, ##args); } while (0)

#define NLM_PCI_VENDOR_ID (0x184e)
#define NLM_PCI_DEV_ID (0x8)

enum nlm_device_type
{
  NLM_DEVICE_UNKNOWN = 0,
  NLM_DEVICE_MARS1,
  NLM_DEVICE_MARS2,
  NLM_DEVICE_FAMOS,
  NLM_DEVICE_XAUIEX2,
  NLM_DEVICE_MARS3
};

static const char *
nlm_device_type_name[] = 
{
  "UNKNOWN",
  "MARS1",
  "MARS2",
  "FAMOS",
  "XAUI EX2",
  "MARS3"
};
  
/* Describes a single NetL7 device on the system. For FAMOS
   type devices, describes each ring */
struct nlm_device 
{
  enum nlm_device_type type;
  pid_t owner_pid;
  struct proc_dir_entry *proc_entry;
  unsigned long long sysmem_base;    /* chunk of system memory for this device */
  unsigned long long packetmem_base; /* chunk of packet memory for this device */
  unsigned long long regmap_base;
  u32 regmap_size;
  u32 sysmem_size;
  u32 packetmem_size;
  spinlock_t lock;
};

/* A list of all the devices discovered on the system 
   we support upto 16 devices. */
#define MAX_DEVICES 16
struct nlm_device device_list[MAX_DEVICES];
static u32 n_devices;

static struct proc_dir_entry *nlm_proc_root;

static int create_proc_entries (void);
static void destroy_proc_entries (void);
static int memory_manager_init (void);
static void memory_manager_fini (void);

#ifdef KERNEL_DEMO
#include "./kerneldemo.c"
#endif

/* PCI SECTION */
static int
nlm_pci_probe (struct pci_dev *nlm_dev, const struct pci_device_id *nlm_id)
{
  u16 vendor_id, device_id;
  u32  dev_id;
  enum nlm_device_type type;
  
  if (n_devices >= MAX_DEVICES)
    {
      NLM_INFO (": Exceeded max devices that driver can support\n");
      return -EINVAL;
    }
  
  if (nlm_dev == NULL)
    {
      NLM_INFO (": Probe called with null dev ptr\n");
      return -EINVAL;
    }

  /* Enable the device before any operations */
  if (pci_enable_device (nlm_dev) != 0)
    {
      NLM_INFO (": Cannot enable device (%p)\n", nlm_dev);
      return -EINVAL;
    }
  
  pci_read_config_word (nlm_dev, PCI_VENDOR_ID, &vendor_id);
  pci_read_config_word (nlm_dev, PCI_DEVICE_ID, &device_id);
  
  switch (nlm_id->device)
    {
    case NLM_PCI_DEV_ID:
      /* mars 1 */
      type = NLM_DEVICE_MARS1;
      break;
    case NLM_PCI_DEV_ID + 1:
      /* mars 2 */
      type = NLM_DEVICE_MARS2;
      break;
    case NLM_PCI_DEV_ID + 2:
      /* Famos */
      type = NLM_DEVICE_FAMOS;
      break;
    case NLM_PCI_DEV_ID + 3:
      type = NLM_DEVICE_XAUIEX2;
      break;
    case NLM_PCI_DEV_ID + 4:
      type = NLM_DEVICE_MARS3;
      break;
    default:
      NLM_INFO (": Unexpected device ID %d\n", nlm_id->device);
      return -EINVAL;
    }
    
  dev_id = n_devices++;
  device_list[dev_id].type = type;
  device_list[dev_id].regmap_base = pci_resource_start (nlm_dev, 0);
  device_list[dev_id].regmap_size = pci_resource_len (nlm_dev, 0);
  device_list[dev_id].proc_entry = NULL;
  spin_lock_init (&device_list[dev_id].lock);

  if (type == NLM_DEVICE_FAMOS)
    {
      /* Total 6 rings on FAMOS, so occupy 5 more slots */
      int i;
      u64 regmap_base = device_list[dev_id].regmap_base;
      u32 regmap_size = device_list[dev_id].regmap_size;

      if ((n_devices + 5) >= MAX_DEVICES)
        {
          NLM_INFO (": Exceeded max devices support in driver. Cannot accomodate FAMOS rings\n");
          return -EINVAL;
        }
        
      for (i = 0; i < 5; i++)
        {
          dev_id = n_devices++;
          device_list[dev_id].type = type;
          device_list[dev_id].regmap_base = regmap_base;
          device_list[dev_id].regmap_size = regmap_size;
          device_list[dev_id].proc_entry = NULL;
          spin_lock_init (&device_list[dev_id].lock);
        }
    }
  else if  (type == NLM_DEVICE_XAUIEX2)
    {
      /* xaui exercser 2 takes up one more slot. One slot
         for each xaui ring. Ideally it looks exactly like
         famos, however, we would like to not reserve
         memory for the unused rings */
      u64 regmap_base = device_list[dev_id].regmap_base;
      u32 regmap_size = device_list[dev_id].regmap_size;

      if ((n_devices + 1) >= MAX_DEVICES)
        {
          NLM_INFO (": Exceeded max devices support in driver. Cannot accomodate FAMOS rings\n");
          return -EINVAL;
        }
        
      dev_id = n_devices++;
      device_list[dev_id].type = type;
      device_list[dev_id].regmap_base = regmap_base;
      device_list[dev_id].regmap_size = regmap_size;
      device_list[dev_id].proc_entry = NULL;
      spin_lock_init (&device_list[dev_id].lock);
    }
  
  /* enable bus mastering */
  pci_set_master (nlm_dev);
  
  return 0;
}

static const struct __devinitdata pci_device_id netl7_pci_ids[] = 
{
  {PCI_DEVICE (NLM_PCI_VENDOR_ID, NLM_PCI_DEV_ID)},
  {PCI_DEVICE (NLM_PCI_VENDOR_ID, NLM_PCI_DEV_ID + 1)},
  {PCI_DEVICE (NLM_PCI_VENDOR_ID, NLM_PCI_DEV_ID + 2)},
  {PCI_DEVICE (NLM_PCI_VENDOR_ID, NLM_PCI_DEV_ID + 3)},
  {PCI_DEVICE (NLM_PCI_VENDOR_ID, NLM_PCI_DEV_ID + 4)},
  {0,},
};

MODULE_DEVICE_TABLE (pci, netl7_pci_ids);

static struct pci_driver pci_driver_funcs =
{
  .name = DRV_NAME "_pci",
  .id_table = netl7_pci_ids,
  .probe = nlm_pci_probe,
  .driver = {
    .name = DRV_NAME "_device",
  }
};

static int __init
nlm_drv_module_init (void)
{
  int status;

  NLM_INFO (": driver load process started.\n");
  status = pci_register_driver (&pci_driver_funcs);
  if (status < 0)
    {
      NLM_INFO (": Unable to register PCI driver! (%d)\n", status);
      return -EINVAL;
    }
  
  if (n_devices == 0)
    {
      NLM_INFO (": probing finished, no NetL7 devices discovered\n");
      return 0;
    }
  
  status = memory_manager_init ();
  if (status < 0)
    {
      NLM_INFO (": Memory allocation failed\n");
      return status;
    }
  
  status = create_proc_entries ();
  if (status < 0)
    {
      NLM_INFO (": Proc entry creation failed\n");
      return status;
    }

  {
    /* Print debug info */
    int i;
    for (i = 0; i < n_devices; i++)
      {
        NLM_INFO (": === Probed device no %d (%p) (%s) === \n",
                  i, &device_list[i], nlm_device_type_name[device_list[i].type]);
        NLM_INFO (":     proc_entry /proc/NetL7/%s\n",
                  device_list[i].proc_entry->name);
        NLM_INFO (":     register_map base = 0x%llx size = 0x%x\n",
                  device_list[i].regmap_base, device_list[i].regmap_size);
        NLM_INFO (":     system memory base = 0x%llx size = 0x%x\n",
                  device_list[i].sysmem_base, device_list[i].sysmem_size);
        NLM_INFO (":     packet memory base = 0x%llx size = 0x%x\n",
                  device_list[i].packetmem_base, device_list[i].packetmem_size);
      }
  }

#ifdef KERNEL_DEMO
  {
      int i;

      /* Do some kernel demo here */
      for (i = 0; i < n_devices ; i++)
        TRY (run_abcd_on_device (i));

      TRY (run_abcd_on_all (1));
  }
#endif
  
  return 0;
}

static void __exit
nlm_drv_module_exit (void)
{
  destroy_proc_entries ();
  memory_manager_fini ();
  pci_unregister_driver (&pci_driver_funcs);
  NLM_INFO (": Unloaded.\n");
}

static inline ulong
get_size_from_order (int order)
{
  return PAGE_SIZE * (1 << order);
}

/* /proc handling section */
static int
nlm_device_info_show (struct seq_file *m, void *v)
{
  struct nlm_device *dev;
  pid_t owner;
  dev = (struct nlm_device *) m->private;

  if (dev == NULL || m == NULL)
    return -EINVAL;
  
  /* We are looking at the owner field, without any form of
     lock. There is a possibility that the owner may be inlocking
     the device as we are doing this. However, this routine is just
     an instant snapshot, and we do not de-reference any memory that
     is changing, so the worst case is data is stale */
  owner = dev->owner_pid;
  seq_printf (m, "Device Type  : %s\n", nlm_device_type_name[dev->type]);
  if (owner == 0)
    seq_printf (m, "       Status: Available\n");
  else
    seq_printf (m, "       Status: Busy, locked by PID %d\n", owner);

  seq_printf (m, "       config.register_map_base_phys = 0x%llx\n", dev->regmap_base);
  seq_printf (m, "       config.register_map_size = %d\n", dev->regmap_size);
  seq_printf (m, "The above variables can be directly plugged into config file\n");
  seq_printf (m, "ioctl the /proc file from your process to obtain the virtual\n");
  seq_printf (m, "address of the register map\n");

  if (dev->sysmem_base == 0)
    {
      seq_printf (m, "Driver could not allocate any memory for this device\n");
    }
  else
    {
      seq_printf (m, "=======================================================\n");
      seq_printf (m, "Driver Assigned memory\n");
      seq_printf (m, "=======================================================\n");
      
      seq_printf (m, "System Memory: 0x%016llx  |  size: 0x%08x\n", dev->sysmem_base, dev->sysmem_size);
      seq_printf (m, "Packet Memory: 0x%016llx  |  size: 0x%08x\n", dev->packetmem_base, dev->packetmem_size);
      seq_printf (m, "\n");
    }

  return 0;
}
  
static int
nlm_device_open (struct inode *inode, struct file *file)
{
  struct nlm_device *dev = PDE (inode)->data;
  
  if (dev == NULL)
    return -EINVAL;
  
  if (file->f_mode & FMODE_WRITE)
    {
      /* Only when writing to the device (ioctl) set the
         current process as the owner. This will create 
         exclusive access to the device for the user process.
         
         For cat on the /proc device, just let them happen */
      unsigned long flags;
      /*NLM_INFO (":open The file reference count is %ld\n", file->f_count.counter);*/
      spin_lock_irqsave (&dev->lock, flags);
      if (dev->owner_pid == 0)
        dev->owner_pid = current->pid;
      else if (dev->owner_pid == current->pid)
        {
          /* recursive */
        }
      else
        {
          spin_unlock_irqrestore (&dev->lock, flags);
          return -EBUSY;
        }
      file->private_data = dev;
      spin_unlock_irqrestore (&dev->lock, flags);
      
      /* Note! we cannot use single open, when the
         file has been opened with write, as we
         use the private_data field in the file
         descriptor field for ioctl reference.
         
         If you single_open or seq_open you will
         die! */
      return 0;
    }

  return single_open (file, nlm_device_info_show, dev);
}

/* In the ioctl routine we will alread have exclusive access
   to the proc file. Multiple writers are blocked out in
   the file open call */
static long
nlm_device_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
  struct nlm_device *dev = file->private_data;
  nlm_sys_cfg_t *buf = (nlm_sys_cfg_t *) arg;
  
  if (dev == NULL)
    {
      NLM_VERB (": ioctl called by pid %d failed, device is null\n", current->pid);
      return -EINVAL;
    }

  /*NLM_INFO (":ioctl The file reference count is %ld\n", file->f_count.counter);*/

  switch (_IOC_TYPE(cmd))
    {  
    case NLM_IOCTL_GET_REGMEM:
      if (put_user (dev->regmap_size, &buf->mem_size))
        return -EFAULT;
      if (put_user (dev->regmap_base, &buf->physical_addr))
        return -EFAULT;
      NLM_VERB (": ioctl regmem for pid %d, addr 0x%llx, size 0x%x\n",
                current->pid, dev->regmap_base, dev->regmap_size);
      break;
      
    case NLM_IOCTL_GET_DMAMEM:
      if (put_user (dev->packetmem_base, &buf->physical_addr))
        return -EFAULT;
      if (put_user (dev->packetmem_size, &buf->mem_size))
        return -EFAULT;
      NLM_VERB (": ioctl packet memory for pid %d, addr 0x%llx, size 0x%x\n",
                current->pid, dev->packetmem_base, dev->packetmem_size);
      break;
      
    case NLM_IOCTL_GET_SYSMEM:
      if (put_user (dev->sysmem_base, &buf->physical_addr))
        return -EFAULT;
      if (put_user (dev->sysmem_size, &buf->mem_size))
        return -EFAULT;
      NLM_VERB (": ioctl system memory for pid %d, addr 0x%llx, size 0x%x\n",
                current->pid, dev->sysmem_base, dev->sysmem_size);
      break;
      
    default:
      NLM_VERB (": ioctl code incorrect on %s by pid %d, ioctl=%d\n",
                dev->proc_entry->name, current->pid, _IOC_TYPE(cmd));
      return -EFAULT;
    }

  return 0;
}

#ifdef CONFIG_COMPAT
static long
nlm_device_compat_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
#ifdef __x86_64
  /* Padding & structure size difference on x64 */
  typedef struct compat_nlm_sys_cfg_t
  {
    unsigned int mem_size;
    compat_u64 physical_addr;
  } compat_nlm_sys_cfg_t;

  struct nlm_device *dev = file->private_data;
  compat_nlm_sys_cfg_t *buf = (compat_nlm_sys_cfg_t *) arg;
  
  if (dev == NULL)
    {
      NLM_VERB (": compat ioctl called by pid %d failed, device is null\n", current->pid);
      return -EINVAL;
    }

  /*NLM_INFO (":ioctl The file reference count is %ld\n", file->f_count.counter);*/

  switch (_IOC_TYPE(cmd))
    {  
    case NLM_IOCTL_GET_REGMEM:
      if (put_user (dev->regmap_size, &buf->mem_size))
        return -EFAULT;
      if (put_user (dev->regmap_base, &buf->physical_addr))
        return -EFAULT;
      NLM_VERB (": compat ioctl regmem for pid %d, addr 0x%llx, size 0x%x\n",
                current->pid, dev->regmap_base, dev->regmap_size);
      break;
      
    case NLM_IOCTL_GET_DMAMEM:
      if (put_user (dev->packetmem_base, &buf->physical_addr))
        return -EFAULT;
      if (put_user (dev->packetmem_size, &buf->mem_size))
        return -EFAULT;
      NLM_VERB (": compat ioctl packet memory for pid %d, addr 0x%llx, size 0x%x\n",
                current->pid, dev->packetmem_base, dev->packetmem_size);
      break;
      
    case NLM_IOCTL_GET_SYSMEM:
      if (put_user (dev->sysmem_base, &buf->physical_addr))
        return -EFAULT;
      if (put_user (dev->sysmem_size, &buf->mem_size))
        return -EFAULT;
      NLM_VERB (": compat ioctl system memory for pid %d, addr 0x%llx, size 0x%x\n",
                current->pid, dev->sysmem_base, dev->sysmem_size);
      break;
      
    default:
      NLM_VERB (": compat ioctl code incorrect on %s by pid %d, ioctl=%d\n",
                dev->proc_entry->name, current->pid, _IOC_TYPE(cmd));
      return -EFAULT;
    }

  return 0;
#else
  /* On remaining systems, go through regular ioctl */
  return nlm_device_ioctl (file, cmd, arg);
#endif
}
#endif

/* Remap the physical address returned to the user through
   ioctl call above to a user visible virtual address */
static int
nlm_device_mmap (struct file *file_p, struct vm_area_struct *vm_p)
{
  int i, cacheable;
  ulong phy_addr;

  if (vm_p->vm_flags & VM_LOCKED)
    return -EPERM;
  
  /*NLM_INFO (":mmap The file reference count is %ld\n", file_p->f_count.counter);*/

  vm_p->vm_flags |= VM_RESERVED;
  vm_p->vm_flags |= VM_IO;
  cacheable = 1;
  
  phy_addr = vm_p->vm_pgoff << PAGE_SHIFT;

#if defined (__arm__)
  /* On ARM all system memory is non cacheable */
  vm_p->vm_page_prot = pgprot_noncached (vm_p->vm_page_prot);
  cacheable = 0;
#else
  for (i = 0; i < n_devices; i++)
    {
      if (device_list[i].regmap_base == phy_addr)
        {
          vm_p->vm_page_prot = pgprot_noncached (vm_p->vm_page_prot);
          cacheable = 0;
          break;
        }
    }
#endif   

  NLM_VERB (": mmap called start 0x%lx, end 0x%lx, pgoff 0x%lx, shifted 0x%lx, cacheable %d\n",
            vm_p->vm_start, vm_p->vm_end, phy_addr, phy_addr >> PAGE_SHIFT, cacheable);

  if (remap_pfn_range (vm_p, vm_p->vm_start, phy_addr >> PAGE_SHIFT,
                       vm_p->vm_end - vm_p->vm_start, vm_p->vm_page_prot))
    {
      NLM_INFO (": remap_pfn_range failed.\n");
      return -EFAULT;
    }

  return 0;
}

static int
nlm_device_close (struct inode *inode, struct file *file)
{
  struct nlm_device *dev = PDE (inode)->data;
  
  if (dev == NULL)
    return -EINVAL;
  
  /*NLM_INFO (":close The file reference count is %ld\n", file->f_count.counter);*/

  if (file->f_mode & FMODE_WRITE)
    {
      unsigned long flags;
      spin_lock_irqsave (&dev->lock, flags);
      dev->owner_pid = 0;
      spin_unlock_irqrestore (&dev->lock, flags);
      return 0;
    }

  return single_release (inode, file);
}

/* Operations on /proc/NetL7/{mars1.*, mars2.*, famos.*} */
static const struct file_operations device_proc_fops = {
  .owner          = THIS_MODULE,
  .open           = nlm_device_open,
  .read           = seq_read,
  .llseek         = seq_lseek,
  .release        = nlm_device_close,
  .unlocked_ioctl = nlm_device_ioctl,
#ifdef CONFIG_COMPAT
  .compat_ioctl   = nlm_device_compat_ioctl,
#endif
  .mmap           = nlm_device_mmap 
};

static int
create_proc_entries ()
{
#define BUF_SIZE 64
  char buf[BUF_SIZE];
  int i, j, failure, mars1_id, mars2_id, mars3_id, famos_id, xauiex2_id;
  struct proc_dir_entry *entry;
  
  /* Lets create the top level /proc/NetL7 directory */
  nlm_proc_root = proc_mkdir ("NetL7", NULL);
  if (nlm_proc_root == NULL)
    {
      NLM_INFO (": Creating top level /proc/NetL7 directory failed\n");
      return -EINVAL;
    }
  
  /* We now create one entry for each device. */
  failure = 0;
  mars1_id = 0;
  mars2_id = 0;
  mars3_id = 0;
  famos_id = 0;
  xauiex2_id = 0;
  
  for (i = 0; i < n_devices; i++)
    {
      switch (device_list[i].type)
        {
        case NLM_DEVICE_MARS1:
          if (snprintf (buf, sizeof (buf), "mars1.%01d", mars1_id) < 0)
            {
              failure = 1;
              break;
            }
          mars1_id++;
          entry = create_proc_entry (buf, S_IFREG | S_IRUGO | S_IWUGO, 
                                     nlm_proc_root);
          if (entry == NULL)
            {
              failure = 1;
              break;
            }

          entry->proc_fops = &device_proc_fops;
          entry->data = &device_list[i];
          device_list[i].proc_entry = entry;
          break;
          
        case NLM_DEVICE_MARS2:
          if (snprintf (buf, sizeof (buf), "mars2.%01d", mars2_id) < 0)
            {
              failure = 1;
              break;
            }
          mars2_id++;
          entry = create_proc_entry (buf, S_IFREG | S_IRUGO | S_IWUGO, 
                                     nlm_proc_root);
          if (entry == NULL)
            {
              failure = 1;
              break;
            }

          entry->proc_fops = &device_proc_fops;
          entry->data = &device_list[i];
          device_list[i].proc_entry = entry;
          break;

        case NLM_DEVICE_FAMOS:
          /* FAMOS has 4 PCIE rings & 2 xaui rings,
             create explicit entries */
          for (j = 0; j < 6; j++)
            {
              if (snprintf (buf, sizeof (buf), "fms.%01d.r%01d", famos_id, j) < 0)
                {
                  failure = 1;
                  break;
                }
              entry = create_proc_entry (buf, S_IFREG | S_IRUGO | S_IWUGO, 
                                         nlm_proc_root);
              if (entry == NULL)
                {
                  failure = 1;
                  break;
                }

              entry->proc_fops = &device_proc_fops;
              entry->data = &device_list[i];
              device_list[i].proc_entry = entry;
              i++;
            }
          famos_id++;
          i--; /* i will be incremented again in for loop */
          break;
          
        case NLM_DEVICE_XAUIEX2:
          /* Has 2 rings */
          for (j = 0; j < 2; j++)
            {
              if (snprintf (buf, sizeof (buf), "xauiex2.%01d.r%01d", xauiex2_id, j) < 0)
                {
                  failure = 1;
                  break;
                }
              entry = create_proc_entry (buf, S_IFREG | S_IRUGO | S_IWUGO, 
                                         nlm_proc_root);
              if (entry == NULL)
                {
                  failure = 1;
                  break;
                }

              entry->proc_fops = &device_proc_fops;
              entry->data = &device_list[i];
              device_list[i].proc_entry = entry;
              i++;
            }
          xauiex2_id++;
          i--; /* i will be incremented again in for loop */
          break;
          
        case NLM_DEVICE_MARS3:
          if (snprintf (buf, sizeof (buf), "mars3.%01d", mars3_id) < 0)
            {
              failure = 1;
              break;
            }
          mars3_id++;
          entry = create_proc_entry (buf, S_IFREG | S_IRUGO | S_IWUGO, 
                                     nlm_proc_root);
          if (entry == NULL)
            {
              failure = 1;
              break;
            }

          entry->proc_fops = &device_proc_fops;
          entry->data = &device_list[i];
          device_list[i].proc_entry = entry;
          break;
        default:
          NLM_INFO (": Unexpected device ID during /proc creation\n");
        }
      
      if (failure)
        break;
    }
  
  if (failure)
    {
      destroy_proc_entries ();
      return -EINVAL;
    }

  return 0;
}

static void
destroy_proc_entries ()
{
  int i;
  struct proc_dir_entry *entry;
  NLM_INFO (": Destroy proc entries number of devices = %d\n",
            n_devices);
  
  for (i = 0; i < n_devices; i++)
    {
      if (!device_list[i].proc_entry)
        break;
      entry = device_list[i].proc_entry;
      device_list[i].proc_entry = NULL;
      NLM_INFO (": Going to destroy /proc/NetL7/%s\n",
                entry->name);
      remove_proc_entry (entry->name, nlm_proc_root);
    }
  
  /* Remove root directory */
  remove_proc_entry ("NetL7", NULL);
}

/* If dmamem_size is not specified this is default value */
#define NLM_DEFAULT_DMA_MEM_SIZE (2 * 1024 * 1024)

/* Any memory chunk below this size is not interesting */
#define NLM_MIN_ALLOC_SIZE (2 * 1024 * 1024)

/* The size of the scratch page where allocated page addresses
   are saved */
#define NLM_SCRATCH_PAGE_SIZE (2 * 1024 * 1024)

/* The default size of dynamic chunk we will look for */
#define NLM_DEFAULT_PHYS_MEM_SIZE (256 * 1024 * 1024)

/* break up the provided memory region and assign to system
   memory and dma memory for each device */
static int
fixed_memory_chunking (ulong phys_base, ulong phys_size, ulong dma_size)
{
  ulong sysmem_size, base;
  int i;
  
  if (phys_base == 0
      || phys_size == 0)
    return -EINVAL;

  /* Lets try 2 Mb for packet memory, if zero is passed*/
  if (dma_size == 0)
    dma_size = NLM_DEFAULT_DMA_MEM_SIZE;

  /* Align everything to page boundary. This is required, as freeing memory
     during dynamic memory allocation becomes easier */
  base = (phys_base + PAGE_SIZE - 1) & PAGE_MASK;
  if (base != phys_base)
    phys_size -= base - phys_base;

  sysmem_size = phys_size / n_devices;
  sysmem_size &= PAGE_MASK;
  dma_size &= PAGE_MASK;

  if (dma_size > sysmem_size)
    {
      NLM_INFO (": error packet memory requested %ld larger than fixed memory size per device %ld\n",
                dma_size, sysmem_size);
      return -ENOMEM;
    }
  
  sysmem_size -= dma_size;
  NLM_VERB ("Carving up system memory at 0x%lx, into sysmem %ld, dmamem %ld\n",
            base, sysmem_size, dma_size);
  
  /* The per device chunk is further fragmented into
     system memory and packet memory */
  for (i = 0; i < n_devices; i++)
    {
      /* system memory */
      device_list[i].sysmem_base = base;
      device_list[i].sysmem_size = sysmem_size;
      base += sysmem_size;
    }
    
  /* Create the packet memory free list */
  for (i = 0; i < n_devices; i++)
    {
      /* packet memory */
      device_list[i].packetmem_base = base;
      device_list[i].packetmem_size = dma_size;
      base += dma_size;
    }
  
  if (base > (phys_base + phys_size))
    {
      NLM_INFO (":Fixed memory chunking logic error, top after chunking 0x%ld, actual 0x%ld\n",
                base, (phys_base + phys_size));
      return -EFAULT;
    }

  return 0;
}

struct mem_desc
{
  struct mem_desc *next;
  int npages;
};

/* Insert the element into the existing list_head
   and return a new list_head pointer */
static struct mem_desc *
insert_into_sorted_list (struct mem_desc *list_head,
                         struct mem_desc *element)
{
  struct mem_desc *prev, *cur;
  
  if (!list_head)
    return element;
  
  prev = NULL;
  cur = list_head;
  while (cur)
    {
      if (element->npages > cur->npages)
        {
          if (prev)
            {
              prev->next = element;
              element->next = cur;
              return list_head;
            }
          else
            {
              element->next = cur;
              return element;
            }
        }
      prev = cur;
      cur = cur->next;
    }

  prev->next = element;
  element->next = NULL;
  return list_head;
}

static int
compare_addresses (const void *a, const void *b)
{
  return *(long *) a - *(long *) b;
}

/* The list is a virtual address pointer, however,
   the range of addresses are physically contigious.
   Make sure we convert the pages to virtual
   first before we free them */
static void
release_chunks (struct mem_desc *list)
{
  while (list)
    {
      struct mem_desc *temp = list->next;
      int i, max;
      ulong page;
      
      /*NLM_VERB (": Freeing chunk 0x%lx, npages %d \n", (ulong)virt_to_phys ((void *)list), list->npages);*/
      max = list->npages;
      page = (ulong) virt_to_phys(list);
      for (i = 0; i < max; i++)
        {
          free_page ((ulong)phys_to_virt(page));
          page += PAGE_SIZE;
        }

      list = temp;
    }
}

/* The algorithm is pretty simple

   - grab a 2Mb page and use it as out scratch area
     for bookkeeping. We treat it as an array of
     pointers to allocated pages.
   - Allocate pages and store each pointer in this page
   - Sort this array of pointers.
   - Grab contigious chunks and save good ones and bad
     ones in separate lists.
   - Free the bad ones
   - We can pick and choose chunks from the good list
   - Free the ones rejected from the free list
  */
static int
find_free_chunks (struct mem_desc **result)
{
  ulong *page_table, memory, prev, start;
  int max_index, npages;
  int i, j;
  int page_threshold, total_pages_alloced, total_pages_rec;
  struct mem_desc *good_list, *free_list;
  
  *result = NULL;
  page_table = (ulong *) __get_free_pages (__GFP_NOWARN, get_order (NLM_SCRATCH_PAGE_SIZE));
  if (!page_table)
    return -ENOMEM;

  NLM_VERB (": dynamic memory region search started\n");

  free_list = NULL;
  good_list = NULL;
  total_pages_alloced = 0;
  total_pages_rec = 0;
  
 restart:
  memset (page_table, 0, NLM_SCRATCH_PAGE_SIZE);
  max_index = NLM_SCRATCH_PAGE_SIZE / sizeof (ulong);

  /* We will ignore any contigious chunks below this
     size 4 MB */
  page_threshold = (NLM_MIN_ALLOC_SIZE) / PAGE_SIZE;

  /* allocate as many as possible, convert the virtual
     address to a physical address, as the pages may
     appear virtually contigious, however, they are not
     physically contigious */
  for (i = 0; i < max_index; i++)
    {
      memory = __get_free_pages (__GFP_NOWARN, 0);
      if (!memory)
        break;

      total_pages_alloced++;
      page_table[i] = virt_to_phys ((void *)memory);
    }
  
  /* typically for a 2Mb scratch page and 8 byte pointers, we
     can scan 1Gb of memory */
  NLM_VERB (": sorting array of size %d\n", i);
  
  /* Lets sort the array */
  sort (page_table, i, sizeof (ulong),
        compare_addresses, NULL);
  
  start = prev = page_table[0];
  npages = 1;
  for (j = 1; j < i; j++)
    {
      memory = page_table[j];
      if ((prev + PAGE_SIZE) == memory)
        {
          npages++;
          prev = memory;
        }
      else
        {
          /* If npages is below our threshold, put the set of
             pages into a free list */
          if (npages < page_threshold)
            {
              struct mem_desc *segment = (struct mem_desc *) phys_to_virt (start);
              segment->next = free_list;
              segment->npages = npages;
              free_list = segment;
            }
          else
            {
              struct mem_desc *segment = (struct mem_desc *) phys_to_virt (start);
              segment->next = NULL;
              segment->npages = npages;
              good_list = insert_into_sorted_list (good_list, segment);
            }
          
          prev = start = memory;
          npages = 1;
        }
    }

  /* When we fall out we may have one set of free pages left.
     We may go back and re-fill our 2Mb page, in which case
     this address may straddle our scratch page, and we will
     miss it as a contigious chunk. Unfortunately, let it go
     for now */
  if (npages < page_threshold)
    {
      struct mem_desc *segment = (struct mem_desc *) phys_to_virt (start);
      segment->next = free_list;
      segment->npages = npages;
      free_list = segment;
    }
  else
    {
      struct mem_desc *segment = (struct mem_desc *) phys_to_virt (start);
      segment->next = good_list;
      segment->npages = npages;
      good_list = insert_into_sorted_list (good_list, segment);
    }
  
  if (i == max_index)
    {
      /* we exhausted our bookkeeping page, lets go
         back and try again */
      goto restart;
    }
  
  /* We have exhausted the whole RAM, lets see what we got */
  {
    struct mem_desc *ptr = good_list;
    while (ptr)
      {
        total_pages_rec += ptr->npages;
        NLM_VERB (": Good chunk 0x%lx, npages %d\n", (ulong)virt_to_phys (ptr), ptr->npages);
        ptr = ptr->next;
      }
    ptr = free_list;
    while (ptr)
      {
        total_pages_rec += ptr->npages;
        ptr = ptr->next;
      }
    release_chunks (free_list);
  }

  if (total_pages_alloced != total_pages_rec)
    {
      NLM_INFO (": Total pages allocated %d, manipulated %d do not match\n",
                total_pages_alloced, total_pages_rec);
      return -EFAULT;
    }
  
  free_pages ((ulong)page_table, get_order (NLM_SCRATCH_PAGE_SIZE));
  *result = good_list;
  return 0;
}

static int
dynamic_memory_init (void)
{
  int i, err;
  ulong size;
  struct mem_desc *chunklist, *chunk;

  err = find_free_chunks (&chunklist);
  if (err)
    return err;

  /* We have a bunch of chunks we could find from the
     system. We now pick the ones that are interesting
     to us.
     
     We use the following scheme. If the user asked us
     to try allocating fixed_phys_size, then find the
     chunk that is larger or equal to this request.
     If the user asked for 256 Mb, we have a Gigabyte
     chunk, then pick it we will free up the remaining
     from this chunk.
     
     If the user did not request a size, then we attempt
     to find a 256Mb chunk.
     
     If there is nothing reasonable, we will go ahead
     and see how many chunks we have on the list and
     sequentially assign each chunk to a device. Each
     device may then get different sized memory. */

  if (fixed_phys_size == 0)
    size = NLM_DEFAULT_PHYS_MEM_SIZE;
  else
    size = fixed_phys_size;
  
  size = PAGE_ALIGN (size);
  NLM_VERB (": Looking for suitable chunk of size %ld\n", size);
  
  if ((chunklist->npages * PAGE_SIZE) == size)
    {
      /* We have a chunk, carve it up to the requested
         size and allocate out of it. */
      NLM_INFO (": Found exact match chunk of size %ld at 0x%lx\n", size, (ulong) virt_to_phys (chunklist));
      release_chunks (chunklist->next);
      return fixed_memory_chunking ((ulong)virt_to_phys (chunklist), size, dmamem_size);
    }
  else if ((chunklist->npages * PAGE_SIZE) > size)
    {
      NLM_INFO (": Found bigger chunk than requested of size %ld at 0x%lx, %ld\n", 
                size, (ulong) virt_to_phys (chunklist), chunklist->npages * PAGE_SIZE);
      chunk = (struct mem_desc *) (((ulong) chunklist) + size);
      chunk->next = chunklist->next;
      chunk->npages = chunklist->npages - (size / PAGE_SIZE);
      release_chunks (chunk);
      return fixed_memory_chunking ((ulong)virt_to_phys (chunklist), size, dmamem_size);
    }
  
  /* absolutely nothing reasonable, lets go round robin
     and allocate what we have to each device. First
     allocate all system memory and then packet memory. Since
     the list is sorted, we will get smaller chunks for packet memory */
  for (i = 0; i < n_devices; i++)
    {
      if (chunklist)
        {
          device_list[i].sysmem_base = virt_to_phys ((void *)chunklist);
          device_list[i].sysmem_size = chunklist->npages * PAGE_SIZE;
          chunklist = chunklist->next;
        }
      else
        return -ENOMEM;
    }
  
  /* Now packet memory */
  for (i = 0; i < n_devices; i++)
    {
      if (chunklist)
        {
          device_list[i].packetmem_base = virt_to_phys ((void *)chunklist);
          device_list[i].packetmem_size = chunklist->npages * PAGE_SIZE;
          chunklist = chunklist->next;
        }
      else
        return -ENOMEM;
    }

  release_chunks (chunklist);
  return 0;
}

static int
memory_manager_init ()
{
  if (fixed_phys_base != 0)
    return fixed_memory_chunking (fixed_phys_base, fixed_phys_size, dmamem_size);
  
  return dynamic_memory_init ();
}

static void
memory_manager_fini ()
{
  int i;
  if (fixed_phys_base != 0)
    return;
  
  /* return the dynamically allocated memory segments */
  for (i = 0; i < n_devices; i++)
    {
      if (device_list[i].sysmem_base)
        {
          struct mem_desc *list = (struct mem_desc *) phys_to_virt (device_list[i].sysmem_base);
          list->next = NULL;
          list->npages = device_list[i].sysmem_size / PAGE_SIZE;
          release_chunks (list);
        }
      if (device_list[i].packetmem_base)
        {
          struct mem_desc *list = (struct mem_desc *) phys_to_virt (device_list[i].packetmem_base);
          list->next = NULL;
          list->npages = device_list[i].packetmem_size / PAGE_SIZE;
          release_chunks (list);
        }
    }
}

module_init (nlm_drv_module_init);
module_exit (nlm_drv_module_exit);

MODULE_LICENSE ("Proprietary");
MODULE_AUTHOR ("NetLogic Microsystems Inc.");
MODULE_DESCRIPTION ("Driver for NetL7 device");

