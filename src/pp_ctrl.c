// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 sevenlab engineering GmbH
 */
#include "tusb.h"

#include "bsp/board_api.h"

#include "byte_ops.h"
#include "dln2.h"

TU_ATTR_UNUSED static const char *ctrl_cmd2str(uint16_t cmd)
{
	// clang-format off
	switch (cmd) {
	case CMD_GET_DEVICE_VER: return "GET_DEVICE_VER";
	case CMD_GET_DEVICE_SN: return "GET_DEVICE_SN";
	default: return "???";
	}
	// clang-format on
}

static bool handle_request(uint16_t cmd, uint32_t *value)
{
	switch (cmd) {
	case CMD_GET_DEVICE_VER:
		*value = DLN2_HW_ID;
		break;
	case CMD_GET_DEVICE_SN: {
		uint8_t uid[4];
		size_t len = board_get_unique_id(uid, 4);
		TU_ASSERT(len == 4); /* This should always work on RP2040. */
		*value = u32_from_buf_le(uid);
		break;
	}
	default:
		TU_VERIFY(false);
	}

	return true;
}

bool pp_ctrl_handle_request(uint16_t cmd, uint8_t const *data_in,
			    uint16_t data_in_len, uint8_t *data_out,
			    uint16_t *data_out_len)
{
	(void)data_in;

	TU_LOG3("CTRL: %s\r\n", ctrl_cmd2str(cmd));

	TU_VERIFY(data_in_len == 0);
	TU_ASSERT(*data_out_len >= 4);
	*data_out_len = 0;

	uint32_t val;

	bool res = handle_request(cmd, &val);
	TU_VERIFY(res);

	u32_to_buf_le(data_out, val);
	*data_out_len = 4;

	return true;
}
