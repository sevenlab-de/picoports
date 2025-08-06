#ifndef _PICOPORTS_DLN2_H_
#define _PICOPORTS_DLN2_H_

// --- Defines from Linux include/linux/mfd/dln2.h ---

/* SPDX-License-Identifier: GPL-2.0 */

#define DLN2_CMD(cmd, id)		((cmd) | ((id) << 8))

// --- End of defines from Linux include/linux/mfd/dln2.h ---


// --- Defines from Linux drivers/gpio/gpio-dln2.c ---

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the Diolan DLN-2 USB-GPIO adapter
 *
 * Copyright (c) 2014 Intel Corporation
 */

#define DLN2_GPIO_ID			0x01

#define DLN2_GPIO_GET_PIN_COUNT		DLN2_CMD(0x01, DLN2_GPIO_ID)
#define DLN2_GPIO_SET_DEBOUNCE		DLN2_CMD(0x04, DLN2_GPIO_ID)
#define DLN2_GPIO_GET_DEBOUNCE		DLN2_CMD(0x05, DLN2_GPIO_ID)
#define DLN2_GPIO_PORT_GET_VAL		DLN2_CMD(0x06, DLN2_GPIO_ID)
#define DLN2_GPIO_PIN_GET_VAL		DLN2_CMD(0x0B, DLN2_GPIO_ID)
#define DLN2_GPIO_PIN_SET_OUT_VAL	DLN2_CMD(0x0C, DLN2_GPIO_ID)
#define DLN2_GPIO_PIN_GET_OUT_VAL	DLN2_CMD(0x0D, DLN2_GPIO_ID)
#define DLN2_GPIO_CONDITION_MET_EV	DLN2_CMD(0x0F, DLN2_GPIO_ID)
#define DLN2_GPIO_PIN_ENABLE		DLN2_CMD(0x10, DLN2_GPIO_ID)
#define DLN2_GPIO_PIN_DISABLE		DLN2_CMD(0x11, DLN2_GPIO_ID)
#define DLN2_GPIO_PIN_SET_DIRECTION	DLN2_CMD(0x13, DLN2_GPIO_ID)
#define DLN2_GPIO_PIN_GET_DIRECTION	DLN2_CMD(0x14, DLN2_GPIO_ID)
#define DLN2_GPIO_PIN_SET_EVENT_CFG	DLN2_CMD(0x1E, DLN2_GPIO_ID)
#define DLN2_GPIO_PIN_GET_EVENT_CFG	DLN2_CMD(0x1F, DLN2_GPIO_ID)

#define DLN2_GPIO_EVENT_NONE		0
#define DLN2_GPIO_EVENT_CHANGE		1
#define DLN2_GPIO_EVENT_LVL_HIGH	2
#define DLN2_GPIO_EVENT_LVL_LOW		3
#define DLN2_GPIO_EVENT_CHANGE_RISING	0x11
#define DLN2_GPIO_EVENT_CHANGE_FALLING  0x21
#define DLN2_GPIO_EVENT_MASK		0x0F

#define DLN2_GPIO_DIRECTION_IN		0
#define DLN2_GPIO_DIRECTION_OUT		1

// --- End of defines from Linux drivers/gpio/gpio-dln2.c ---


// --- Defines from Linux drivers/mfd/dln2.c ---

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the Diolan DLN-2 USB adapter
 *
 * Copyright (c) 2014 Intel Corporation
 *
 * Derived from:
 *  i2c-diolan-u2c.c
 *  Copyright (c) 2010-2011 Ericsson AB
 */

#define DLN2_GENERIC_MODULE_ID		0x00
#define DLN2_GENERIC_CMD(cmd)		DLN2_CMD(cmd, DLN2_GENERIC_MODULE_ID)
#define CMD_GET_DEVICE_VER		DLN2_GENERIC_CMD(0x30)
#define CMD_GET_DEVICE_SN		DLN2_GENERIC_CMD(0x31)

#define DLN2_HW_ID			0x200

enum dln2_handle {
	DLN2_HANDLE_EVENT = 0,		/* don't change, hardware defined */
	DLN2_HANDLE_CTRL,
	DLN2_HANDLE_GPIO,
	DLN2_HANDLE_I2C,
	DLN2_HANDLE_SPI,
	DLN2_HANDLE_ADC,
	DLN2_HANDLES
};

// --- End of defines from Linux drivers/mfd/dln2.c ---

#endif /* _PICOPORTS_DLN2_H_ */
