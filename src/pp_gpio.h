#ifndef _PICOPORTS_PP_GPIO_H_
#define _PICOPORTS_PP_GPIO_H_

void pp_gpio_init(void);
void pp_gpio_task(void);
bool pp_gpio_handle_request(uint16_t cmd, uint8_t const *data_in,
			    uint16_t data_in_len, uint8_t *data_out,
			    uint16_t *data_out_len);

#endif /* _PICOPORTS_PP_GPIO_H_ */
