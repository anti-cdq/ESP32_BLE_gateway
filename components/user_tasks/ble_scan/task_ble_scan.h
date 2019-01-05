/*
 * ble_task.h
 *
 *  Created on: 2018Äê10ÔÂ17ÈÕ
 *      Author: Anti-
 */

#ifndef EXAMPLES_BLE_GATEWAY_DEMO_MAIN_BLE_TASK_H_
#define EXAMPLES_BLE_GATEWAY_DEMO_MAIN_BLE_TASK_H_

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"


extern uint32_t nodes_index;

typedef struct
{
    esp_gap_search_evt_t search_evt;            /*!< Search event type */
    esp_bd_addr_t bda;                          /*!< Bluetooth device address which has been searched */
    esp_bt_dev_type_t dev_type;                 /*!< Device type */
    esp_ble_addr_type_t ble_addr_type;          /*!< Ble device address type */
    esp_ble_evt_type_t ble_evt_type;            /*!< Ble scan result event type */
    int rssi;                                   /*!< Searched device's RSSI */
    uint8_t  ble_adv[ESP_BLE_ADV_DATA_LEN_MAX + ESP_BLE_SCAN_RSP_DATA_LEN_MAX];     /*!< Received EIR */
    int flag;                                   /*!< Advertising data flag bit */
    int num_resps;                              /*!< Scan result number */
    uint8_t adv_data_len;                       /*!< Adv data length */
    uint8_t scan_rsp_len;                       /*!< Scan response length */
}scan_rst_node_t;


void ble_scan_result_init(void);
void device_mac_add(esp_ble_gap_cb_param_t* scanned_dev);
void lcd_display_task_ble_scan(void);
void mem_free_task_ble_scan(void);
void task_ble_scan(void *pvParameter);



#endif /* EXAMPLES_BLE_GATEWAY_DEMO_MAIN_BLE_TASK_H_ */
