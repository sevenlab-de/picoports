#include "tusb.h"

#include "bsp/board_api.h"

#include "dln2.h"
#include "byte_ops.h"
#include "pp_ctrl.h"
#include "pp_gpio.h"

int main(void)
{
	board_init();

	tusb_rhport_init_t dev_init = {
		.role = TUSB_ROLE_DEVICE,
		.speed = TUSB_SPEED_AUTO
	};
	tusb_init(BOARD_TUD_RHPORT, &dev_init);

	if (board_init_after_tusb) {
		board_init_after_tusb();
	}

	if (!pp_gpio_init()) {
		TU_LOG1("main: Failed to initialize GPIO module!\r\n");
	}

	while (1) {
		tud_task();
	}
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, const tusb_control_request_t* request)
{
	(void)rhport; (void)stage; (void)request;

	/* Is this called at some point? */
	TU_LOG1("==> tud_vendor_control_xfer_cb %u %u: ", rhport, stage);
	TU_LOG1_BUF((const uint8_t *)request, sizeof(request));

	return false; /* stall */
}

TU_ATTR_UNUSED static const char *handle2str(uint16_t handle)
{
	switch(handle) {
		case DLN2_HANDLE_EVENT: return "EVENT";
		case DLN2_HANDLE_CTRL: return "CTRL";
		case DLN2_HANDLE_GPIO: return "GPIO";
		case DLN2_HANDLE_I2C: return "I2C";
		case DLN2_HANDLE_SPI: return "SPI";
		case DLN2_HANDLE_ADC: return "ADC";
		default: return "???";
	}
}

// Request:
//   Header:
//     0: u16 size
//     2: u16 id
//     4: u16 echo
//     6: u16 handle
//   Payload:
//     8: u8[] data
//
// Response:
//   Header:
//     0: u16 size
//     2: u16 id
//     4: u16 echo
//     6: u16 handle
//     8: u16 response code
//   Payload:
//     10: u8[] data
#define REQUEST_HEADER_SIZE 8
#define RESPONSE_HEADER_SIZE 10
// From the driver code, it seems all codes above 0x80 are failure codes.
#define RESPONSE_CODE_OK 0
#define RESPONSE_CODE_FAILED 0xFFFF

static bool handle_rx_data(const uint8_t *buf_in, uint16_t buf_in_size)
{
	TU_VERIFY(buf_in_size >= REQUEST_HEADER_SIZE);

	uint16_t size = u16_from_buf_le(&buf_in[0]);
	const uint16_t id = u16_from_buf_le(&buf_in[2]);
	const uint16_t echo = u16_from_buf_le(&buf_in[4]);
	const uint16_t handle = u16_from_buf_le(&buf_in[6]);

	TU_VERIFY(size == buf_in_size);

	TU_LOG3("main: Request to handle %u (%s): command %u (size=%u, echo=%u)\r\n",
		handle, handle2str(handle), id, size, echo); (void)echo;

	const uint8_t *data_in = &buf_in[REQUEST_HEADER_SIZE];
	uint16_t data_in_len = buf_in_size - REQUEST_HEADER_SIZE;
	uint8_t buf_out[CFG_TUD_VENDOR_RX_BUFSIZE];
	uint8_t *data_out = &buf_out[RESPONSE_HEADER_SIZE];
	uint16_t data_out_len = TU_ARRAY_SIZE(buf_out) - RESPONSE_HEADER_SIZE;

	bool ok;
	switch(handle) {
		case DLN2_HANDLE_CTRL:
			ok = pp_ctrl_handle_request(id, data_in, data_in_len, data_out, &data_out_len);
			break;

		case DLN2_HANDLE_GPIO:
			ok = pp_gpio_handle_request(id, data_in, data_in_len, data_out, &data_out_len);
			break;

		default:
			TU_LOG1("main: Handle %u (%s) not implemented\r\n", handle,
				handle2str(handle));
			ok = false;
			data_out_len = 0;
	}
	size = RESPONSE_HEADER_SIZE + data_out_len;

	if (!ok) {
		TU_LOG2("main: Failed to handle %s request\r\n", handle2str(handle));
	}

	u16_to_buf_le(&buf_out[0], size);
	/* Copy request metadata to response while skipping the size. */
	memcpy(&buf_out[2], &buf_in[2], REQUEST_HEADER_SIZE - 2);
	u16_to_buf_le(&buf_out[8], ok ? RESPONSE_CODE_OK : RESPONSE_CODE_FAILED);

	TU_LOG3("main: buf_out = ");
	TU_LOG3_BUF(buf_out, size);

	tud_vendor_write(buf_out, size);
	tud_vendor_write_flush();

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
