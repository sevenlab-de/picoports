// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2025 sevenlab engineering GmbH
 */
#include "tusb.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"

#define PP_UART_INST uart1
#define PP_UART_SPEED 115200
#define PP_UART_PIN_TX 20
#define PP_UART_PIN_RX 21

void pp_uart_init(void)
{
#ifndef PP_GPIO_ONLY
	gpio_set_function(PP_UART_PIN_TX, GPIO_FUNC_UART);
	gpio_set_function(PP_UART_PIN_RX, GPIO_FUNC_UART);
	uart_init(PP_UART_INST, PP_UART_SPEED);
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
		TU_LOG3("Forwarded %" PRIu32 " bytes!\r\n", written);
		(void)written;
	}
#endif
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
	TU_LOG3("UART: Line state changed (DTR=%u, RTS=%u)\r\n", dtr, rts);
	(void)itf;
	(void)dtr;
	(void)rts;
}

void tud_cdc_rx_cb(uint8_t itf)
{
	(void)itf;

#ifndef PP_GPIO_ONLY
	while (tud_cdc_available()) {
		uint8_t buf[CFG_TUD_CDC_EP_BUFSIZE];
		uint32_t count = tud_cdc_read(buf, sizeof(buf));
		uart_write_blocking(PP_UART_INST, buf, count);
	}
#endif
}
