#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"

#include "ws2818.h"
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
rgb_t *ws2818_rgb;


void lcd_display_task_for_test(void)
{
	if(for_test == NULL)
		return;
	if(for_test->display_flag == 0)
		return;

	if(for_test->display_flag == 1)
	{
		LCD_Clear(BLACK);
		LCD_DrawPoint(20, 150);//画点
		LCD_DrawPoint_big(40, 150);//画一个大点
		Draw_Circle(100, 100, 30);
		LCD_DrawLine(10, 150, 200, 100);
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
	free(ws2818_rgb);
}


void task_for_test(void *pvParameter)
{
	uint8_t pixel_index = 0;

	for_test = (for_test_s *)malloc(sizeof(for_test_s));
	memset(for_test, 0, sizeof(for_test_s));
	for_test->display_flag = 1;

	ws2818_spi_init();
	ws2818_rgb = (rgb_t *)malloc(sizeof(rgb_t)*PIXEL_NUM);
	memset(ws2818_rgb, 0, sizeof(rgb_t)*PIXEL_NUM);

	for (;;)
	{
		ws2818_update(ws2818_rgb, PIXEL_NUM);
		if(xQueueReceive(button_evt_queue, for_test->button_evt, 10/portTICK_PERIOD_MS) == pdTRUE)
		{
			for(uint8_t i=0;i<BUTTON_NUM;i++)
			{
				if(for_test->button_evt[i] & 0x0F)
				{
					printf("button %d: 0x%02X\n", i, for_test->button_evt[i]);
				}
			}

			if((for_test->button_evt[BUTTON_BACK]&0x0F) == BUTTON_EVT_SINGLE_CLICK)
			{
				memset(ws2818_rgb, 0, sizeof(rgb_t)*PIXEL_NUM);
				ws2818_update(ws2818_rgb, PIXEL_NUM);
				ws2818_spi_deinit();
				user_task_disable();
			}

			if((for_test->button_evt[BUTTON_UP]&0x0F) == BUTTON_EVT_SINGLE_CLICK)
			{
				for_test->counter++;
				pixel_index++;
				if(pixel_index == 0x08)
					pixel_index = 0x00;
				for_test->display_flag = 2;
			}
			if((for_test->button_evt[BUTTON_DOWN]&0x0F) == BUTTON_EVT_SINGLE_CLICK)
			{
				for_test->counter--;
				pixel_index--;
				if(pixel_index == 0xFF)
					pixel_index = 0x07;
				for_test->display_flag = 2;
			}
			if((for_test->button_evt[BUTTON_LEFT]&0x0F) == BUTTON_EVT_SINGLE_CLICK)
			{
				for_test->counter--;
				ws2818_rgb[pixel_index].green--;
				for_test->display_flag = 2;
			}
			if((for_test->button_evt[BUTTON_RIGHT]&0x0F) == BUTTON_EVT_SINGLE_CLICK)
			{
				for_test->counter++;
				ws2818_rgb[pixel_index].green++;
				for_test->display_flag = 2;
			}
			if(for_test->button_evt[BUTTON_LEFT] == BUTTON_EVT_HOLD_DOWN)
			{
				for_test->counter -= 2;
				ws2818_rgb[pixel_index].green -= 2;
				for_test->display_flag = 2;
			}
			if(for_test->button_evt[BUTTON_RIGHT] == BUTTON_EVT_HOLD_DOWN)
			{
				for_test->counter += 2;
				ws2818_rgb[pixel_index].green += 2;
				for_test->display_flag = 2;
			}

			if((for_test->button_evt[BUTTON_BOOT]&0x0F) == BUTTON_EVT_SINGLE_CLICK)
			{
				for_test->counter = 1000;
				for_test->display_flag = 2;
			}
		}
	}
}
