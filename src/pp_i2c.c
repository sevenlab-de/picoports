#include "tusb.h"

#include "hardware/gpio.h"

#include "pico/binary_info.h"

#include "byte_ops.h"
#include "dln2.h"

#ifdef USE_I2C_GPIOS
#include "hardware/i2c.h"
#endif

#define PP_I2C_INST i2c0
#define PP_I2C_SPEED_100KHZ (100 * 1000)
#define PP_I2C_PIN_SDA 16
#define PP_I2C_PIN_SCL 17

TU_ATTR_UNUSED static const char *i2c_cmd2str(uint16_t cmd)
{
	// clang-format off
	switch (cmd) {
	case DLN2_I2C_GET_PORT_COUNT: return "GET_PORT_COUNT";
	case DLN2_I2C_ENABLE: return "ENABLE";
	case DLN2_I2C_DISABLE: return "DISABLE";
	case DLN2_I2C_IS_ENABLED: return "IS_ENABLED";
	case DLN2_I2C_WRITE: return "WRITE";
	case DLN2_I2C_READ: return "READ";
	case DLN2_I2C_SCAN_DEVICES: return "SCAN_DEVICES";
	case DLN2_I2C_PULLUP_ENABLE: return "PULLUP_ENABLE";
	case DLN2_I2C_PULLUP_DISABLE: return "PULLUP_DISABLE";
	case DLN2_I2C_PULLUP_IS_ENABLED: return "PULLUP_IS_ENABLED";
	case DLN2_I2C_TRANSFER: return "TRANSFER";
	case DLN2_I2C_SET_MAX_REPLY_COUNT: return "SET_MAX_REPLY_COUNT";
	case DLN2_I2C_GET_MAX_REPLY_COUNT: return "GET_MAX_REPLY_COUNT";
	default: return "???";
	}
	// clang-format on
}

bool pp_i2c_handle_request(uint16_t cmd, uint8_t const *data_in,
			   uint16_t data_in_len, uint8_t *data_out,
			   uint16_t *data_out_len)
{
	TU_VERIFY(data_in_len >= 1);

	uint8_t port = data_in[0];
	TU_VERIFY(port == 0); // always 0 in kernel driver

#ifndef USE_I2C_GPIOS
	(void)cmd;
	(void)data_out;
	(void)data_out_len;
	// We have two choices here: Let the request to enable pass or fail it.
	// a) We let it pass, the kernel driver will initialize without issue,
	//    but the created device will fail when used.
	// b) We fail it, the kernel driver will fail to initialize and print an
	//    error log to the kernel log.
	// While a) looks nicer in the kernel log, I think b) is cleaner.
	return false;
#else
	switch (cmd) {
	case DLN2_I2C_ENABLE:
		TU_LOG3("I2C: Enabled\r\n");
		*data_out_len = 0;
		break;
	case DLN2_I2C_DISABLE:
		TU_LOG3("I2C: Disabled\r\n");
		*data_out_len = 0;
		break;
	case DLN2_I2C_WRITE: {
		// 0: u8 port (checked above)
		// 1: u8 addr
		// 2: u8 mem_addr_len
		// 3: u32 mem_addr
		// 7: u16 buf_len
		// 9: u8 buf[DLN2_I2C_MAX_XFER_SIZE]
		// 9+buf_len
		TU_VERIFY(data_in_len >= 9);
		uint8_t addr = data_in[1];
		uint8_t mem_addr_len = data_in[2];
		uint32_t mem_addr = u32_from_buf_le(&data_in[3]);
		uint16_t buf_len = u16_from_buf_le(&data_in[7]);
		const uint8_t *buf = &data_in[9];
		TU_VERIFY(addr <= 0x7f); // only 7-bit addresses are supported
		TU_VERIFY(mem_addr_len == 0); // always 0 in kernel driver
		TU_VERIFY(mem_addr == 0); // always 0 in kernel driver
		TU_VERIFY(data_in_len >= 9 + buf_len);

		TU_LOG3("I2C: Write %u byte to 0x%02X\r\n", buf_len, addr);
		TU_LOG3_BUF(buf, buf_len);

		int num_bytes = i2c_write_blocking(PP_I2C_INST, addr, buf,
						   buf_len, false);
		if (num_bytes != buf_len) {
			TU_LOG3("I2C: Write failed (%d)\r\n", num_bytes);
		}
		TU_VERIFY(num_bytes == buf_len);
		*data_out_len = 0;
		break;
	}
	case DLN2_I2C_READ: {
		// 0: u8 port (checked above)
		// 1: u8 addr
		// 2: u8 mem_addr_len
		// 3: u32 mem_addr
		// 7: u16 buf_len
		// 9
		TU_VERIFY(data_in_len >= 9);
		uint8_t addr = data_in[1];
		uint8_t mem_addr_len = data_in[2];
		uint32_t mem_addr = u32_from_buf_le(&data_in[3]);
		uint16_t buf_len = u16_from_buf_le(&data_in[7]);
		TU_VERIFY(addr <= 0x7f); // only 7-bit addresses are supported
		TU_VERIFY(mem_addr_len == 0); // always 0 in kernel driver
		TU_VERIFY(mem_addr == 0); // always 0 in kernel driver
		TU_VERIFY(buf_len + 2 <= *data_out_len);

		TU_LOG3("I2C: Read %u byte from 0x%02X\r\n", buf_len, addr);

		// 0: u16 buf_len;
		// 2: u8 buf[DLN2_I2C_MAX_XFER_SIZE]
		int num_bytes = i2c_read_blocking(PP_I2C_INST, addr,
						  &data_out[2], buf_len, false);
		if (num_bytes < 0) {
			TU_LOG3("I2C: Read failed (%d)\r\n", num_bytes);
		}
		TU_VERIFY(num_bytes >= 0);
		TU_ASSERT(num_bytes <= buf_len);

		TU_LOG3_BUF(&data_out[2], (uint16_t)num_bytes);

		u16_to_buf_le(&data_out[0], (uint16_t)num_bytes);
		*data_out_len = (uint16_t)num_bytes + 2;
		break;
	}
	default:
		TU_LOG1("I2C: Command not implemented: %s (%u)\r\n",
			i2c_cmd2str(cmd), cmd);
		TU_VERIFY(false);
	}

	return true;
#endif
}

int pp_i2c_init()
{
#ifdef USE_I2C_GPIOS
	i2c_init(PP_I2C_INST, PP_I2C_SPEED_100KHZ);
	gpio_set_function(PP_I2C_PIN_SDA, GPIO_FUNC_I2C);
	gpio_set_function(PP_I2C_PIN_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(PP_I2C_PIN_SDA);
	gpio_pull_up(PP_I2C_PIN_SCL);
	// Make the I2C pins available to picotool
	bi_decl(bi_2pins_with_func(PP_I2C_PIN_SDA, PP_I2C_PIN_SCL,
				   GPIO_FUNC_I2C));
#endif
	return 0;
}
