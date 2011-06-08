/*******************************************************************************
* Legal Notice:
* This product is the confidential property of NetLogic Microsystems Inc.
* ("NetLogic"), is provided under a non-disclosure agreement, and is protected
* under applicable copyright, patent, and trade secret laws. Unauthorized use,
* reproduction, distribution or other dissemination without the prior written
* authorization from NetLogic is strictly prohibited.
*
* NETLOGIC DISCLAIMS ALL WARRANTIES OF ANY NATURE, EXPRESS OR IMPLIED,
* INCLUDING, WITHOUT LIMITATION, THE WARRANTIES OF FITNESS FOR A PARTICULAR
* PURPOSE, MERCHANTABILITY AND/OR NON-INFRINGEMENT OF THIRD PARTY RIGHTS.
* NETLOGIC ASSUMES NO LIABILITY FOR ANY ERROR OR OMISSIONS IN THIS PRODUCT,OR
* FOR THE USE OF THIS PRODUCT. IN NO EVENT SHALL NETLOGIC BE LIABLE TO ANY OTHER
* PARTY FOR ANY SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL DAMAGES, WHETHER
* BASED ON BREACH OF CONTRACT, TORT, PRODUCT LIABILITY, INFRINGEMENT OF
* INTELLECTUAL PROPERTY RIGHTS OR OTHERWISE. NETLOGIC RESERVES THE RIGHT TO MAKE
* CHANGES TO, OR DISCONTINUE, ITS PRODUCTS AT ANY TIME.
*
* Distribution of the product herein does not convey a license or any other
* right in any patent, trademark, or other intellectual property of NetLogic.
*
* Use of the product shall serve as acceptance of these terms and conditions.
* If you do not accept these terms, you should return or destroy the product
* and any other accompanying information immediately.
*
* Copyright, 2008-2009, NetLogic Microsystems, Inc. All rights reserved.
*******************************************************************************/
#ifndef __NLM_DATABASE_EXTRA_H
#define __NLM_DATABASE_EXTRA_H

#include "nlm_database_api.h"

struct fms_cam_node;
struct fms_search_state;

nlm_status nlm_db_select_groups (struct nlm_database *database, uint32_t *group_id, uint32_t group_cnt,
                                 struct fms_search_state *state);

nlm_status nlm_db_parse_line (struct nlm_database *db, nlm_logger *logger,
                              uint32_t line_num, 
                              char *src, char *end);
                              
nlm_status nlm_database_analyze (struct nlm_database *db, const void *image, uint32_t image_size, 
                                 uint32_t insn_addr, uint32_t *rule_id);

#endif

