/*
 * wifi_task.c
 *
 *  Created on: 2018Äê10ÔÂ17ÈÕ
 *      Author: Anti-
 */




/* Scan Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
    This example shows how to use the All Channel Scan or Fast Scan to connect
    to a Wi-Fi network.

    In the Fast Scan mode, the scan will stop as soon as the first network matching
    the SSID is found. In this mode, an application can set threshold for the
    authentication mode and the Signal strength. Networks that do not meet the
    threshold requirements will be ignored.

    In the All Channel Scan mode, the scan will end only after all the channels
    are scanned, and connection will start with the best network. The networks
    can be sorted based on Authentication Mode or Signal Strength. The priority
    for the Authentication mode is:  WPA2 > WPA > WEP > Open
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "lcd.h"
#include "global_config.h"
#include "button.h"
#include "multi_task_management.h"


/*Set the SSID and Password via "make menuconfig"*/
#define DEFAULT_SSID	0
#define DEFAULT_PWD		0

#if CONFIG_WIFI_ALL_CHANNEL_SCAN
	#define DEFAULT_SCAN_METHOD		WIFI_ALL_CHANNEL_SCAN
#elif CONFIG_WIFI_FAST_SCAN
	#define DEFAULT_SCAN_METHOD		WIFI_FAST_SCAN
#else
	#define DEFAULT_SCAN_METHOD		WIFI_FAST_SCAN
#endif /*CONFIG_SCAN_METHOD*/

#if CONFIG_WIFI_CONNECT_AP_BY_SIGNAL
	#define DEFAULT_SORT_METHOD		WIFI_CONNECT_AP_BY_SIGNAL
#elif CONFIG_WIFI_CONNECT_AP_BY_SECURITY
	#define DEFAULT_SORT_METHOD		WIFI_CONNECT_AP_BY_SECURITY
#else
	#define DEFAULT_SORT_METHOD		WIFI_CONNECT_AP_BY_SIGNAL
#endif /*CONFIG_SORT_METHOD*/

#if CONFIG_FAST_SCAN_THRESHOLD
	#define DEFAULT_RSSI			CONFIG_FAST_SCAN_MINIMUM_SIGNAL

	#if CONFIG_EXAMPLE_OPEN
		#define DEFAULT_AUTHMODE 	WIFI_AUTH_OPEN
	#elif CONFIG_EXAMPLE_WEP
		#define DEFAULT_AUTHMODE	WIFI_AUTH_WEP
	#elif CONFIG_EXAMPLE_WPA
		#define DEFAULT_AUTHMODE	WIFI_AUTH_WPA_PSK
	#elif CONFIG_EXAMPLE_WPA2
		#define DEFAULT_AUTHMODE	WIFI_AUTH_WPA2_PSK
	#else
		#define DEFAULT_AUTHMODE	WIFI_AUTH_OPEN
	#endif
#else
	#define DEFAULT_RSSI			-127
	#define DEFAULT_AUTHMODE		WIFI_AUTH_OPEN
#endif /*CONFIG_FAST_SCAN_THRESHOLD*/


#define MAX_WIFI_NUM			20
static const char *TAG = "WIFI TASK";
static uint8_t preinit_flag = 0;
static uint8_t scan_status = 0;

typedef struct
{
	uint16_t scan_ap_num;
	uint8_t display_flag;
	uint8_t scan_flag;
	wifi_ap_record_t scan_result[MAX_WIFI_NUM];
	char print_temp[40];
	uint8_t button_evt[BUTTON_NUM];
}wifi_scan_s;

wifi_scan_s *wifi_scan;


wifi_scan_config_t wifi_scan_config =
{
	.ssid = NULL,
	.bssid = NULL,
	.channel = 0,
	.show_hidden = 1
};


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            break;
        case SYSTEM_EVENT_SCAN_DONE:
        	if(scan_status)
        	{
				wifi_scan->scan_flag = 1;
        	}
			break;
        default:
            break;
    }
    return ESP_OK;
}


void lcd_display_task_wifi_scan(void)
{
	uint16_t i;

	if(wifi_scan == NULL)
		return;
	if(wifi_scan->display_flag == 0)
		return;
	sprintf(wifi_scan->print_temp, "Scan done.%d AP scanned:", wifi_scan->scan_ap_num);
	LCD_ShowString(0, 0, (const uint8_t *)wifi_scan->print_temp);

	for(i=0;i<wifi_scan->scan_ap_num;i++)
	{
		if(wifi_scan == NULL)
			return;
		sprintf(wifi_scan->print_temp, " %20.20s | %d", wifi_scan->scan_result[i].ssid, wifi_scan->scan_result[i].rssi);
//		sprintf(ap_mac, "%02X:%02X:%02X:%02X:%02X:%02X"
//						, scan_result[i].bssid[0]
//						, scan_result[i].bssid[1]
//						, scan_result[i].bssid[2]
//						, scan_result[i].bssid[3]
//						, scan_result[i].bssid[4]
//						, scan_result[i].bssid[5]);
//		printf(" %-18.18s| ", ap_mac);
		printf("%s\n", wifi_scan->print_temp);

		if(i<14)
		{
			LCD_ShowString(8, 16*i+16, (const uint8_t *)wifi_scan->print_temp);
		}
	}
	if(i<=14)
	{
		LCD_Fill(0, 16*i+16, 239, 239, BLACK);
	}
	ESP_ERROR_CHECK(esp_wifi_scan_start(&wifi_scan_config, 0));
	printf("Scanning...\n");
	wifi_scan->display_flag = 0;
}


void mem_free_task_wifi_scan(void)
{
	scan_status = 0;
	ESP_ERROR_CHECK(esp_wifi_scan_stop());
	ESP_ERROR_CHECK(esp_wifi_stop());
	ESP_ERROR_CHECK(esp_wifi_deinit());

	free(wifi_scan);
}


void task_wifi_scan(void *pvParameter)
{
	wifi_scan = (wifi_scan_s *)malloc(sizeof(wifi_scan_s));
	memset(wifi_scan, 0, sizeof(wifi_scan_s));

	/* For esp-idf current not support deinit, init for tcpip adapter and event loop only do once. */
	if(preinit_flag == 0)
	{
		preinit_flag = 1;
		/* Initialize Wi-Fi as sta and set scan method */
		tcpip_adapter_init();
		ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	}

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));	// Set the WiFi API configuration storage type
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());

	printf("Scanning...\n");
	ESP_ERROR_CHECK(esp_wifi_scan_start(&wifi_scan_config, 0));
	scan_status = 1;

	for (;;)
	{
		if(xQueueReceive(button_evt_queue, wifi_scan->button_evt, 10/portTICK_PERIOD_MS) == pdTRUE)
		{
			if(wifi_scan->button_evt[BUTTON_BACK] == BUTTON_EVT_PRESSED_UP)
			{
				user_task_disable();
			}
		}
		if(wifi_scan->scan_flag == 1)
		{
			esp_wifi_scan_get_ap_num(&wifi_scan->scan_ap_num);
			printf("Scan done.%d AP scanned: \n", wifi_scan->scan_ap_num);
			if(wifi_scan->scan_ap_num > MAX_WIFI_NUM)
				wifi_scan->scan_ap_num = MAX_WIFI_NUM;
			ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&wifi_scan->scan_ap_num, wifi_scan->scan_result));
			wifi_scan->display_flag = 1;
			wifi_scan->scan_flag = 0;
		}
	}
}
