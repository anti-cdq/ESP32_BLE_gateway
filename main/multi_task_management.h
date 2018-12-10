/*
 * multi_task_management.h
 *
 *  Created on: 2018Äê10ÔÂ25ÈÕ
 *      Author: Linuxer
 */

#ifndef _MULTI_TASK_MANAGEMENT_H_
#define _MULTI_TASK_MANAGEMENT_H_


#define	MAX_TASK_NUM					10
#define USER_TASK_DEFAULT_PRIORITY		13

#define	MAIN_PAGE_FIRST_LINE			10
#define	MAIN_PAGE_LINE_SPACE			18
#define	MAIN_PAGE_LINE_MARGIN			50
#define	MAIN_PAGE_CURSOR_SPACE			30


typedef struct
{
	TaskFunction_t task;			/* point to task function */
	char name[20];					/* name of task function */
	uint32_t usStackDepth;			/* point to task function */
	void (*display)(void);			/* display while task running */
	void (*memfree)(void);			/* free mem after task was deleted */
}user_task_t;


typedef struct
{
	TaskFunction_t default_task;
	void (*dafault_display)(void);
	TaskFunction_t current_task;
	void (*current_display)(void);
	uint8_t task_num;				/* total task registered */
	uint8_t task_index_c;			/* current task index */
	uint8_t task_index_p;			/* previours task index */
	uint8_t task_status;			/* task status */
	TaskHandle_t task_temp_handle;
	uint32_t task_temp_params;
	user_task_t	task[MAX_TASK_NUM];	/* previours task index */
}task_manager_t;


void task_manager_init();
uint8_t register_a_task(user_task_t *task_to_reg);
void user_task_disable(void);
void user_task_lcd_dispaly(void);


#endif /* _MULTI_TASK_MANAGEMENT_H_ */
