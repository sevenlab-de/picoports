#include "tusb.h"

#include "hardware/gpio.h"

#include "byte_ops.h"
#include "dln2.h"
#include "main.h"

static uint8_t gpio_pins[] = {
	/* 0 and 1 are used for UART logging */
	2,  3,	4,  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
#ifdef PP_GPIO_ONLY
	16, 17, // I2C
#endif
	18, 19,
#ifdef PP_GPIO_ONLY
	20, 21, // UART
#endif
	22,
#ifdef PP_GPIO_ONLY
	26, 27, 28, // ADC
#endif
	25 // Pico LED
};

#define NUM_GPIOS TU_ARRAY_SIZE(gpio_pins)

TU_ATTR_UNUSED static const char *gpio_cmd2str(uint16_t cmd)
{
	// clang-format off
	switch (cmd) {
	case DLN2_GPIO_GET_PIN_COUNT: return "GET_PIN_COUNT";
	case DLN2_GPIO_SET_DEBOUNCE: return "SET_DEBOUNCE";
	case DLN2_GPIO_GET_DEBOUNCE: return "GET_DEBOUNCE"; /* Unused by kernel driver */
	case DLN2_GPIO_PORT_GET_VAL: return "PORT_GET_VAL"; /* Unused by kernel driver */
	case DLN2_GPIO_PIN_GET_VAL: return "PIN_GET_VAL";
	case DLN2_GPIO_PIN_SET_OUT_VAL: return "PIN_SET_OUT_VAL";
	case DLN2_GPIO_PIN_GET_OUT_VAL: return "PIN_GET_OUT_VAL";
	case DLN2_GPIO_CONDITION_MET_EV: return "CONDITION_MET_EV";
	case DLN2_GPIO_PIN_ENABLE: return "PIN_ENABLE";
	case DLN2_GPIO_PIN_DISABLE: return "PIN_DISABLE";
	case DLN2_GPIO_PIN_SET_DIRECTION: return "PIN_SET_DIRECTION";
	case DLN2_GPIO_PIN_GET_DIRECTION: return "PIN_GET_DIRECTION";
	case DLN2_GPIO_PIN_SET_EVENT_CFG: return "PIN_SET_EVENT_CFG";
	case DLN2_GPIO_PIN_GET_EVENT_CFG: return "PIN_GET_EVENT_CFG"; /* Unused by kernel driver */
	default: return "???";
	}
	// clang-format on
}

TU_ATTR_UNUSED static const char *gpio_type2str(uint8_t type)
{
	// clang-format off
	switch (type) {
	case DLN2_GPIO_EVENT_NONE: return "NONE";
	case DLN2_GPIO_EVENT_CHANGE: return "CHANGE";
	case DLN2_GPIO_EVENT_LVL_HIGH: return "LVL_HIGH";
	case DLN2_GPIO_EVENT_LVL_LOW: return "LVL_LOW";
	case DLN2_GPIO_EVENT_CHANGE_RISING: return "CHANGE_RISING";
	case DLN2_GPIO_EVENT_CHANGE_FALLING: return "CHANGE_FALLING";
	default: return "???";
	}
	// clang-format on
}

TU_ATTR_UNUSED static const char *gpio_dir2str(uint8_t pin_dir)
{
	// clang-format off
	switch (pin_dir) {
	case DLN2_GPIO_DIRECTION_IN: return "IN";
	case DLN2_GPIO_DIRECTION_OUT: return "OUT";
	default: return "???";
	}
	// clang-format on
}

#define INVALID_PIN UINT16_MAX
#define INVALID_VAL UINT8_MAX

