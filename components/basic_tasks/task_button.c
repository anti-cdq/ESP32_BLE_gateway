/*
 * task_button.c
 *
 *  Created on: 2018Äê12ÔÂ11ÈÕ
 *      Author: Anti-
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "button.h"


xQueueHandle  button_evt_queue;

void user_button_evt_handler(uint8_t button_evt[BUTTON_NUM])
{
	uint8_t i;

	for(i=0;i<BUTTON_NUM;i++)
	{
		if(button_evt[i])
		{
			xQueueSend(button_evt_queue, button_evt, NULL);
			return;
		}
	}
}


void button_task(void *pvParameter)
{
    button_evt_queue = xQueueCreate(10, BUTTON_NUM);
    button_init(user_button_evt_handler);

    while(1)
    {
    	button_detect();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

