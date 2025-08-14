#ifndef _PICOPORTS_PP_I2C_H_
#define _PICOPORTS_PP_I2C_H_

bool pp_i2c_handle_request(uint16_t cmd, uint8_t const *data_in,
			   uint16_t data_in_len, uint8_t *data_out,
			   uint16_t *data_out_len);

void pp_i2c_init(void);

#endif /* _PICOPORTS_PP_I2C_H_ */
