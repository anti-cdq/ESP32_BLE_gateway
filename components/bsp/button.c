/*
 * button.c
 *
 *  Created on: 2018年10月12日
 *      Author: Linuxer
 */
#include <stdint.h>
#include "driver/gpio.h"
#include "button.h"


#define	BUTTON_UP_IO						34
#define	BUTTON_DOWN_IO						5
#define	BUTTON_LEFT_IO						23
#define	BUTTON_RIGHT_IO						35
#define	BUTTON_BACK_IO						36
#define	BUTTON_BOOT_IO						0
#define	BUTTON_MIDDLE_IO					22


#define	BUTTON_IO_STATE_PRESSED_UP			0x80
#define	BUTTON_IO_STATE_PRESSED_DOWN		0x7F
#define	BUTTON_IO_STATE_HOLD_DOWN			0xFF
#define	BUTTON_IO_STATE_HOLD_UP				0x00


#define	BUTTON_STATE_IDLE					0
#define	BUTTON_STATE_PRESSED_UP				1
#define	BUTTON_STATE_PRESSED_DOWN			2
#define	BUTTON_STATE_LONG_PRESS				3


#define BUTTON_SHORT_CLICK_UP				20
#define BUTTON_SHORT_CLICK_DOWN				35
#define BUTTON_LONG_PRESS					50

#define	BUTTON_HOLD_PRESCALE				5


typedef struct
{
	uint16_t cnt;				//用于各个事件的计时，判断连击的按下和松开是否超时，或者判断是否为长按等等
	uint8_t io;					//用于记录按键的状态，每bit代表一次检测状态，0表示弹起，1表示按下，最低位为最新检测
	uint8_t state		: 4;	//零表示没有按键事件正在进行，1表示按下，2表示弹起, 3表示长按
	uint8_t click		: 4;	//用于计算连击数，比如单击、双击、三击
}button_t;


button_t buttons[BUTTON_NUM];
button_event_handler button_evt_handler = NULL;


const uint8_t button_io_array[BUTTON_NUM] =
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
		result[i] = BUTTON_EVT_IDLE;	//事件先清除
		buttons[i].io <<= 1;
		if( !gpio_get_level(button_io_array[i]) )
			buttons[i].io |= 0x01;

		if(buttons[i].state == BUTTON_STATE_IDLE)
		{
			buttons[i].cnt = 0;
			buttons[i].click = 0;
		}
		else
			buttons[i].cnt++;

		switch(buttons[i].io)
		{
			case BUTTON_IO_STATE_PRESSED_DOWN:				//按下
				result[i] = BUTTON_EVT_PRESSED_DOWN;
				buttons[i].cnt = 0;
				buttons[i].state = BUTTON_STATE_PRESSED_DOWN;
				break;

			case BUTTON_IO_STATE_PRESSED_UP:				//弹起
				result[i] = BUTTON_EVT_PRESSED_UP;
				if(buttons[i].state == BUTTON_STATE_PRESSED_DOWN && buttons[i].cnt < BUTTON_SHORT_CLICK_DOWN)
				{
					buttons[i].state = BUTTON_STATE_PRESSED_UP;
					buttons[i].click++;
					buttons[i].cnt = 0;
				}
				else
					buttons[i].state = BUTTON_STATE_IDLE;
				break;

			case BUTTON_IO_STATE_HOLD_DOWN:					//按住
				if(buttons[i].cnt > BUTTON_LONG_PRESS)		//降低长按触发频率
				{
					if(buttons[i].click == 0 || buttons[i].state == BUTTON_STATE_LONG_PRESS)
					{
						result[i] = BUTTON_EVT_HOLD_DOWN;
						buttons[i].state = BUTTON_STATE_LONG_PRESS;
						buttons[i].cnt -= BUTTON_HOLD_PRESCALE;
					}
					else
					{
						buttons[i].state = BUTTON_STATE_IDLE;
					}
				}
				break;

			default:
				if(buttons[i].cnt > BUTTON_SHORT_CLICK_UP && buttons[i].state == BUTTON_STATE_PRESSED_UP)
				{
					buttons[i].state = BUTTON_STATE_IDLE;
					if(buttons[i].click < 4)
						result[i] |= buttons[i].click;
				}
				break;
		}
	}

	button_evt_handler(result);
}

