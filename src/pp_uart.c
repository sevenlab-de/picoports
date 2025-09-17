// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 sevenlab engineering GmbH
 */
#include "tusb.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"

#define PP_UART_INST uart1
#define PP_UART_PIN_TX 20
#define PP_UART_PIN_RX 21
#define PP_UART_DEFAULT_SPEED 115200
#define PP_UART_DEFAULT_DATA_BITS 8
#define PP_UART_DEFAULT_STOP_BITS 1
#define PP_UART_DEFAULT_PARITY UART_PARITY_NONE

void pp_uart_init(void)
{
#ifndef PP_GPIO_ONLY
	gpio_set_function(PP_UART_PIN_TX, GPIO_FUNC_UART);
	gpio_set_function(PP_UART_PIN_RX, GPIO_FUNC_UART);
	uart_init(PP_UART_INST, PP_UART_DEFAULT_SPEED);
#endif
}

void pp_uart_task(void)
{
#ifndef PP_GPIO_ONLY
	char buf[CFG_TUD_CDC_EP_BUFSIZE];

	uint32_t i;
	for (i = 0; i < sizeof(buf) && uart_is_readable(PP_UART_INST); i++) {
		buf[i] = uart_getc(PP_UART_INST);
	}

	if (i > 0) {
		uint32_t written = tud_cdc_write(buf, i);
		tud_cdc_write_flush();
		TU_LOG3("Forwarded %" PRIu32 " bytes to host\r\n", written);
		(void)written;
	}
#endif
}

#ifndef PP_GPIO_ONLY

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
	TU_LOG3("UART: Line state changed (DTR=%u, RTS=%u)\r\n", dtr, rts);
	(void)itf;
	(void)dtr;
	(void)rts;
}

static uint cdc_to_pico_uart_stop_bits(cdc_line_coding_stopbits_t stop_bits)
{
	switch (stop_bits) {
	case CDC_LINE_CODING_STOP_BITS_1:
		return 1;
	case CDC_LINE_CODING_STOP_BITS_2:
		return 2;
	default:
		TU_LOG1("UART: Unsupported number of stop bits (%u)\r\n",
			stop_bits);
		return PP_UART_DEFAULT_STOP_BITS;
	};
}

static uart_parity_t cdc_to_pico_uart_parity(cdc_line_coding_parity_t parity)
{
	switch (parity) {
	case CDC_LINE_CODING_PARITY_NONE:
		return UART_PARITY_NONE;
	case CDC_LINE_CODING_PARITY_ODD:
		return UART_PARITY_ODD;
	case CDC_LINE_CODING_PARITY_EVEN:
		return UART_PARITY_EVEN;
	default:
		TU_LOG1("UART: Unsupported parity mode (%u)\r\n", parity);
		return PP_UART_DEFAULT_PARITY;
	};
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *p_line_coding)
{
	(void)itf;

	TU_LOG3("UART: Line coding changed (bit_rate=%" PRIu32
		", stop_bits=%" PRIu8 ", parity=%" PRIu8 ", data_bits=%" PRIu8
		")\r\n",
		p_line_coding->bit_rate, p_line_coding->stop_bits,
		p_line_coding->parity, p_line_coding->data_bits);

	uint baud = uart_set_baudrate(PP_UART_INST, p_line_coding->bit_rate);
	TU_LOG3("UART: try setting baud %" PRIu32 ", baud is %u\r\n",
		p_line_coding->bit_rate, baud);
	(void)baud;

	uint stop_bits = cdc_to_pico_uart_stop_bits(p_line_coding->stop_bits);
	uart_parity_t parity = cdc_to_pico_uart_parity(p_line_coding->parity);
	uint data_bits;
	if (p_line_coding->data_bits >= 5 && p_line_coding->data_bits <= 8) {
		data_bits = p_line_coding->data_bits;
	} else {
		TU_LOG1("UART: Unsupported number of data bits (%u)\r\n",
			p_line_coding->data_bits);
		data_bits = PP_UART_DEFAULT_DATA_BITS;
	}

	uart_set_format(PP_UART_INST, data_bits, stop_bits, parity);
}

void tud_cdc_rx_cb(uint8_t itf)
{
	(void)itf;

	while (tud_cdc_available()) {
		uint8_t buf[CFG_TUD_CDC_EP_BUFSIZE];
		uint32_t count = tud_cdc_read(buf, sizeof(buf));
		uart_write_blocking(PP_UART_INST, buf, count);
		TU_LOG3("Forwarded %" PRIu32 " bytes to UART\r\n", count);
	}
}

#endif
