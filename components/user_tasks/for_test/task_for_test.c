#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"

#include "lcd.h"
#include "global_config.h"
#include "button.h"
#include "multi_task_management.h"


static const char *TAG = "TEST TASK";
typedef struct
{
	uint8_t display_flag;
	uint8_t button_evt[BUTTON_NUM];
	uint32_t counter;
}for_test_s;

for_test_s *for_test;


void lcd_display_task_for_test(void)
{
	if(for_test == NULL)
		return;
	if(for_test->display_flag == 0)
		return;

	if(for_test->display_flag == 1)
	{
		LCD_Clear(BLACK);
		LCD_DrawPoint(10, 20);//画点
		LCD_DrawPoint_big(50, 40);//画一个大点
		Draw_Circle(100, 100, 30);
		LCD_DrawLine(20, 20, 200, 100);
//		LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
		LCD_Fill(10, 200, 200, 230, YELLOW);

		for_test->display_flag = 2;
	}
	else if(for_test->display_flag == 2)
	{
		LCD_ShowNum(10, 10, for_test->counter, 10);
		for_test->display_flag = 0;
	}
}


void mem_free_task_for_test(void)
{
	free(for_test);
}


void task_for_test(void *pvParameter)
{
	for_test = (for_test_s *)malloc(sizeof(for_test_s));
	memset(for_test, 0, sizeof(for_test_s));
	for_test->display_flag = 1;

	for (;;)
	{
		if(xQueueReceive(button_evt_queue, for_test->button_evt, 10/portTICK_PERIOD_MS) == pdTRUE)
		{
			if(for_test->button_evt[BUTTON_BACK] == BUTTON_EVT_PRESSED_UP)
			{
				user_task_disable();
			}

			if(for_test->button_evt[BUTTON_UP] == BUTTON_EVT_PRESSED_UP)
			{
				for_test->counter--;
				for_test->display_flag = 2;
			}
			if(for_test->button_evt[BUTTON_DOWN] == BUTTON_EVT_PRESSED_UP)
			{
				for_test->counter++;
				for_test->display_flag = 2;
			}
			if(for_test->button_evt[BUTTON_LEFT] == BUTTON_EVT_PRESSED_UP)
			{
				for_test->counter--;
				for_test->display_flag = 2;
			}
			if(for_test->button_evt[BUTTON_RIGHT] == BUTTON_EVT_PRESSED_UP)
			{
				for_test->counter++;
				for_test->display_flag = 2;
			}
			if(for_test->button_evt[BUTTON_BOOT] == BUTTON_EVT_PRESSED_UP)
			{
				for_test->counter = 1000;
				for_test->display_flag = 2;
			}
		}
	}
}
