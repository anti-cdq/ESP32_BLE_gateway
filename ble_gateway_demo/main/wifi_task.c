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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "global_config.h"
#include "lcd.h"


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
static const char *TAG = "scan";

uint16_t *scan_ap_num = NULL;
volatile uint8_t *scan_flag = NULL;
wifi_ap_record_t *scan_result = NULL;
char *print_temp = NULL;
//wifi_scan_config_t *wifi_scan_config = NULL;
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
        	esp_wifi_scan_get_ap_num(scan_ap_num);
        	printf("Scan done.%d AP scanned: \n", *scan_ap_num);
        	if(*scan_ap_num > MAX_WIFI_NUM)
        		*scan_ap_num = MAX_WIFI_NUM;
        	*scan_flag = 1;
            break;
        default:
            break;
    }
    return ESP_OK;
}


void wifi_scan_result_print(void)
{
	uint16_t i;

	sprintf(print_temp, "Scan done.%d AP scanned:", *scan_ap_num);
	LCD_ShowString(0, 0, (const uint8_t *)print_temp);

	for(i=0;i<*scan_ap_num;i++)
	{
		sprintf(print_temp, " %20.20s | %d", scan_result[i].ssid, scan_result[i].rssi);
//				sprintf(ap_mac, "%02X:%02X:%02X:%02X:%02X:%02X"
//								, scan_result[i].bssid[0]
//								, scan_result[i].bssid[1]
//								, scan_result[i].bssid[2]
//								, scan_result[i].bssid[3]
//								, scan_result[i].bssid[4]
//								, scan_result[i].bssid[5]);
//				printf(" %-18.18s| ", ap_mac);
		printf("%s\n", print_temp);

		if(i<14)
		{
			LCD_ShowString(8, 16*i+16, (const uint8_t *)print_temp);
		}
	}
	if(i<=14)
	{
		LCD_Fill(0, 16*i, 239, 239, BLACK);
	}
	ESP_ERROR_CHECK(esp_wifi_scan_start(&wifi_scan_config, 0));
	printf("Scanning...\n");
}


void wifi_task_mem_free(void)
{
	ESP_ERROR_CHECK(esp_wifi_scan_stop());
	ESP_ERROR_CHECK(esp_wifi_stop());
	ESP_ERROR_CHECK(esp_wifi_deinit());

	free(scan_ap_num);
	free(scan_flag);
	free(scan_result);
	free(print_temp);
//	free(wifi_scan_config);
}


void wifi_task(void *pvParameter)
{
	scan_ap_num = (uint16_t *)malloc(sizeof(uint16_t));
	scan_flag = (volatile uint8_t *)malloc(sizeof(uint8_t));
	scan_result = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t)*MAX_WIFI_NUM);
	print_temp = (char *)malloc(sizeof(uint8_t)*40);
//	wifi_scan_config = (wifi_scan_config_t *)malloc(sizeof(wifi_scan_config_t));
//
//	wifi_scan_config->ssid = NULL;
//	wifi_scan_config->bssid = NULL;
//	wifi_scan_config->channel = 0;
//	wifi_scan_config->show_hidden = 1;
//	wifi_scan_config->scan_type = WIFI_SCAN_TYPE_ACTIVE;
//	wifi_scan_config->scan_time.active.min = 1000;
//	wifi_scan_config->scan_time.active.max = 2000;

	/* Initialize Wi-Fi as sta and set scan method */
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));	// Set the WiFi API configuration storage type
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    printf("Scanning...\n");
    ESP_ERROR_CHECK(esp_wifi_scan_start(&wifi_scan_config, 1));

    for (;;)
    {
    	if(*scan_flag == 1)
    	{
			ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(scan_ap_num, scan_result));
			xEventGroupSetBits(ble_event_group, LCD_WIFI_UPDATE_BIT);
			*scan_flag = 0;
    	}
    	vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
