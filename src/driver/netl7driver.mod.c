#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v0000184Ed00000008sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000184Ed00000009sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000184Ed0000000Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000184Ed0000000Bsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v0000184Ed0000000Csv*sd*bc*sc*i*");
