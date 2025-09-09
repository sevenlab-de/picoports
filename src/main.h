// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 sevenlab engineering GmbH
 */
#ifndef _PP_MAIN_H_
#define _PP_MAIN_H_

void send_message_delayed(uint16_t cmd, uint16_t echo, enum dln2_handle handle,
			  uint8_t *data, uint16_t data_len);

#endif /* _PP_MAIN_H_ */
