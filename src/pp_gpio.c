#include "tusb.h"

#include "hardware/gpio.h"

#include "dln2.h"
#include "byte_ops.h"

static uint8_t gpio_pins[] = {
	/* 0 and 1 are used for UART logging */
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	26, 27, 28,
	25 /* LED */
};

#define NUM_GPIOS  TU_ARRAY_SIZE(gpio_pins)

TU_ATTR_UNUSED static const char *gpio_cmd2str(uint16_t cmd)
{
	switch(cmd) {
		case DLN2_GPIO_GET_PIN_COUNT: return "GET_PIN_COUNT";
		case DLN2_GPIO_SET_DEBOUNCE: return "SET_DEBOUNCE";
		case DLN2_GPIO_GET_DEBOUNCE: return "GET_DEBOUNCE";
		case DLN2_GPIO_PORT_GET_VAL: return "PORT_GET_VAL";
		case DLN2_GPIO_PIN_GET_VAL: return "PIN_GET_VAL";
		case DLN2_GPIO_PIN_SET_OUT_VAL: return "PIN_SET_OUT_VAL";
		case DLN2_GPIO_PIN_GET_OUT_VAL: return "PIN_GET_OUT_VAL";
		case DLN2_GPIO_CONDITION_MET_EV: return "CONDITION_MET_EV";
		case DLN2_GPIO_PIN_ENABLE: return "PIN_ENABLE";
		case DLN2_GPIO_PIN_DISABLE: return "PIN_DISABLE";
		case DLN2_GPIO_PIN_SET_DIRECTION: return "PIN_SET_DIRECTION";
		case DLN2_GPIO_PIN_GET_DIRECTION: return "PIN_GET_DIRECTION";
		case DLN2_GPIO_PIN_SET_EVENT_CFG: return "PIN_SET_EVENT_CFG";
		case DLN2_GPIO_PIN_GET_EVENT_CFG: return "PIN_GET_EVENT_CFG";
		default: return "???";
	}
}

TU_ATTR_UNUSED static const char *gpio_dir2str(uint8_t pin_dir)
{
	switch(pin_dir) {
	case DLN2_GPIO_DIRECTION_IN: return "IN";
	case DLN2_GPIO_DIRECTION_OUT: return "OUT";
	default: return "???";
	}
}

#define INVALID_PIN UINT16_MAX
#define INVALID_VAL UINT8_MAX

static bool handle_request(uint16_t cmd, uint16_t *pin, uint8_t *val)
{
	switch(cmd) {
		case DLN2_GPIO_GET_PIN_COUNT:
			/* The response value is not a pin here, but the number of GPIO pins. */
			*pin = NUM_GPIOS;
			TU_LOG3("GPIO: Reporting pin count: %u\r\n", *pin);
			break;
		case DLN2_GPIO_PIN_GET_VAL:
			TU_VERIFY(*pin < NUM_GPIOS);
			*val = !!gpio_get(gpio_pins[*pin]);
			TU_LOG3("GPIO: Getting pin %u value: %u\r\n", *pin, *val);
			break;
		case DLN2_GPIO_PIN_SET_OUT_VAL:
			TU_VERIFY(*pin < NUM_GPIOS);
			TU_VERIFY(*val != INVALID_VAL);
			TU_LOG3("GPIO: Setting pin %u value: %u\r\n", *pin, *val);
			gpio_put(gpio_pins[*pin], *val);
			break;
		case DLN2_GPIO_PIN_ENABLE:
			TU_VERIFY(*pin < NUM_GPIOS);
			TU_LOG3("GPIO: Enabling pin %u\r\n", *pin);
			break;
		case DLN2_GPIO_PIN_DISABLE:
			TU_VERIFY(*pin < NUM_GPIOS);
			TU_LOG3("GPIO: Disabling pin %u\r\n", *pin);
			break;
		case DLN2_GPIO_PIN_SET_DIRECTION:
			TU_VERIFY(*pin < NUM_GPIOS);
			TU_VERIFY(*val == DLN2_GPIO_DIRECTION_IN || *val == DLN2_GPIO_DIRECTION_OUT);
			gpio_set_dir(gpio_pins[*pin], *val == DLN2_GPIO_DIRECTION_OUT);
			TU_LOG3("GPIO: Setting pin %u direction: %s\r\n", *pin, gpio_dir2str(*val));
			break;
		case DLN2_GPIO_PIN_GET_DIRECTION:
			TU_VERIFY(*pin < NUM_GPIOS);
			*val = gpio_get_dir(gpio_pins[*pin]) ? DLN2_GPIO_DIRECTION_OUT : DLN2_GPIO_DIRECTION_IN;
			TU_LOG3("GPIO: Getting pin %u direction: %s\r\n", *pin, gpio_dir2str(*val));
			break;
		default:
			TU_VERIFY(false);
	}

	return true;
}

bool pp_gpio_handle_request(uint16_t cmd, uint8_t const *data_in, uint16_t data_in_len, uint8_t *data_out, uint16_t *data_out_len)
{
	TU_LOG3("GPIO: %s\r\n", gpio_cmd2str(cmd));

	TU_VERIFY(*data_out_len >= 3);
	*data_out_len = 0;

	uint16_t pin = INVALID_PIN;
	if (data_in_len >= 2) {
		pin = u16_from_buf_le(&data_in[0]);
		TU_ASSERT(pin != INVALID_PIN);
	}

	uint8_t val = INVALID_VAL;
	if (data_in_len >= 3) {
		val = data_in[2];
		TU_ASSERT(val != INVALID_VAL);
	}

	bool res = handle_request(cmd, &pin, &val);
	TU_VERIFY(res);

	if (pin != INVALID_PIN) {
		u16_to_buf_le(&data_out[*data_out_len], pin);
		*data_out_len += 2;
	}

	if (val != INVALID_VAL) {
		data_out[*data_out_len] = val;
		*data_out_len += 1;
	}

	return true;
}

bool pp_gpio_init(void)
{
	for (uint8_t i = 0; i < NUM_GPIOS; i++) {
		gpio_init(gpio_pins[i]);
	}

	return true;
}
