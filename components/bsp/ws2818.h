/*
 * ws2818.h
 *
 *  Created on: 2019Äê2ÔÂ23ÈÕ
 *      Author: Anti-
 */

#ifndef EXAMPLES_ESP32_FULL_TEST_COMPONENTS_BSP_WS2818_H_
#define EXAMPLES_ESP32_FULL_TEST_COMPONENTS_BSP_WS2818_H_

#define	PIXEL_NUM				8


typedef struct
{
	uint8_t green;
	uint8_t red;
	uint8_t blue;
}rgb_t;


void ws2818_spi_deinit(void);
void ws2818_spi_init(void);
void ws2818_update(rgb_t *rgb, uint8_t num);



#endif /* EXAMPLES_ESP32_FULL_TEST_COMPONENTS_BSP_WS2818_H_ */
