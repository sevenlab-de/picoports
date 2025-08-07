#ifndef _PICOPORTS_TUSB_CONFIG_H_
#define _PICOPORTS_TUSB_CONFIG_H_

#if CFG_TUSB_MCU != OPT_MCU_RP2040
#error Only rp2040 is supported!
#endif

#define BOARD_TUD_RHPORT 0
#define CFG_TUD_ENABLED 1
#define CFG_TUD_VENDOR 1

#endif /* _PICOPORTS_TUSB_CONFIG_H_ */