static bool handle_request(uint16_t cmd, uint16_t *pin, uint8_t *val)
{
	switch (cmd) {
	case DLN2_GPIO_GET_PIN_COUNT:
		/* The output value isn't a pin here, but the number of pins. */
		*pin = NUM_GPIOS;
		TU_LOG3("GPIO: Reporting pin count: %u\r\n", *pin);
		break;
	case DLN2_GPIO_PIN_GET_VAL:
		TU_VERIFY(*pin < NUM_GPIOS);
		*val = gpio_get(gpio_pins[*pin]);
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
		TU_VERIFY(*val == DLN2_GPIO_DIRECTION_IN ||
			  *val == DLN2_GPIO_DIRECTION_OUT);
		gpio_set_dir(gpio_pins[*pin], *val == DLN2_GPIO_DIRECTION_OUT);
		TU_LOG3("GPIO: Setting pin %u direction: %s\r\n", *pin,
			gpio_dir2str(*val));
		break;
	case DLN2_GPIO_PIN_GET_DIRECTION:
		TU_VERIFY(*pin < NUM_GPIOS);
		*val = gpio_get_dir(gpio_pins[*pin]) ? DLN2_GPIO_DIRECTION_OUT :
						       DLN2_GPIO_DIRECTION_IN;
		TU_LOG3("GPIO: Getting pin %u direction: %s\r\n", *pin,
			gpio_dir2str(*val));
		break;
	case DLN2_GPIO_PIN_SET_EVENT_CFG:
		/* The rising edge and falling edge triggers are not used by the kernel driver. */
		TU_VERIFY(*val == DLN2_GPIO_EVENT_NONE ||
			  *val == DLN2_GPIO_EVENT_CHANGE);
		TU_LOG3("GPIO: Setting event config for pin %u: type=%s (%u)\r\n",
			*pin, gpio_type2str(*val), *val);
		gpio_set_irq_enabled(gpio_pins[*pin],
				     GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
				     *val == DLN2_GPIO_EVENT_CHANGE);
		break;
	default:
		TU_VERIFY(false);
	}

	return true;
}

bool pp_gpio_handle_request(uint16_t cmd, uint8_t const *data_in,
			    uint16_t data_in_len, uint8_t *data_out,
			    uint16_t *data_out_len)
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

static bool gpio_id_events[TU_ARRAY_SIZE(gpio_pins)];

static bool take_pin_event(uint16_t *pin)
{
	uint16_t gpio_id;
	bool event_found = false;

	irq_set_enabled(IO_IRQ_BANK0, false);

	for (uint16_t i = 0; i < TU_ARRAY_SIZE(gpio_id_events); i++) {
		if (gpio_id_events[i] == true) {
			gpio_id_events[i] = false;
			gpio_id = i;
			event_found = true;
			break;
		}
	}

	irq_set_enabled(IO_IRQ_BANK0, true);

	if (!event_found)
		return false;

	// Get pin id from gpio_id.
	for (uint16_t i = 0; i < TU_ARRAY_SIZE(gpio_pins); i++) {
		if (gpio_id == gpio_pins[i]) {
			*pin = i;
			return true;
		}
	}

	TU_LOG1("GPIO: Got gpio irq from unmapped GPIO pin.\r\n");
	return false;
}

void gpio_process_events(void)
{
	uint16_t pin;

	if (!take_pin_event(&pin))
		return;

	// Event payload:
	//   0: u16 count
	//   2: u8 type
	//   3: u16 pin
	//   5: u8 value
	//   6
	uint8_t data[6];
	u16_to_buf_le(&data[0], 0); // Unused by kernel driver
	data[2] = 0; // Unused by kernel driver
	u16_to_buf_le(&data[3], pin);
	data[5] = gpio_get(gpio_pins[pin]);

	// unsolicited message, so no echo code
	send_message_delayed(DLN2_GPIO_CONDITION_MET_EV, 0, DLN2_HANDLE_EVENT,
			     data, 6);
}

static void gpio_callback(unsigned int gpio_id, uint32_t event_mask)
{
	gpio_id_events[gpio_id] = true;

	TU_LOG3("GPIO: Pin %u has eventmask 0x%02" PRIx32 "\r\n", gpio_id,
		event_mask);
	(void)event_mask;
}

void pp_gpio_init(void)
{
	for (uint8_t i = 0; i < NUM_GPIOS; i++) {
		gpio_init(gpio_pins[i]);
	}
	gpio_set_irq_callback(gpio_callback);
	irq_set_enabled(IO_IRQ_BANK0, true);
}
