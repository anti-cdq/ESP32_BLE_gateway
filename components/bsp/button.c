/*
 * button.c
 *
 *  Created on: 2018��10��12��
 *      Author: Linuxer
 */
#include <stdint.h>
#include "driver/gpio.h"
#include "button.h"


typedef struct
{
	uint16_t cnt;				//���ڸ����¼��ļ�ʱ���ж������İ��º��ɿ��Ƿ�ʱ�������ж��Ƿ�Ϊ�����ȵ�
	uint8_t button;				//���ڼ�¼������״̬��ÿbit����һ�μ��״̬��0��ʾ����1��ʾ���£����λΪ���¼��
	uint8_t state		: 4;	//���ʾû�а����¼����ڽ��У�������ʾ���£�ż����ʾ����
	uint8_t click		: 4;	//���ڼ��������������絥����˫��������
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
 *	������⣬10ms����һ�μ���
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

		if(buttons[i].button == BUTTON_STATE_PRESSED_DOWN)
		{
			result[i] = BUTTON_EVT_PRESSED_DOWN;
			if(buttons[i].state == 0 && buttons[i].cnt == 0)
			{
				buttons[i].state++;
			}
			else if(buttons[i].state || buttons[i].cnt < BUTTON_SHORT_CLICK_DOWN)
			{
				buttons[i].cnt = 0;
				buttons[i].state++;
			}
		}
		else if(buttons[i].button == BUTTON_STATE_PRESSED_UP)
		{
			result[i] = BUTTON_EVT_PRESSED_UP;
			if(buttons[i].state && buttons[i].state < 0xF)
				buttons[i].state++;
			else
				buttons[i].state = 0;
			buttons[i].cnt = 0;
		}
		else if(buttons[i].button == BUTTON_STATE_HOLD_DOWN)
		{
			if(buttons[i].cnt > BUTTON_LONG_PRESS)
			{
				if(buttons[i].state == 0x01 || buttons[i].state == 0xF)
				{
					buttons[i].state = 0xF;
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

