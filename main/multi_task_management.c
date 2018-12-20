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


task_manager_t task_manager;


void task_default(void *pvParameter)
{
	uint8_t button_evt[BUTTON_NUM];

	while(1)
	{
		if(xQueueReceive(button_evt_queue, button_evt, 10/portTICK_PERIOD_MS) == pdTRUE)
		{
			if(button_evt[BUTTON_UP] == BUTTON_EVT_PRESSED_UP)
			{
				task_manager.task_index_c--;
			}

			if(button_evt[BUTTON_DOWN] == BUTTON_EVT_PRESSED_UP)
			{
				task_manager.task_index_c++;
			}

			if(button_evt[BUTTON_MIDDLE] == BUTTON_EVT_PRESSED_UP)
			{
				xTaskCreate(task_manager.task[task_manager.task_index_c].task,
							task_manager.task[task_manager.task_index_c].name,
							task_manager.task[task_manager.task_index_c].usStackDepth,
							&task_manager.user_task_params,
							USER_TASK_DEFAULT_PRIORITY,
							&task_manager.user_task_handle);
				task_manager.current_display = task_manager.task[task_manager.task_index_c].display;

				vTaskSuspend(task_manager.default_task_handle);
				task_manager.current_task = task_manager.default_task;
				task_manager.current_display = task_manager.dafault_display;
				task_manager.task_index_c = 0;
				task_manager.task_index_p = MAX_TASK_NUM - 1;
				task_manager.display_status = 1;
			}

			if(task_manager.task_index_c > task_manager.task_num)
				task_manager.task_index_c = task_manager.task_num - 1;
			if(task_manager.task_index_c == task_manager.task_num)
				task_manager.task_index_c = 0;
		}
	}
}


void display_default(void)
{
	if(task_manager.display_status)
	{
		LCD_Clear(BLACK);
		for(uint8_t i=0;i<=task_manager.task_num;i++)
		{
			LCD_ShowString(	MAIN_PAGE_LINE_MARGIN,
							i*MAIN_PAGE_LINE_SPACE+MAIN_PAGE_FIRST_LINE,
							(const uint8_t *)task_manager.task[i].name);
		}
		task_manager.display_status = 0;
	}
	if(task_manager.task_index_c != task_manager.task_index_p)
	{
		LCD_ShowString(	MAIN_PAGE_CURSOR_SPACE,
						task_manager.task_index_p*MAIN_PAGE_LINE_SPACE + MAIN_PAGE_FIRST_LINE,
						(const uint8_t *)" ");
		LCD_ShowString(	MAIN_PAGE_CURSOR_SPACE,
						task_manager.task_index_c*MAIN_PAGE_LINE_SPACE+MAIN_PAGE_FIRST_LINE,
						(const uint8_t *)">");

		task_manager.task_index_p = task_manager.task_index_c;
	}
}


void task_manager_init(void)
{
	memset(&task_manager, 0, sizeof(task_manager_t));
	task_manager.task_index_p = MAX_TASK_NUM - 1;

	task_manager.default_task = task_default;
	task_manager.dafault_display = display_default;

	task_manager.current_task = task_manager.default_task;
	task_manager.current_display = task_manager.dafault_display;

	task_manager.display_status = 1;

    xTaskCreate(task_manager.default_task,
    			"default_task",
				configMINIMAL_STACK_SIZE,
				NULL,
				USER_TASK_DEFAULT_PRIORITY,
				&task_manager.default_task_handle);
}


uint8_t register_a_task(user_task_t *task_to_reg)
{
	if(task_manager.task_num == MAX_TASK_NUM-1)
		return 1;
	if(task_to_reg == NULL)
		return 2;

	memcpy(&task_manager.task[task_manager.task_num], task_to_reg, sizeof(user_task_t));
	task_manager.task_num++;

	return 0;
}


void user_task_disable(void)
{
	task_manager.task[task_manager.task_index_c].memfree();
	vTaskResume(task_manager.default_task_handle);
	vTaskDelete(task_manager.user_task_handle);
}


void user_task_lcd_dispaly(void)
{
	vTaskDelay(20 / portTICK_PERIOD_MS);
	task_manager.current_display();
}

