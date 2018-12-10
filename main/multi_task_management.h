/*
 * multi_task_management.h
 *
 *  Created on: 2018Äê10ÔÂ25ÈÕ
 *      Author: Linuxer
 */

#ifndef _MULTI_TASK_MANAGEMENT_H_
#define _MULTI_TASK_MANAGEMENT_H_

#define TASK_WIFI_INDEX					0
#define TASK_BLE_INDEX					1
#define TASK_SD_CARD_INDEX				2
#define TASK_MAX_INDEX					2

#define USER_TASK_STARTING				1
#define USER_TASK_RUNNING				2
#define USER_TASK_DELETING				3
#define USER_TASK_NOT_RUNNING			4

#define	MAX_TASK_NUM					10
#define USER_TASK_DEFAULT_PRIORITY		14

#define	MAIN_PAGE_FIRST_LINE			10
#define	MAIN_PAGE_LINE_SPACE			18
#define	MAIN_PAGE_LINE_MARGIN			50
#define	MAIN_PAGE_CURSOR_SPACE			30


typedef struct
{
	TaskFunction_t task;		/* point to task function */
	char name[10];				/* name of task function */
	uint32_t usStackDepth;		/* point to task function */
	void (*button)(void);		/* for recieving button event */
	void (*display)(void);		/* display while task running */
	void (*memfree)(void);		/* free mem after task was deleted */
}user_task_t;


typedef struct
{
	uint8_t task_num;			/* total task registered */
	uint8_t task_index_c;		/* current task index */
	uint8_t task_index_p;		/* previours task index */
	uint8_t task_status;		/* task status */
	TaskHandle_t task_temp_handle;
	uint32_t task_temp_params;
	user_task_t	task[MAX_TASK_NUM];		/* previours task index */
}task_manager_t;


void main_page_display(uint8_t task_index);
void user_task_enable(uint8_t task_index);
void user_task_disable(uint8_t task_index);
void user_task_lcd_dispaly(uint8_t task_index);


#endif /* _MULTI_TASK_MANAGEMENT_H_ */
