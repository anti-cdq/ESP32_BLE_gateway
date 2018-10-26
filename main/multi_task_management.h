/*
 * multi_task_management.h
 *
 *  Created on: 2018Äê10ÔÂ25ÈÕ
 *      Author: Linuxer
 */

#ifndef _MULTI_TASK_MANAGEMENT_H_
#define _MULTI_TASK_MANAGEMENT_H_

#define TASK_WIFI_INDEX				0
#define TASK_BLE_INDEX				1
#define TASK_SD_CARD_INDEX			2
#define TASK_MAX_INDEX				2

#define USER_TASK_STARTING			1
#define USER_TASK_RUNNING			2
#define USER_TASK_DELETING			3
#define USER_TASK_NOT_RUNNING		4


void main_page_display(uint8_t task_index);
void user_task_enable(uint8_t task_index);
void user_task_disable(uint8_t task_index);
void user_task_lcd_dispaly(uint8_t task_index);


#endif /* _MULTI_TASK_MANAGEMENT_H_ */
