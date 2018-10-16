/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/



/****************************************************************************
*
* This file is for gatt client. It can scan ble device, connect one device.
* Run the gatt_server demo, the client demo will automatically connect to the gatt_server demo.
* Client demo will enable gatt_server's notify after connection. Then the two devices will exchange
* data.
*
****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"

#include "driver/uart.h"
#include "soc/uart_struct.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "lcd.h"
#include "led_control.h"
#include "button.h"

#include "globel_config.h"
#include "ble_task.h"


const int SCAN_RESULT_BIT = BIT0;				//定义事件，占用事件变量的第0位，最多可以定义32个事件。
const int LCD_BLE_UPDATE_BIT = BIT1;
const int LCD_NUM_UPDATE_BIT = BIT2;

EventGroupHandle_t ble_event_group;	//定义一个事件的句柄
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;


uint32_t test_num = 1000;
int16_t x_index = 120, x_index_p = 120;
int16_t y_index = 120, y_index_p = 120;

void user_button_evt_handler(uint8_t button_evt[BUTTON_NUM])
{
	if(button_evt[BUTTON_UP] == BUTTON_EVT_PRESSED_UP)
	{
		test_num++;
		y_index++;
	}
	if(button_evt[BUTTON_DOWN] == BUTTON_EVT_PRESSED_UP)
	{
		test_num--;
		y_index--;
	}
	if(button_evt[BUTTON_LEFT] == BUTTON_EVT_PRESSED_UP)
	{
		test_num -= 10;
		x_index++;
	}
	if(button_evt[BUTTON_RIGHT] == BUTTON_EVT_PRESSED_UP)
	{
		test_num += 10;
		x_index--;
	}

	if(button_evt[BUTTON_UP] == BUTTON_EVT_HOLD_DOWN)
	{
		test_num++;
		y_index++;
	}

	if(button_evt[BUTTON_DOWN] == BUTTON_EVT_HOLD_DOWN)
	{
		test_num--;
		y_index--;
	}
	if(button_evt[BUTTON_LEFT] == BUTTON_EVT_HOLD_DOWN)
	{
		test_num -= 10;
		x_index++;
	}
	if(button_evt[BUTTON_RIGHT] == BUTTON_EVT_HOLD_DOWN)
	{
		test_num += 10;
		x_index--;
	}

	if(button_evt[BUTTON_BACK] == BUTTON_EVT_PRESSED_UP)
	{
		test_num += 100;
	}
	if(button_evt[BUTTON_BOOT] == BUTTON_EVT_PRESSED_UP)
	{
		test_num -= 100;
	}
	if(button_evt[BUTTON_MIDDLE] == BUTTON_EVT_PRESSED_UP)
	{
		test_num += 1000;
	}

	if(x_index<1)
		x_index = 238;
	if(x_index>238)
		x_index = 1;
	if(y_index<1)
		y_index = 238;
	if(y_index>238)
		y_index = 1;

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

	led_init();
	lcd_init();
	LCD_Clear(YELLOW);

    // Initialize NVS.
	uart_set_baudrate(UART_NUM_0, 115200);

    ble_event_group = xEventGroupCreate();    //创建一个事件标志组
    xEventGroupSetBits(ble_event_group, LCD_NUM_UPDATE_BIT);
    xTaskCreate(&button_task, "button_task", configMINIMAL_STACK_SIZE, NULL, 14, NULL);

	while(1)
	{
		event_bits = xEventGroupWaitBits(ble_event_group, LCD_NUM_UPDATE_BIT | LCD_BLE_UPDATE_BIT, 0, 0, portMAX_DELAY);

		if(event_bits & LCD_NUM_UPDATE_BIT)
		{
			LCD_ShowNum(50, 200, test_num, 9);//显示数字
//			LCD_Fill(x_index_p-1, y_index_p-1, x_index_p+1, y_index_p+1, YELLOW);
//			LCD_DrawPoint_big(x_index, y_index);
//			x_index_p = x_index;
//			y_index_p = y_index;
			xEventGroupClearBits(ble_event_group, LCD_NUM_UPDATE_BIT);
		}
		if(event_bits & LCD_BLE_UPDATE_BIT)
		{
			sprintf(print_temp, "%d devices scanned:", nodes_index);
			LCD_ShowString(0, 0, (const uint8_t *)print_temp);
//			scan_result_nodes_init();
			scan_result_nodes_print();
			led_off();
			xEventGroupClearBits(ble_event_group, LCD_BLE_UPDATE_BIT);
		}
	}
}

