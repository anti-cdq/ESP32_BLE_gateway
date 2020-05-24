#ifndef __CDQ_HAL_IIC_H__
#define __CDQ_HAL_IIC_H__

#include <stdint.h>


#define IIC_SUCCESS				0x00
#define IIC_ACK_TIMEOUT			0x01

#define SEND_NACK				0x00
#define	SEND_ACK				0x01


void iic_gpio_init(void);
uint8_t iic_write_bytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data);
uint8_t iic_read_bytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data);
uint8_t iic_write_byte(uint8_t dev, uint8_t reg, uint8_t data);
uint8_t iic_read_byte(uint8_t dev, uint8_t reg, uint8_t *data);
void iic_addr_test(uint8_t reg, uint8_t *addr, uint8_t addr_num);


#endif /* __CDQ_HAL_IIC_H__ */

