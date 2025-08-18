# PicoPorts

A GPIO/ADC/I2C-Expander/Breakout via USB based on Raspberry Pi Pico 1.

## Usage

### GPIO

Example: Read GP2

```bash
gpioget gpiochip0 0
```

Example: Set GP10 high for 1 second (gpio states persist after gpioset command exits)

```bash
gpioset gpiochip0 8=1
sleep 1
gpioset gpiochip0 8=0
```

Special case: Switch on Pico LED (connected to GP25)

```bash
gpioset gpiochip0 24=1
```

The gpiochip line numbers depend on the firmware variant:

| Pico pin GP_              | GP2 | GP3 | GP4 | GP5 | GP6 | GP7 | GP8 | GP9 | GP10 | GP11 | GP12 | GP13 | GP14 | GP15 | GP16 | GP17 | GP18 | GP19 | GP20 | GP21 | GP22 | GP26 | GP27 | GP28 | GP25* |
|---------------------------|-----|-----|-----|-----|-----|-----|-----|-----|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|-------|
| gpiochip line             |   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 |    8 |    9 |   10 |   11 |   12 |   13 |    - |    - |   14 |   15 |   16 |   17 |   18 |    - |    - |    - |    19 |
| gpiochip line (GPIO-only) |   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 |    8 |    9 |   10 |   11 |   12 |   13 |   14 |   15 |   16 |   17 |   18 |   19 |   20 |   21 |   22 |   23 |    24 |

$*$ GP25 is connected to the LED on the Pico

### ADC

Example: Analog read of GP26/ADC0 (in volt)

```bash
echo "$(cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw) * $(cat /sys/bus/iio/devices/iio:device0/scale)" | bc
```

Special case: Analog read of Pico VSYS (in volt)

```bash
echo "$(cat /sys/bus/iio/devices/iio:device0/in_voltage3_raw) * $(cat /sys/bus/iio/devices/iio:device0/scale) * 3" | bc
```

Special case: Analog read of Pico onboard temperature sensor (in degree Celsius)

```bash
echo "27.0 - ($(cat /sys/bus/iio/devices/iio:device0/in_voltage4_raw) * $(cat /sys/bus/iio/devices/iio:device0/scale) - 0.706) / 0.001721" | bc
```

The voltage index (`X` in `in_voltageX_raw`) depends on the firmware variant:

| Pico Pin                  | GP26/ADC0 | GP27/ADC1 | GP28/ADC2 | VSYS/3 | int. temp. |
|---------------------------|-----------|-----------|-----------|--------|------------|
| voltage index             |         0 |         1 |         2 |      3 |          4 |
| voltage index (GPIO-only) |         - |         - |         - |      0 |          1 |

### I2C

For I2C access from the command line you can use the tools provided by the package `i2c-tools`

```bash
sudo apt install i2c-tools
```

Example: Find your I2C device number

```bash
i2cdetect -l
# Example output:
# i2c-1   i2c   dln2-i2c-3-1.1:1.0-0   I2C adapter
```

Example: Scan for device on I2C bus using read operation* (using I2C device `i2c-1`)

```bash
i2cdetect -r 1
```

$*$ I2C quick write is not supported, so detection has to use the read command (`-r` argument).

Example: Write `0x40 0xff` to the device at I2C address `0x48` (using I2C device `i2c-1`)

```bash
i2cset 1 0x48 0x40 0xff
```

Example: Read data from the device at I2C address `0x48` (using I2C device `i2c-1`)

```bash
i2cget 1 0x48
```

Example: Attach kernel driver `pcf8591` to a Philips PCF8591 ADC/DAC at I2C address `0x48` (using
I2C device `i2c-1`)

```bash
echo pcf8591 0x48 | sudo tee /sys/class/i2c-adapter/i2c-1/new_device
# Example: Read value from first ADC channel
cat /sys/class/i2c-adapter/i2c-1/1-0048/in0_input
```

I2C is only available in the full firmware variant (not in the `GPIO-only` variant).

| Pico Pin | GP16 | GP17 |
|----------|------|------|
| I2C line |  SDA |  SCL |

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
