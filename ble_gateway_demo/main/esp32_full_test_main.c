/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
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
#include "ble_task.h"
#include "wifi_task.h"
#include "sd_card_task.h"


#define TASK_WIFI_INDEX				0
#define TASK_BLE_INDEX				1
#define TASK_SD_CARD_INDEX			2

#define TASK_MAX_INDEX				2

/************ global variables ************/
const int SCAN_RESULT_BIT = BIT0;				//定义事件，占用事件变量的第0位，最多可以定义32个事件。
const int LCD_DISPLAY_UPDATE_BIT = BIT1;
const int SELECTED_TASK_START_BIT = BIT2;
const int SELECTED_TASK_STOP_BIT = BIT3;

EventGroupHandle_t ble_event_group;	//定义一个事件的句柄
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;


/************ local variables ************/
static int8_t task_index = TASK_WIFI_INDEX;
static int8_t is_task_running = false;
const uint8_t task_name[3][10] =
{
		{"WIFI"},
		{"BLE"},
		{"SD_CARD"},
};


void user_button_evt_handler(uint8_t button_evt[BUTTON_NUM])
{
	if(button_evt[BUTTON_UP] == BUTTON_EVT_PRESSED_UP)
	{
		if(is_task_running == false)
			task_index--;
	}
	if(button_evt[BUTTON_DOWN] == BUTTON_EVT_PRESSED_UP)
	{
		if(is_task_running == false)
			task_index++;
	}
	if(button_evt[BUTTON_LEFT] == BUTTON_EVT_PRESSED_UP)
	{

	}
	if(button_evt[BUTTON_RIGHT] == BUTTON_EVT_PRESSED_UP)
	{

	}
	if(button_evt[BUTTON_BACK] == BUTTON_EVT_PRESSED_UP)
	{
		if(is_task_running == true)
			xEventGroupSetBits(ble_event_group, SELECTED_TASK_STOP_BIT);
	}
	if(button_evt[BUTTON_BOOT] == BUTTON_EVT_PRESSED_UP)
	{

	}
	if(button_evt[BUTTON_MIDDLE] == BUTTON_EVT_PRESSED_UP)
	{
		if(is_task_running == false)
			xEventGroupSetBits(ble_event_group, SELECTED_TASK_START_BIT);
	}


	if(button_evt[BUTTON_UP] == BUTTON_EVT_HOLD_DOWN)
	{

	}
	if(button_evt[BUTTON_DOWN] == BUTTON_EVT_HOLD_DOWN)
	{

	}
	if(button_evt[BUTTON_LEFT] == BUTTON_EVT_HOLD_DOWN)
	{

	}
	if(button_evt[BUTTON_RIGHT] == BUTTON_EVT_HOLD_DOWN)
	{

	}
	if(button_evt[BUTTON_BACK] == BUTTON_EVT_HOLD_DOWN)
	{

	}
	if(button_evt[BUTTON_BOOT] == BUTTON_EVT_HOLD_DOWN)
	{

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
	char print_temp[30];
	esp_err_t ret;
	uint32_t task_temp_params;
	TaskHandle_t task_temp_handle;

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

    ble_event_group = xEventGroupCreate();    //创建一个事件标志组

    xTaskCreate(button_task, "button_task", configMINIMAL_STACK_SIZE, NULL, 14, NULL);
    xEventGroupSetBits(ble_event_group, LCD_DISPLAY_UPDATE_BIT);

	while(1)
	{
		event_bits = xEventGroupWaitBits(ble_event_group,
				LCD_DISPLAY_UPDATE_BIT  |
				SELECTED_TASK_START_BIT |
				SELECTED_TASK_STOP_BIT, 0, 0, 10/portTICK_PERIOD_MS);

		if(is_task_running == false)
		{
			for(uint8_t i=0;i<=TASK_MAX_INDEX;i++)
			{
				LCD_ShowString(50, i*20+30, &task_name[i][0]);
			}
			for(uint8_t i=0;i<=TASK_MAX_INDEX;i++)
			{
				LCD_ShowString(35, i*20+30, (const uint8_t *)" ");
			}
			LCD_ShowString(35, task_index*20+30, (const uint8_t *)">");
		}
		if(event_bits & LCD_DISPLAY_UPDATE_BIT)
		{
			if(is_task_running == true)
			{
			    switch(task_index)
			    {
					case TASK_WIFI_INDEX:
						wifi_scan_result_print();
						break;
					case TASK_BLE_INDEX:
						sprintf(print_temp, "%d devices scanned:", nodes_index);
						LCD_ShowString(0, 0, (const uint8_t *)print_temp);
						ble_scan_result_print();
						ble_scan_result_init();
						led_off();
						break;
					case TASK_SD_CARD_INDEX:
						sd_card_info_display();
						break;
					default:
						break;
			    }
			}
			xEventGroupClearBits(ble_event_group, LCD_DISPLAY_UPDATE_BIT);
		}

		if(event_bits & SELECTED_TASK_START_BIT)
		{
			is_task_running = true;
		    switch(task_index)
		    {
				case TASK_WIFI_INDEX:
				    xTaskCreate(wifi_task, "wifi_task", 2048, &task_temp_params, 14, &task_temp_handle);
					break;
				case TASK_BLE_INDEX:
				    xTaskCreate(ble_task, "ble_task", 2048, &task_temp_params, 13, &task_temp_handle);
					break;
				case TASK_SD_CARD_INDEX:
				    xTaskCreate(sd_card_task, "sd_card_task", 4096, &task_temp_params, 12, &task_temp_handle);
					break;
				default:
					break;
		    }
			xEventGroupClearBits(ble_event_group, SELECTED_TASK_START_BIT);
		}

		if(event_bits & SELECTED_TASK_STOP_BIT)
		{
		    switch(task_index)
		    {
				case TASK_WIFI_INDEX:
					wifi_task_mem_free();
					break;
				case TASK_BLE_INDEX:
					ble_task_mem_free();
					break;
				case TASK_SD_CARD_INDEX:
					sd_card_task_mem_free();
					break;
				default:
					break;
		    }

			vTaskDelete(task_temp_handle);
			xEventGroupClearBits(ble_event_group, SELECTED_TASK_STOP_BIT);
			is_task_running = false;
			LCD_Clear(BLACK);
		}
	}
}

