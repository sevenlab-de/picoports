#include "tusb.h"

#include "bsp/board_api.h"

// String Descriptors

enum {
	STRID_LANGID = 0,
	STRID_MANUFACTURER,
	STRID_PRODUCT,
	STRID_SERIALNUMBER,
	STRID_IFNAME,
	STRIDS,
};

#define MAX_CHARS 32

char const *string_desc_arr[] = {
	[STRID_LANGID] = (const char[]){ 0x09, 0x04 }, // 0x0409 = English
	[STRID_MANUFACTURER] = "PicoPorts",
	[STRID_PRODUCT] = "GPIO Expander",
	[STRID_SERIALNUMBER] = NULL, // read from pico hw
	[STRID_IFNAME] = "DLN2",
};

static uint16_t _desc_str[MAX_CHARS + 1]; // +1 for header: length and type

uint16_t const *tud_descriptor_string_cb(uint8_t strid, uint16_t langid)
{
	(void)langid;
	size_t num_chars;

	memset(_desc_str, 0, sizeof(_desc_str));

	uint16_t *string_desc = &_desc_str[1];

	if (strid == STRID_LANGID) {
		memcpy(string_desc, string_desc_arr[strid], 2);
		num_chars = 1;
	} else if (strid == STRID_SERIALNUMBER) {
		num_chars = board_usb_get_serial(string_desc, MAX_CHARS);
	} else if (strid < STRIDS) {
		const char *str = string_desc_arr[strid];

		num_chars = strlen(str);
		if (num_chars > MAX_CHARS)
			num_chars = MAX_CHARS;

		// Copy ASCII string to UTF-16-LE string descriptor
		for (size_t i = 0; i < num_chars; i++)
			string_desc[i] = str[i];
	} else {
		return NULL;
	}

	uint16_t desc_len = (uint16_t)(2 * num_chars + 2);

	_desc_str[0] = TUSB_DESC_STRING << 8 | desc_len;

	return _desc_str;
}

// Device Descriptor

// clang-format off
const tusb_desc_device_t desc_device = {
	.bLength		= sizeof(tusb_desc_device_t),
	.bDescriptorType	= TUSB_DESC_DEVICE,
	.bcdUSB			= 0x0200,

	// Interfaces specify their own class, sub-class, and protocol.
	.bDeviceClass		= TUSB_CLASS_UNSPECIFIED,
	.bDeviceSubClass	= 0x00,
	.bDeviceProtocol	= 0x00,
	.bMaxPacketSize0	= CFG_TUD_ENDPOINT0_SIZE,

	.idVendor		= 0xa257,
	.idProduct		= 0x2013,
	.bcdDevice		= 0x0100,
	.iManufacturer		= STRID_MANUFACTURER,
	.iProduct		= STRID_PRODUCT,
	.iSerialNumber		= STRID_SERIALNUMBER,

	.bNumConfigurations 	= 1,
};
// clang-format on

const uint8_t *tud_descriptor_device_cb(void)
{
	return (const uint8_t *)&desc_device;
}

// Configuration & Interface Descriptors

#define TU_EDPT_ADDR(num, dir) (uint8_t)(num | (dir ? TUSB_DIR_IN_MASK : 0))

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_VENDOR_DESC_LEN)

#define EPNUM_VENDOR_OUT TU_EDPT_ADDR(0x01, TUSB_DIR_OUT)
#define EPNUM_VENDOR_IN TU_EDPT_ADDR(0x02, TUSB_DIR_IN)

const uint8_t desc_configuration[] = {
	TUD_CONFIG_DESCRIPTOR(1, 1, STRID_LANGID, CONFIG_TOTAL_LEN, 0x00, 100),
	TUD_VENDOR_DESCRIPTOR(0, STRID_IFNAME, EPNUM_VENDOR_OUT,
			      EPNUM_VENDOR_IN, TUD_OPT_HIGH_SPEED ? 512 : 64),
};

const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
	(void)index;
	return desc_configuration;
}
