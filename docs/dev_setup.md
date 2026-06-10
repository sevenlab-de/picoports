# Development Setup

```shell
git clone --no-recurse-submodules https://github.com/sevenlab-de/picoports.git
cd picoports
git submodule update --init -- pico-sdk debugprobe
git -C pico-sdk submodule update --init -- lib/tinyusb
git -C debugprobe submodule update --init -- freertos
# Only required for Pico 2 build:
git -C debugprobe/freertos submodule update --init -- portable/ThirdParty/Community-Supported-Ports
```

## Build

```shell
cmake -B build -DPICO_BOARD=pico -DLOG_ON_GP01=no -DBOOTSEL_BUTTON=no
make -C build
# quick install (if your system uses udisks2 auto-mounting):
cp build/picoports.uf2 /media/$USER/RPI-RP2/
```

- `PICO_BOARD`: Choose between `pico` = Build for Raspberry Pi Pico (1) and
  `pico2` = Build for Raspberry Pi Pico 2. Firmware images are not compatible between
  Pico (1) and Pico 2. CMake cache needs to be cleaned before changing the board.
- `LOG_ON_GP01`: Enable debug logging on GP0/GP1, TX/RX resp. (GPIO lines will start at GP2)
- `BOOTSEL_BUTTON`: Pressing the button resets the pico into BOOTSEL mode. Enabling this impacts
  flash accesses, which interferes with the SWD capabilities. The host software may occasionally
  show warnings and errors when this is enabled.

## Reset into boot select mode

On Linux, a running PicoPorts device (with firmware version >= `v2.2.0`) can be switched into
firmware upgrade mode using `usb_modeswitch`:

```shell
usb_modeswitch -v 0xa257 -p 0x2013 -b ${BUS} -g ${DEVICE} -i 0 -m 0x01 -M 4649524d5741524555504752414445
```

The message payload is the ASCII string `FIRMWAREUPGRADE`.

A helper script is provided at `scripts/picoports_firmware_upgrade.py`. When executing this script
it tries to reset an attached PicoPorts device into firmware upgrade mode. If more than one
PicoPorts device is attached, the USB serial number must be provided.

This feature needs privileged access on the host. The udev rule `60-dln2-plugdev-access.rules`
grants this for all users in the `plugdev` group. You can install it with:

```bash
sudo cp 60-dln2-plugdev-access.rules /etc/udev/rules.d/
sudo udevadm control --reload
sudo udevadm trigger
```

## Theory of operation

PicoPorts works without a custom driver, because it's using a driver that already exists. The driver
is called `dln2` (`gpio-dln2`, `dln2-adc`, `i2c-dln2`) and was written for the Diolan DLN-2 USB
adapter. PicoPorts just implements the other side of the interface which the driver provides. It's
mainly a glue layer from this kernel interface to the Pico's SDK interface.

Many thanks to the contributors who upstreamed this driver!

Additionally, PicoPorts incorporates the [debugprobe](https://github.com/raspberrypi/debugprobe/)
project. Luckily OpenOCD does not use the vendor and product ID of the device, but scans the product
name and interface name for `"CMSIS-DAP"`, so we can keep using the vendor and product ID required
for detection by the `dln2` driver.

### What about SPI?

The SPI subsystem is troublesome in the kernel. While a `spi-dln2` driver exists, it's simply not
possible to attach a device driver to a hotpluggable SPI interface without major efforts from the
user. The only way to attach a SPI device to an interface is via Devicetree/ACPI, which is not made
to be used with hotpluggable devices like USB devices.

As a workaround, PicoPorts may in the future add a custom USB interface for SPI and add a tool which
works similarly to the `spidev` virtual device driver, but in user space using `libusb`.

## Further resources

- All one needs to know about USB: <https://www.beyondlogic.org/usbnutshell/usb1.shtml>
