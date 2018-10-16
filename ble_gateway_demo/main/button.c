/*
 * button.c
 *
 *  Created on: 2018年10月12日
 *      Author: Linuxer
 */
#include <stdint.h>
#include "driver/gpio.h"
#include "button.h"

button_event_handler button_evt_handler = NULL;

uint8_t button_io_array[BUTTON_NUM] =
{
	BUTTON_UP_IO,
	BUTTON_DOWN_IO,
	BUTTON_LEFT_IO,
	BUTTON_RIGHT_IO,
	BUTTON_BACK_IO,
	BUTTON_BOOT_IO,
	BUTTON_MIDDLE_IO,
};


void button_init(button_event_handler evt_handler)
{
    gpio_set_direction(BUTTON_UP_IO, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_DOWN_IO, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_LEFT_IO, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_RIGHT_IO, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_BACK_IO, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_BOOT_IO, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_MIDDLE_IO, GPIO_MODE_INPUT);

    gpio_set_pull_mode(BUTTON_UP_IO, GPIO_FLOATING);
    gpio_set_pull_mode(BUTTON_DOWN_IO, GPIO_FLOATING);
    gpio_set_pull_mode(BUTTON_LEFT_IO, GPIO_FLOATING);
    gpio_set_pull_mode(BUTTON_RIGHT_IO, GPIO_FLOATING);
    gpio_set_pull_mode(BUTTON_BACK_IO, GPIO_FLOATING);
    gpio_set_pull_mode(BUTTON_BOOT_IO, GPIO_FLOATING);
    gpio_set_pull_mode(BUTTON_MIDDLE_IO, GPIO_FLOATING);

    button_evt_handler = evt_handler;
}


/**
 *	按键检测，10ms调用一次即可
 */
void button_detect(void)
{
	static volatile uint8_t buttons[BUTTON_NUM] = {0, 0, 0, 0, 0, 0, 0};
	static volatile uint8_t hold_cnt[BUTTON_NUM] = {0, 0, 0, 0, 0, 0, 0};
	uint8_t result[BUTTON_NUM];
	uint8_t i;

	for(i=0;i<BUTTON_NUM;i++)
	{
		result[i] = BUTTON_EVT_IDLE;
		buttons[i] <<= 1;
		if( !gpio_get_level(button_io_array[i]) )
			buttons[i] |= 0x01;

		if(buttons[i] == BUTTON_STATE_PRESSED_UP)
			result[i] = BUTTON_EVT_PRESSED_UP;
		else if(buttons[i] == BUTTON_STATE_PRESSED_DOWN)
			result[i] = BUTTON_EVT_PRESSED_DOWN;

		if(buttons[i] == BUTTON_STATE_HOLD_DOWN)
		{
			hold_cnt[i]++;
			if(hold_cnt[i] == 25)
			{
				hold_cnt[i] = 24;
				result[i] = BUTTON_EVT_HOLD_DOWN;
			}
		}
		else
			hold_cnt[i] = 0;
	}

	button_evt_handler(result);
}

