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
#include "multi_task_management.h"
#include "task_ble_scan.h"
#include "task_wifi_scan.h"
#include "task_sd_card_file_browser.h"
#include "task_button.h"


/************ global variables ************/
const int SCAN_RESULT_BIT = BIT0;				//�����¼���ռ���¼������ĵ�0λ�������Զ���32���¼���


EventGroupHandle_t ble_event_group;	//����һ���¼��ľ��
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;


/************ local variables ************/
user_task_t tasks[MAX_TASK_NUM] =
{
	{wifi_task, "wifi_task", 2048, wifi_scan_result_print, wifi_task_mem_free},
	{ble_task, "ble_task", 2048, ble_scan_result_print, ble_task_mem_free},
	{sd_card_task, "sd_card_task", 4096, sd_card_info_display, sd_card_task_mem_free},
};

void app_main()
{
	esp_err_t ret;
	int64_t temp1, temp2;

	uart_set_baudrate(UART_NUM_0, 115200);
	led_init();
	lcd_init();
	temp1 = esp_timer_get_time();
	LCD_Clear(BLACK);
	temp2 = esp_timer_get_time();
	printf("%d us\n", (int)(temp2-temp1));

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ble_event_group = xEventGroupCreate();		//����һ���¼���־��
    xTaskCreate(button_task, "button_task", configMINIMAL_STACK_SIZE, NULL, 14, NULL);

    task_manager_init();
    register_a_task(&tasks[0]);
    register_a_task(&tasks[1]);
    register_a_task(&tasks[2]);

    while(1)
	{
		user_task_lcd_dispaly();
	}
}

