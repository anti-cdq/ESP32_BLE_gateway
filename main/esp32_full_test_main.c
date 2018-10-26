/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "nvs.h"
#include "nvs_flash.h"


#include "global_config.h"
#include "lcd.h"
#include "led_control.h"
#include "button.h"
#include "multi_task_management.h"


/************ global variables ************/
const int SCAN_RESULT_BIT = BIT0;				//定义事件，占用事件变量的第0位，最多可以定义32个事件。
const int LCD_DISPLAY_UPDATE_BIT = BIT1;
const int SELECTED_TASK_START_BIT = BIT2;
const int SELECTED_TASK_STOP_BIT = BIT3;

EventGroupHandle_t ble_event_group;	//定义一个事件的句柄
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;


/************ local variables ************/
static int8_t task_index = TASK_WIFI_INDEX;
static int8_t user_task_status = USER_TASK_NOT_RUNNING;


void user_button_evt_handler(uint8_t button_evt[BUTTON_NUM])
{
	if(button_evt[BUTTON_UP] == BUTTON_EVT_PRESSED_UP)
	{
		if(user_task_status == USER_TASK_NOT_RUNNING)
			task_index--;
	}
	if(button_evt[BUTTON_DOWN] == BUTTON_EVT_PRESSED_UP)
	{
		if(user_task_status == USER_TASK_NOT_RUNNING)
			task_index++;
	}

	if(button_evt[BUTTON_BACK] == BUTTON_EVT_PRESSED_UP)
	{
		if(user_task_status == USER_TASK_RUNNING)
		{
			user_task_status = USER_TASK_DELETING;
			xEventGroupSetBits(ble_event_group, SELECTED_TASK_STOP_BIT);
		}
	}

	if(button_evt[BUTTON_MIDDLE] == BUTTON_EVT_PRESSED_UP)
	{
		if(user_task_status == USER_TASK_NOT_RUNNING)
		{
			user_task_status = USER_TASK_STARTING;
			xEventGroupSetBits(ble_event_group, SELECTED_TASK_START_BIT);
		}
	}

	if(task_index < 0)
		task_index = TASK_MAX_INDEX;
	if(task_index > TASK_MAX_INDEX)
		task_index = 0;
}


void button_task(void *pvParameter)
{
    button_init(user_button_evt_handler);

    while(1)
    {
    	button_detect();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}


void app_main()
{
	uint32_t event_bits;
	esp_err_t ret;

	uart_set_baudrate(UART_NUM_0, 115200);
	led_init();
	lcd_init();
	LCD_Clear(BLACK);

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ble_event_group = xEventGroupCreate();		//创建一个事件标志组
    xTaskCreate(button_task, "button_task", configMINIMAL_STACK_SIZE, NULL, 14, NULL);

	while(1)
	{
		event_bits = xEventGroupWaitBits(ble_event_group,
				LCD_DISPLAY_UPDATE_BIT  |
				SELECTED_TASK_START_BIT |
				SELECTED_TASK_STOP_BIT, 0, 0, 10/portTICK_PERIOD_MS);

		if(user_task_status == USER_TASK_NOT_RUNNING)
		{
			main_page_display(task_index);
		}

		if(event_bits & LCD_DISPLAY_UPDATE_BIT)
		{
			user_task_lcd_dispaly(task_index);
			xEventGroupClearBits(ble_event_group, LCD_DISPLAY_UPDATE_BIT);
		}

		if(event_bits & SELECTED_TASK_START_BIT)
		{
			user_task_enable(task_index);
			xEventGroupClearBits(ble_event_group, SELECTED_TASK_START_BIT);
			user_task_status = USER_TASK_RUNNING;
		}

		if(event_bits & SELECTED_TASK_STOP_BIT)
		{
			user_task_disable(task_index);
			xEventGroupClearBits(ble_event_group, SELECTED_TASK_STOP_BIT);
			LCD_Clear(BLACK);
			user_task_status = USER_TASK_NOT_RUNNING;
		}
	}
}

