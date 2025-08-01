#ifndef _PICOPORTS_BYTE_OPS_H_
#define _PICOPORTS_BYTE_OPS_H_

inline uint16_t u16_from_buf_le(const uint8_t *buf)
{
	return buf[1] << 8 | buf[0];
}

inline uint32_t u32_from_buf_le(const uint8_t *buf)
{
	return buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
}

inline void u16_to_buf_le(uint8_t *buf, uint16_t value)
{
	buf[0] = (uint8_t)value;
	buf[1] = (uint8_t)(value >> 8);
}

inline void u32_to_buf_le(uint8_t *buf, uint32_t value)
{
	buf[0] = (uint8_t)value;
	buf[1] = (uint8_t)(value >> 8);
	buf[2] = (uint8_t)(value >> 16);
	buf[3] = (uint8_t)(value >> 24);
}

#endif /* _PICOPORTS_BYTE_OPS_H_ */
