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

/************ global variables ************/
const int SCAN_RESULT_BIT = BIT0;				//定义事件，占用事件变量的第0位，最多可以定义32个事件。
const int LCD_BLE_UPDATE_BIT = BIT1;
const int LCD_NUM_UPDATE_BIT = BIT2;
const int LCD_WIFI_UPDATE_BIT = BIT3;
const int WIFI_TASK_START_BIT = BIT4;
const int WIFI_TASK_STOP_BIT = BIT5;
const int BLE_TASK_START_BIT = BIT6;
const int BLE_TASK_STOP_BIT = BIT7;
const int SD_CARD_TASK_START_BIT = BIT8;
const int SD_CARD_TASK_STOP_BIT = BIT9;

EventGroupHandle_t ble_event_group;	//定义一个事件的句柄
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;


/************ local variables ************/
uint32_t test_num = 1000;


void user_button_evt_handler(uint8_t button_evt[BUTTON_NUM])
{
	if(button_evt[BUTTON_UP] == BUTTON_EVT_PRESSED_UP)
	{
		test_num++;
	}
	if(button_evt[BUTTON_DOWN] == BUTTON_EVT_PRESSED_UP)
	{
		test_num--;
	}
	if(button_evt[BUTTON_LEFT] == BUTTON_EVT_PRESSED_UP)
	{
		test_num -= 10;
	}
	if(button_evt[BUTTON_RIGHT] == BUTTON_EVT_PRESSED_UP)
	{
		test_num += 10;
	}
	if(button_evt[BUTTON_BACK] == BUTTON_EVT_PRESSED_UP)
	{
		test_num += 100;
		xEventGroupSetBits(ble_event_group, WIFI_TASK_START_BIT);
	}
	if(button_evt[BUTTON_BOOT] == BUTTON_EVT_PRESSED_UP)
	{
		test_num -= 100;
		xEventGroupSetBits(ble_event_group, WIFI_TASK_STOP_BIT);
	}
	if(button_evt[BUTTON_MIDDLE] == BUTTON_EVT_PRESSED_UP)
	{
		test_num += 1000;
	}


	if(button_evt[BUTTON_UP] == BUTTON_EVT_HOLD_DOWN)
	{
		test_num++;
	}
	if(button_evt[BUTTON_DOWN] == BUTTON_EVT_HOLD_DOWN)
	{
		test_num--;
	}
	if(button_evt[BUTTON_LEFT] == BUTTON_EVT_HOLD_DOWN)
	{
		test_num -= 10;
	}
	if(button_evt[BUTTON_RIGHT] == BUTTON_EVT_HOLD_DOWN)
	{
		test_num += 10;
	}
	if(button_evt[BUTTON_BACK] == BUTTON_EVT_HOLD_DOWN)
	{
		test_num += 100;
	}
	if(button_evt[BUTTON_BOOT] == BUTTON_EVT_HOLD_DOWN)
	{
		test_num -= 100;
	}


	for(uint8_t i=0;i<BUTTON_NUM;i++)
	{
		if(button_evt[i])
		{
			xEventGroupSetBits(ble_event_group, LCD_NUM_UPDATE_BIT);
			return;
		}
	}
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
    xEventGroupSetBits(ble_event_group, LCD_NUM_UPDATE_BIT);

    xTaskCreate(button_task, "button_task", configMINIMAL_STACK_SIZE, NULL, 14, NULL);

	while(1)
	{
		event_bits = xEventGroupWaitBits(ble_event_group,
				LCD_NUM_UPDATE_BIT | LCD_BLE_UPDATE_BIT | LCD_WIFI_UPDATE_BIT |
				WIFI_TASK_START_BIT | WIFI_TASK_STOP_BIT |
				BLE_TASK_START_BIT | BLE_TASK_STOP_BIT, 0, 0, portMAX_DELAY);

		if(event_bits & LCD_NUM_UPDATE_BIT)
		{
//			LCD_ShowNum(50, 200, test_num, 10);//显示数字
			xEventGroupClearBits(ble_event_group, LCD_NUM_UPDATE_BIT);
		}
		if(event_bits & LCD_BLE_UPDATE_BIT)
		{
//			sprintf(print_temp, "%d devices scanned:", nodes_index);
//			LCD_ShowString(0, 0, (const uint8_t *)print_temp);
////			ble_scan_result_init();
//			ble_scan_result_print();
//			led_off();
			xEventGroupClearBits(ble_event_group, LCD_BLE_UPDATE_BIT);
		}
		if(event_bits & LCD_WIFI_UPDATE_BIT)
		{
			wifi_scan_result_print();
			xEventGroupClearBits(ble_event_group, LCD_WIFI_UPDATE_BIT);
		}


		if(event_bits & SD_CARD_TASK_START_BIT)
		{
		    xTaskCreate(sd_card_task, "sd_card_task", configMINIMAL_STACK_SIZE, &task_temp_params, 12, &task_temp_handle);
			xEventGroupClearBits(ble_event_group, SD_CARD_TASK_START_BIT);
		}
		if(event_bits & SD_CARD_TASK_STOP_BIT)
		{
			vTaskDelete(task_temp_handle);
			ble_task_mem_free();
			xEventGroupClearBits(ble_event_group, SD_CARD_TASK_STOP_BIT);
		}


		if(event_bits & BLE_TASK_START_BIT)
		{
		    xTaskCreate(ble_task, "ble_task", 4096, &task_temp_params, 12, &task_temp_handle);
			xEventGroupClearBits(ble_event_group, BLE_TASK_START_BIT);
		}
		if(event_bits & BLE_TASK_STOP_BIT)
		{
			vTaskDelete(task_temp_handle);
			ble_task_mem_free();
			xEventGroupClearBits(ble_event_group, BLE_TASK_STOP_BIT);
		}


		if(event_bits & WIFI_TASK_START_BIT)
		{
		    xTaskCreate(wifi_task, "wifi_task", 4096, &task_temp_params, 12, &task_temp_handle);
			xEventGroupClearBits(ble_event_group, WIFI_TASK_START_BIT);
		}
		if(event_bits & WIFI_TASK_STOP_BIT)
		{
			vTaskDelete(task_temp_handle);
			wifi_task_mem_free();
			xEventGroupClearBits(ble_event_group, WIFI_TASK_STOP_BIT);
		}
	}
}

