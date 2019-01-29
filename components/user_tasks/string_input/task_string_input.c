/*
 * task_string_input.c
 *
 *  Created on: 2019Äê1ÔÂ15ÈÕ
 *      Author: Anti-
 */


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


#define	ASCII_CHAR_MIN			0x20
#define	ASCII_CHAR_MAX			0x7E
#define	MAX_STRING_LEN			100


static const char *TAG = "STRING INPUT";
typedef struct
{
	uint8_t display_flag;
	uint8_t button_evt[BUTTON_NUM];
	uint8_t char_index;
	char input[MAX_STRING_LEN];
}string_input_s;

string_input_s *string_input;


void lcd_display_task_string_input(void)
{
	if(string_input == NULL)
		return;
	if(string_input->display_flag == 0)
		return;

	if(string_input->display_flag == 1)
	{
		LCD_Clear(BLACK);
		LCD_ShowString(10, 50, (const uint8_t *)"String:");
		string_input->display_flag = 2;
	}
	else if(string_input->display_flag == 2)
	{
		LCD_ShowString(66, 50, (const uint8_t *)string_input->input);
		string_input->display_flag = 0;
	}
}


void mem_free_task_string_input(void)
{
	free(string_input);
}


void task_string_input(void *pvParameter)
{
	string_input = (string_input_s *)malloc(sizeof(string_input_s));
	memset(string_input, 0, sizeof(string_input_s));

	string_input->char_index = 0;
	string_input->input[string_input->char_index] = ASCII_CHAR_MIN;
	string_input->display_flag = 1;

	for (;;)
	{
		if(xQueueReceive(button_evt_queue, string_input->button_evt, 10/portTICK_PERIOD_MS) == pdTRUE)
		{
			if(string_input->button_evt[BUTTON_BACK] == BUTTON_EVT_PRESSED_UP)
			{
				user_task_disable();
			}

			if(string_input->button_evt[BUTTON_UP] == BUTTON_EVT_PRESSED_UP)
			{
				ESP_LOGI(TAG, "pressed");
				if(string_input->input[string_input->char_index] == ASCII_CHAR_MAX)
					string_input->input[string_input->char_index] = ASCII_CHAR_MIN;
				else
					string_input->input[string_input->char_index]++;
				string_input->display_flag = 2;
			}
			if(string_input->button_evt[BUTTON_DOWN] == BUTTON_EVT_PRESSED_UP)
			{
				if(string_input->input[string_input->char_index] == ASCII_CHAR_MIN)
					string_input->input[string_input->char_index] = ASCII_CHAR_MAX;
				else
					string_input->input[string_input->char_index]--;
				string_input->display_flag = 2;
			}
			if(string_input->button_evt[BUTTON_LEFT] == BUTTON_EVT_PRESSED_UP)
			{

			}
			if(string_input->button_evt[BUTTON_RIGHT] == BUTTON_EVT_PRESSED_UP)
			{

			}
			if(string_input->button_evt[BUTTON_MIDDLE] == BUTTON_EVT_PRESSED_UP)
			{
				if(string_input->char_index < MAX_STRING_LEN)
				{
					string_input->char_index++;
					string_input->input[string_input->char_index] = ASCII_CHAR_MIN;
				}
			}
			if(string_input->button_evt[BUTTON_BOOT] == BUTTON_EVT_PRESSED_UP)
			{
				printf("String:%s\n", string_input->input);
				memset(string_input, 0, sizeof(string_input_s));
			}
		}
	}
}


