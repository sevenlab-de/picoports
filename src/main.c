#include "tusb.h"

#include "bsp/board_api.h"

#include "byte_ops.h"
#include "dln2.h"
#include "pp_adc.h"
#include "pp_ctrl.h"
#include "pp_gpio.h"
#include "pp_i2c.h"

static void send_delayed_messages(void);

int main(void)
{
	board_init();

	tusb_rhport_init_t dev_init = { .role = TUSB_ROLE_DEVICE,
					.speed = TUSB_SPEED_AUTO };
	tusb_init(BOARD_TUD_RHPORT, &dev_init);

	if (board_init_after_tusb) {
		board_init_after_tusb();
	}

	pp_gpio_init();
	pp_adc_init();
	pp_i2c_init();

	while (1) {
		tud_task();
		gpio_process_events();
		send_delayed_messages();
	}
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage,
				const tusb_control_request_t *request)
{
	(void)rhport;
	(void)stage;
	(void)request;

	/* Is this called at some point? */
	TU_LOG1("==> tud_vendor_control_xfer_cb %u %u: ", rhport, stage);
	TU_LOG1_BUF((const uint8_t *)request, sizeof(request));

	return false; /* stall */
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
static uint8_t message_buffer[MAX_NUM_BUF_MSGS * CFG_TUD_VENDOR_TX_BUFSIZE];
static size_t r_id;
static size_t w_id;

static void send_delayed_messages(void)
{
	if (r_id == w_id)
		return;

	uint32_t bytes_avail = tud_vendor_write_available();
	if (bytes_avail != CFG_TUD_VENDOR_TX_BUFSIZE)
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

	r_id += CFG_TUD_VENDOR_TX_BUFSIZE;
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
	TU_ASSERT(data_len <= CFG_TUD_VENDOR_TX_BUFSIZE - MSG_HDR_SZ, );

	uint8_t *buf = &message_buffer[w_id];

	uint16_t size = MSG_HDR_SZ + data_len;
	u16_to_buf_le(&buf[0], size);
	u16_to_buf_le(&buf[2], cmd);
	u16_to_buf_le(&buf[4], echo);
	u16_to_buf_le(&buf[6], handle);
	memcpy(&buf[MSG_HDR_SZ], data, data_len);

	TU_LOG3("main: Request to send %u byte from %s\r\n", data_len,
		handle2str(handle));

	w_id += CFG_TUD_VENDOR_TX_BUFSIZE;
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

	TU_LOG3("main: Request to handle %u (%s): command %u (size=%u, echo=%u)\r\n",
		handle, handle2str(handle), id, size, echo);
	(void)echo;

	const uint8_t *data_in = &buf_in[MSG_HDR_SZ];
	uint16_t data_in_len = buf_in_size - MSG_HDR_SZ;
	uint8_t buf_out[CFG_TUD_VENDOR_TX_BUFSIZE - MSG_HDR_SZ];
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
	(void)itf;

	TU_LOG3("main: buf_in = ");
	TU_LOG3_BUF(buf_in, buf_in_size);

	handle_rx_data(buf_in, buf_in_size);

	tud_vendor_read_flush();
}
