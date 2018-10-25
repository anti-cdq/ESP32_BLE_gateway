/*
 * multi_task_management.c
 *
 *  Created on: 2018Äê10ÔÂ25ÈÕ
 *      Author: Linuxer
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "multi_task_management.h"
#include "ble_task.h"
#include "wifi_task.h"
#include "sd_card_task.h"

TaskHandle_t task_temp_handle;
uint32_t task_temp_params;


void user_task_enable(uint8_t task_index)
{
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
}


void user_task_disable(uint8_t task_index)
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
}


void user_task_lcd_dispaly(uint8_t task_index)
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

