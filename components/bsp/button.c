/*
 * button.c
 *
 *  Created on: 2018年10月12日
 *      Author: Linuxer
 */
#include <stdint.h>
#include "driver/gpio.h"
#include "button.h"


typedef struct
{
	uint16_t cnt;
	uint8_t button;
	uint8_t state		: 4;
	uint8_t click		: 4;
}button_t;


button_t buttons[BUTTON_NUM];
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
	uint8_t result[BUTTON_NUM];
	uint8_t i;

	for(i=0;i<BUTTON_NUM;i++)
	{
		result[i] = BUTTON_EVT_IDLE;
		buttons[i].button <<= 1;
		if( !gpio_get_level(button_io_array[i]) )
			buttons[i].button |= 0x01;

		if(buttons[i].state)
			buttons[i].cnt++;

		if(buttons[i].button[i] == BUTTON_STATE_PRESSED_DOWN)
		{
			result[i] = BUTTON_EVT_PRESSED_DOWN;
			if(buttons[i].state || buttons[i].cnt < BUTTON_SHORT_CLICK)
			{
				buttons[i].cnt = 0;
				buttons[i].state++;
			}
		}
		else if(buttons[i].button[i] == BUTTON_STATE_PRESSED_UP)
		{
			result[i] = BUTTON_EVT_PRESSED_UP;
			if(buttons[i].state != 0xFF)
				buttons[i].state++;
			else
				buttons[i].state = 0;
			buttons[i].cnt = 0;
		}
		else if(buttons[i].button[i] == BUTTON_STATE_HOLD_DOWN)
		{
			if(buttons[i].cnt > BUTTON_LONG_PRESS)
			{
				if(buttons[i].state == 0x01 || buttons[i].state == 0xFF)
				{
					buttons[i].state = 0xFF;
					buttons[i].cnt -= 10;
				}
				else
				{
					buttons[i].state = 0;
					buttons[i].cnt = 0;
				}
			}
		}
	}

	button_evt_handler(result);
}

