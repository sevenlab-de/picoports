// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 sevenlab engineering GmbH
 */
#ifndef _PICOPORTS_TUSB_CONFIG_H_
#define _PICOPORTS_TUSB_CONFIG_H_

#include "dln2.h"

// TinyUSB names the Raspberry Pi RP-series USB backend OPT_MCU_RP2040.
// That backend is used for both Pico 1 and Pico 2 builds.
#if CFG_TUSB_MCU != OPT_MCU_RP2040
#error PicoPorts requires the TinyUSB Raspberry Pi RP-series USB backend
#endif

#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE
#define CFG_TUD_ENABLED 1

#define CFG_TUD_VENDOR 2

// We cannot set different buffer sizes per vendor interface. Debugprobe uses
// 8192, dln2 uses 512, upper bounding to 8192.
#define CFG_TUD_VENDOR_RX_BUFSIZE 8192
#define CFG_TUD_VENDOR_TX_BUFSIZE 8192
#define CFG_TUD_VENDOR_EPSIZE 64

#define CFG_TUD_CDC 1

#define CFG_TUD_CDC_RX_BUFSIZE 64
#define CFG_TUD_CDC_TX_BUFSIZE 64
#define CFG_TUD_CDC_EP_BUFSIZE 64

#endif /* _PICOPORTS_TUSB_CONFIG_H_ */
