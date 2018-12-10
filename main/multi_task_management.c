/*
 * multi_task_management.c
 *
 *  Created on: 2018Äê10ÔÂ25ÈÕ
 *      Author: Linuxer
 */

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "multi_task_management.h"

#include "global_config.h"
#include "lcd.h"
#include "button.h"

task_manager_t task_maneger;


void task_default(void *pvParameter)
{
	uint8_t button_evt[BUTTON_NUM];

	while(1)
	{
		if(xQueueReceive(button_evt_queue, button_evt, 5/portTICK_PERIOD_MS) == pdTRUE)
		{
			if(button_evt[BUTTON_UP] == BUTTON_EVT_PRESSED_UP)
			{
				task_maneger.task_index_c++;
			}

			if(button_evt[BUTTON_DOWN] == BUTTON_EVT_PRESSED_UP)
			{
				task_maneger.task_index_c--;
			}

			if(button_evt[BUTTON_MIDDLE] == BUTTON_EVT_PRESSED_UP)
			{
				xTaskCreate(task_maneger.task[task_maneger.task_index_c].task,
							task_maneger.task[task_maneger.task_index_c].name,
							task_maneger.task[task_maneger.task_index_c].usStackDepth,
							&task_maneger.task_temp_params,
							USER_TASK_DEFAULT_PRIORITY,
							&task_maneger.task_temp_handle);
			}

			if(task_maneger.task_index_c > task_maneger.task_num)
				task_maneger.task_index_c = task_maneger.task_num - 1;
			if(task_maneger.task_index_c == task_maneger.task_num)
				task_maneger.task_index_c = 0;
		}
	}
}


void display_default(uint8_t task_index)
{
	for(uint8_t i=0;i<=task_maneger.task_num;i++)
	{
		LCD_ShowString(	MAIN_PAGE_LINE_MARGIN,
						i*MAIN_PAGE_LINE_SPACE+MAIN_PAGE_FIRST_LINE,
						(const uint8_t *)task_maneger.task[i].name);
	}

	LCD_ShowString(	MAIN_PAGE_CURSOR_SPACE,
					task_maneger.task_index_p*MAIN_PAGE_LINE_SPACE + MAIN_PAGE_FIRST_LINE,
					(const uint8_t *)" ");
	LCD_ShowString(	MAIN_PAGE_CURSOR_SPACE,
					task_maneger.task_index_c*MAIN_PAGE_LINE_SPACE+MAIN_PAGE_FIRST_LINE,
					(const uint8_t *)">");
}


uint8_t register_a_task(user_task_t *task_to_reg)
{
	if(task_maneger.task_num == MAX_TASK_NUM-1)
		return 1;
	if(task_to_reg == NULL)
		return 2;

	memcpy(task_to_reg, &(task_maneger.task[task_maneger.task_num]), sizeof(user_task_t));
	task_maneger.task_num++;

	return 0;
}


void user_task_disable(uint8_t task_index)
{
	vTaskDelete(task_maneger.task_temp_handle);
	task_maneger.task[task_maneger.task_index_c].memfree();
}


void user_task_lcd_dispaly(uint8_t task_index)
{

}

