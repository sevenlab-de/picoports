# PicoPorts

A GPIO-Expander (GPIO-Breakout) via USB based on Raspberry Pi Pico 1.

## Usage

Switch on Pico LED:

```bash
gpioset gpiochip0 24=1
```

| gpiochip line |   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 |    8 |    9 |   10 |   11 |   12 |   13 |   14 |   15 |   16 |   17 |   18 |   19 |   20 |   21 |   22 |   23 |    24 |
|---------------|-----|-----|-----|-----|-----|-----|-----|-----|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|-------|
| Pico pin GP_  | GP2 | GP3 | GP4 | GP5 | GP6 | GP7 | GP8 | GP9 | GP10 | GP11 | GP12 | GP13 | GP14 | GP15 | GP16 | GP17 | GP18 | GP19 | GP20 | GP21 | GP22 | GP26 | GP27 | GP28 | GP25* |

$*$ GP25 is connected to the LED on the Pico

## Installation

1. [Download the firmware](https://github.com/gschwaer/picoports/releases/latest).
2. Use [Drag-and-drop installation](https://www.raspberrypi.com/documentation/microcontrollers/micropython.html#drag-and-drop-micropython).
   1. Press and hold the `BOOTSEL` button on your Pico.
   2. Plug the Pico into your PC, the Pico will open as thumb drive.
   3. Copy the firmware onto the Pico thumb drive.

## Development

### Setup

```shell
git clone --no-recurse-submodules https://github.com/gschwaer/minimal-pico-tinyusb-pio-project.git
cd minimal-pico-tinyusb-pio-project
git submodule update --init -- pico-sdk
git -C pico-sdk submodule update --init -- lib/tinyusb
```

### Build

```shell
cmake -B build
make -C build
```

### Further resources

- All one needs to know about USB: <https://www.beyondlogic.org/usbnutshell/usb1.shtml>
