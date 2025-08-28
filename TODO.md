# TODO

- GPIO
  - `DLN2_GPIO_PIN_GET_OUT_VAL`: How to test? Not supported by gpiod tools.
  - `DLN2_GPIO_SET_DEBOUNCE`: How to test? Not supported by gpiod tools.
  - Bug: We're dropping edge triggers on the device:

    ```text
    event: FALLING EDGE offset: 0 timestamp: [    4431.037246200]
    event: FALLING EDGE offset: 1 timestamp: [    4431.044283548]
    event:  RISING EDGE offset: 0 timestamp: [    4431.113954651]
    event:  RISING EDGE offset: 1 timestamp: [    4431.127775481]
    event: FALLING EDGE offset: 0 timestamp: [    4431.173084510]
    event:  RISING EDGE offset: 1 timestamp: [    4431.180145557]
    event:  RISING EDGE offset: 0 timestamp: [    4431.206194587]
    event:  RISING EDGE offset: 1 timestamp: [    4431.213144309]
    event: FALLING EDGE offset: 0 timestamp: [    4431.554252640]
    event: FALLING EDGE offset: 1 timestamp: [    4431.567994787]
    ```

  - Level triggered interrupts
    - `DLN2_GPIO_EVENT_LVL_HIGH`
    - `DLN2_GPIO_EVENT_LVL_LOW`
    - How to test? Not supported by gpiod tools.
    - I assume we can handle them the same way as edge triggered irqs, so we'll just send a trigger when the value changes to the target value. The only difference being that if the irq event enable is requested and the target value is already present, we should also send a trigger immediately.
- ADC
  - buffers so we can use iio-tools
- SPI
  - TODO
- UART
  - support line coding change, see `tud_cdc_line_coding_cb()`
- Add support for Pico 2
- Enable readout of the button state via a GPIO
