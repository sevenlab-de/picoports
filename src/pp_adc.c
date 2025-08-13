#include "tusb.h"

#include "hardware/adc.h"
#include "hardware/gpio.h"

#include "byte_ops.h"
#include "dln2.h"

static const uint8_t adc_gpios[] = {
#ifdef USE_ADC_GPIOS
	26, 27, 28,
#endif
	29 // 1/3 voltage divider on VSYS
	// Channel 5 is the internal temperature sensor.
};

#define NUM_PP_ADC_CHANNELS (TU_ARRAY_SIZE(adc_gpios) + 1)
#define ADC_OFFS (NUM_ADC_CHANNELS - NUM_PP_ADC_CHANNELS)

static const char *adc_cmd2str(uint16_t cmd)
{
	// clang-format off
	switch (cmd) {
	case DLN2_ADC_GET_CHANNEL_COUNT: return "GET_CHANNEL_COUNT";
	case DLN2_ADC_ENABLE: return "ENABLE";
	case DLN2_ADC_DISABLE: return "DISABLE";
	case DLN2_ADC_CHANNEL_ENABLE: return "CHANNEL_ENABLE";
	case DLN2_ADC_CHANNEL_DISABLE: return "CHANNEL_DISABLE";
	case DLN2_ADC_SET_RESOLUTION: return "SET_RESOLUTION";
	case DLN2_ADC_CHANNEL_GET_VAL: return "CHANNEL_GET_VAL";
	case DLN2_ADC_CHANNEL_GET_ALL_VAL: return "CHANNEL_GET_ALL_VAL";
	case DLN2_ADC_CHANNEL_SET_CFG: return "CHANNEL_SET_CFG";
	case DLN2_ADC_CHANNEL_GET_CFG: return "CHANNEL_GET_CFG";
	case DLN2_ADC_CONDITION_MET_EV: return "CONDITION_MET_EV";
	default: return "???";
	}
	// clang-format on
}

bool pp_adc_handle_request(uint16_t cmd, uint8_t const *data_in,
			   uint16_t data_in_len, uint8_t *data_out,
			   uint16_t *data_out_len)
{
	switch (cmd) {
	case DLN2_ADC_GET_CHANNEL_COUNT:
		TU_ASSERT(*data_out_len >= 1);
		TU_VERIFY(data_in_len == 1);
		TU_VERIFY(data_in[0] == 0);
		TU_LOG3("ADC: Getting number of channels\r\n");
		data_out[0] = NUM_PP_ADC_CHANNELS;
		*data_out_len = 1;
		break;
	case DLN2_ADC_SET_RESOLUTION:
		TU_VERIFY(data_in_len == 2);
		TU_VERIFY(data_in[0] == 0 && data_in[1] == DLN2_ADC_DATA_BITS);
		TU_LOG3("ADC: Setting resolution\r\n");
		*data_out_len = 0;
		break;
	case DLN2_ADC_CHANNEL_ENABLE: {
		TU_VERIFY(data_in_len == 2);
		TU_VERIFY(data_in[0] == 0);
		uint8_t chan = data_in[1] + ADC_OFFS;
		TU_LOG3("ADC: Enabling channel %u\r\n", chan);
		(void)chan;
		*data_out_len = 0;
		break;
	}
	case DLN2_ADC_CHANNEL_DISABLE: {
		TU_VERIFY(data_in_len == 2);
		TU_VERIFY(data_in[0] == 0);
		uint8_t chan = data_in[1] + ADC_OFFS;
		TU_LOG3("ADC: Disabling channel %u\r\n", chan);
		(void)chan;
		*data_out_len = 0;
		break;
	}
	case DLN2_ADC_ENABLE: {
		TU_ASSERT(*data_out_len >= 2);
		TU_VERIFY(data_in_len == 1);
		TU_VERIFY(data_in[0] == 0);
		TU_LOG3("ADC: Enabling\r\n");
		u16_to_buf_le(&data_out[0], 0); // no conflict
		*data_out_len = 2;
		break;
	}
	case DLN2_ADC_DISABLE: {
		TU_ASSERT(*data_out_len >= 2);
		TU_VERIFY(data_in_len == 1);
		TU_VERIFY(data_in[0] == 0);
		TU_LOG3("ADC: Disabling\r\n");
		u16_to_buf_le(&data_out[0], 0); // no conflict
		*data_out_len = 2;
		break;
	}
	case DLN2_ADC_CHANNEL_GET_VAL: {
		TU_ASSERT(*data_out_len >= 2);
		TU_VERIFY(data_in_len == 2);
		TU_VERIFY(data_in[0] == 0);
		uint8_t chan = data_in[1] + ADC_OFFS;
		adc_select_input(chan);
		uint16_t val = adc_read();
		// Pico has 12-bit ADC, kernel driver expects 10-bit ADC
		val = val >> 2;
		TU_LOG3("ADC: Getting channel %u value: %u\r\n", chan, val);
		u16_to_buf_le(&data_out[0], val);
		*data_out_len = 2;
		break;
	}
	default:
		TU_LOG1("ADC: Command not implemented: %s (%u)\r\n",
			adc_cmd2str(cmd), cmd);
		TU_VERIFY(false);
	}

	return true;
}

void pp_adc_init(void)
{
	adc_init();
	for (size_t i = 0; i < TU_ARRAY_SIZE(adc_gpios); i++) {
		adc_gpio_init(adc_gpios[i]);
	}
	adc_set_temp_sensor_enabled(true);
}
