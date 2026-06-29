// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 sevenlab engineering GmbH
 */
#include "tusb.h"

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "bsp/board_api.h"
#include "pico/bootrom.h"

#include "byte_ops.h"
#include "dln2.h"
#include "pp_adc.h"
#include "pp_ctrl.h"
#include "pp_gpio.h"
#include "pp_i2c.h"

static void send_delayed_messages(void);

#define FIRMWARE_UPGRADE_MAGIC "FIRMWAREUPGRADE"
#define FIRMWARE_UPGRADE_MAGIC_LEN (sizeof(FIRMWARE_UPGRADE_MAGIC) - 1)

static bool is_firmware_upgrade_request(const uint8_t *buf, uint16_t buf_size)
{
	return buf_size == FIRMWARE_UPGRADE_MAGIC_LEN &&
	       memcmp(buf, FIRMWARE_UPGRADE_MAGIC,
		      FIRMWARE_UPGRADE_MAGIC_LEN) == 0;
}

void picoports_init(void)
{
	pp_gpio_init();
	pp_adc_init();
	pp_i2c_init();
}

void vApplicationIdleHook(void)
{
	for (;;) {
		// This code was not written for preemption, so we're disabling
		// the scheduler while it is running. We're splitting it up to
		// have a preemption point in between.

		vTaskSuspendAll();
		pp_gpio_task();
		xTaskResumeAll();

		vTaskSuspendAll();
		pp_adc_task();
		xTaskResumeAll();

		vTaskSuspendAll();
		send_delayed_messages();
		xTaskResumeAll();
	}
}

TU_ATTR_UNUSED static const char *handle2str(uint16_t handle)
{
	// clang-format off
	switch (handle) {
	case DLN2_HANDLE_EVENT: return "EVENT";
	case DLN2_HANDLE_CTRL: return "CTRL";
	case DLN2_HANDLE_GPIO: return "GPIO";
	case DLN2_HANDLE_I2C: return "I2C";
	case DLN2_HANDLE_SPI: return "SPI";
	case DLN2_HANDLE_ADC: return "ADC";
	default: return "???";
	}
	// clang-format on
}

#define MAX_NUM_BUF_MSGS 16
static uint8_t message_buffer[MAX_NUM_BUF_MSGS * DLN2_RX_BUF_SIZE];
static size_t r_id;
static size_t w_id;

static void send_delayed_messages(void)
{
	if (r_id == w_id)
		return;

	uint32_t bytes_avail = tud_vendor_write_available();
	if (bytes_avail < DLN2_RX_BUF_SIZE)
		return;

	uint8_t *message = &message_buffer[r_id];
	uint16_t size = u16_from_buf_le(&message[0]);

	uint32_t bytes_written = tud_vendor_write(message, size);
	uint32_t bytes_flushed = tud_vendor_write_flush();

	TU_LOG3("main: sent message (%" PRIu32 "-%" PRIu32 "-%" PRIu32 ") = ",
		bytes_avail, bytes_written, bytes_flushed);
	(void)bytes_written;
	(void)bytes_flushed;
	TU_LOG3_BUF(message, size);

	r_id += DLN2_RX_BUF_SIZE;
	if (r_id >= sizeof(message_buffer))
		r_id = 0;
}

// Header:
//   0: u16 size
//   2: u16 id
//   4: u16 echo
//   6: u16 handle
// Payload:
//   8: u8[] data
// In request responses, data begins with a u16 response code.
#define MSG_HDR_SZ 8
// From the driver code, it seems all codes above 0x80 are failure codes.
#define RESPONSE_CODE_OK 0
#define RESPONSE_CODE_FAILED 0xFFFF

void send_message_delayed(uint16_t cmd, uint16_t echo, enum dln2_handle handle,
			  uint8_t *data, uint16_t data_len)
{
	TU_ASSERT(data_len <= DLN2_RX_BUF_SIZE - MSG_HDR_SZ, );

	uint8_t *buf = &message_buffer[w_id];

	uint16_t size = MSG_HDR_SZ + data_len;
	u16_to_buf_le(&buf[0], size);
	u16_to_buf_le(&buf[2], cmd);
	u16_to_buf_le(&buf[4], echo);
	u16_to_buf_le(&buf[6], handle);
	memcpy(&buf[MSG_HDR_SZ], data, data_len);

	TU_LOG3("main: Request to send %u byte from %s\r\n", data_len,
		handle2str(handle));

	w_id += DLN2_RX_BUF_SIZE;
	if (w_id >= sizeof(message_buffer))
		w_id = 0;
}

static bool handle_rx_data(const uint8_t *buf_in, uint16_t buf_in_size)
{
	TU_VERIFY(buf_in_size >= MSG_HDR_SZ);

	uint16_t size = u16_from_buf_le(&buf_in[0]);
	const uint16_t id = u16_from_buf_le(&buf_in[2]);
	const uint16_t echo = u16_from_buf_le(&buf_in[4]);
	const uint16_t handle = u16_from_buf_le(&buf_in[6]);

	TU_VERIFY(size == buf_in_size);
	TU_VERIFY(size <= DLN2_RX_BUF_SIZE);

	TU_LOG3("main: Request to handle %u (%s): command %u (size=%u, echo=%u)\r\n",
		handle, handle2str(handle), id, size, echo);

	const uint8_t *data_in = &buf_in[MSG_HDR_SZ];
	uint16_t data_in_len = buf_in_size - MSG_HDR_SZ;
	// It's a big buffer, declaring it static to avoid blowing up the stack.
	static uint8_t buf_out[DLN2_RX_BUF_SIZE - MSG_HDR_SZ];
	// We're going to insert the response code before the data_out.
	uint8_t *data_out = &buf_out[2];
	uint16_t data_out_len = TU_ARRAY_SIZE(buf_out) - 2;

	bool ok;
	switch (handle) {
	case DLN2_HANDLE_ADC:
		ok = pp_adc_handle_request(id, data_in, data_in_len, data_out,
					   &data_out_len);
		break;

	case DLN2_HANDLE_CTRL:
		ok = pp_ctrl_handle_request(id, data_in, data_in_len, data_out,
					    &data_out_len);
		break;

	case DLN2_HANDLE_GPIO:
		ok = pp_gpio_handle_request(id, data_in, data_in_len, data_out,
					    &data_out_len);
		break;

	case DLN2_HANDLE_I2C:
		ok = pp_i2c_handle_request(id, data_in, data_in_len, data_out,
					   &data_out_len);
		break;

	default:
		TU_LOG1("main: Handle %u (%s) not implemented\r\n", handle,
			handle2str(handle));
		ok = false;
	}

	if (!ok) {
		TU_LOG2("main: Failed to handle %s request\r\n",
			handle2str(handle));
		data_out_len = 0;
	}

	u16_to_buf_le(&buf_out[0],
		      ok ? RESPONSE_CODE_OK : RESPONSE_CODE_FAILED);

	send_message_delayed(id, echo, handle, buf_out, data_out_len + 2);

	return true;
}

void tud_vendor_rx_cb(uint8_t itf, const uint8_t *buf_in, uint16_t buf_in_size)
{
	TU_LOG3("main: buf_in = ");
	TU_LOG3_BUF(buf_in, buf_in_size);

	if (itf != 0)
		goto out;

	if (is_firmware_upgrade_request(buf_in, buf_in_size)) {
		tud_vendor_n_read_flush(itf);
		rom_reset_usb_boot_extra(-1, 0, 0);
	}

	handle_rx_data(buf_in, buf_in_size);

out:
	tud_vendor_n_read_flush(itf);
}
