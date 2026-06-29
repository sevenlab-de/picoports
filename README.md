# PicoPorts

A USB-to-GPIO/ADC/I2C/UART/SWD interface based on the Raspberry Pi Pico.

The goal of this project is to be as easy as possible to set up and use. This is achieved by

- drag and drop firmware [installation](#installation)
- no driver installation on the host*
- works with standard Linux tooling

*depends on distribution: Ubuntu: yes, Debian: [not yet](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1126062),
Arch: yes, Fedora: yes, OpenSUSE: yes

## Pinout

![Pinout](./docs/img/Pinout+Key.png)

Original image from [official Raspberry Pi documentation](https://datasheets.raspberrypi.com/pico/Pico-R3-A4-Pinout.pdf).

## Usage

- **[Usage of GPIO](./docs/usage_gpio.md)**
- **[Usage of ADC](./docs/usage_adc.md)**
- **[Usage of I2C](./docs/usage_i2c.md)**
- **[Usage of UART](./docs/usage_uart.md)**
- **[Usage of SWD](./docs/usage_swd.md)**

## Installation

1. [Download the firmware](https://github.com/sevenlab-de/picoports/releases/latest)
2. Use drag-and-drop installation
   1. Press and hold the `BOOTSEL` button on your Pico
   2. Plug the Pico into your PC, the Pico will open as thumb drive
   3. Copy the firmware onto the Pico thumb drive

A running PicoPorts device (firmware v2.2.0 or newer) can also be switched into firmware
upgrade mode [using `scripts/picoports_firmware_upgrade.py`](/docs/dev_setup.md#reset-into-boot-select-mode).

## Development

See [Development Setup](./docs/dev_setup.md)

## Acknowledgements

Many thanks to the contributors who upstreamed the `dln2` driver and to the contributors of
[debugprobe](https://github.com/raspberrypi/debugprobe/)!

## Licensing

This project is licensed under GPL-2.0-only. See [license text](./LICENSE).

## Trademark notice

Raspberry Pi is a trademark of Raspberry Pi Ltd.

PicoPorts runs on Raspberry Pi Pico.
