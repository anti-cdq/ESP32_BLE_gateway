/*
 * globel_config.h
 *
 *  Created on: 2018年10月17日
 *      Author: Anti-
 */

#ifndef EXAMPLES_BLE_GATEWAY_DEMO_MAIN_GLOBEL_CONFIG_H_
#define EXAMPLES_BLE_GATEWAY_DEMO_MAIN_GLOBEL_CONFIG_H_

#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/queue.h"


extern const int SCAN_RESULT_BIT;			//定义事件，占用事件变量的第0位，最多可以定义32个事件。
extern const int LCD_DISPLAY_UPDATE_BIT;
extern const int SELECTED_TASK_START_BIT;
extern const int SELECTED_TASK_STOP_BIT;

extern xQueueHandle  button_evt_queue;
extern EventGroupHandle_t ble_event_group;	//定义一个事件的句柄


void button_task(void *pvParameter);




#endif /* EXAMPLES_BLE_GATEWAY_DEMO_MAIN_GLOBEL_CONFIG_H_ */
