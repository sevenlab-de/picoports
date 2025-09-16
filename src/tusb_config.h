// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 sevenlab engineering GmbH
 */
#ifndef _PICOPORTS_TUSB_CONFIG_H_
#define _PICOPORTS_TUSB_CONFIG_H_

#include "dln2.h"

#if CFG_TUSB_MCU != OPT_MCU_RP2040
#error Only rp2040 is supported!
#endif

#define BOARD_TUD_RHPORT 0
#define CFG_TUD_ENABLED 1

#define CFG_TUD_VENDOR 1

#define CFG_TUD_VENDOR_RX_BUFSIZE DLN2_RX_BUF_SIZE
#define CFG_TUD_VENDOR_TX_BUFSIZE DLN2_RX_BUF_SIZE
#define CFG_TUD_VENDOR_EPSIZE 64

#ifndef PP_GPIO_ONLY
#define CFG_TUD_CDC 1

#define CFG_TUD_CDC_RX_BUFSIZE 64
#define CFG_TUD_CDC_TX_BUFSIZE 64
#define CFG_TUD_CDC_EP_BUFSIZE 64
#endif

#endif /* _PICOPORTS_TUSB_CONFIG_H_ */
