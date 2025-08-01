#ifndef _PICOPORTS_PP_CTRL_H_
#define _PICOPORTS_PP_CTRL_H_

bool pp_ctrl_handle_request(uint16_t cmd, uint8_t const *data_in, uint16_t data_in_len, uint8_t *data_out, uint16_t *data_out_len);

#endif /* _PICOPORTS_PP_CTRL_H_ */
