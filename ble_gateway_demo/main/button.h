/*
 * button.h
 *
 *  Created on: 2018Äê10ÔÂ12ÈÕ
 *      Author: Linuxer
 */

#ifndef MAIN_BUTTON_H_
#define MAIN_BUTTON_H_


#define	BUTTON_NUM					7


#define	BUTTON_UP					0
#define	BUTTON_DOWN					1
#define	BUTTON_LEFT					2
#define	BUTTON_RIGHT				3
#define	BUTTON_BACK					4
#define	BUTTON_BOOT					5
#define	BUTTON_MIDDLE				6


#define	BUTTON_UP_IO				34
#define	BUTTON_DOWN_IO				5
#define	BUTTON_LEFT_IO				23
#define	BUTTON_RIGHT_IO				35
#define	BUTTON_BACK_IO				36
#define	BUTTON_BOOT_IO				0
#define	BUTTON_MIDDLE_IO			22


#define	BUTTON_STATE_PRESSED_UP		0x80
#define	BUTTON_STATE_PRESSED_DOWN	0x7F
#define	BUTTON_STATE_HOLD_DOWN		0xFF


#define	BUTTON_EVT_IDLE				0
#define	BUTTON_EVT_PRESSED_UP		1
#define	BUTTON_EVT_PRESSED_DOWN		2
#define	BUTTON_EVT_HOLD_DOWN		3

#define	BUTTON_HOLD_PRESCALE		2

typedef void (*button_event_handler)(uint8_t button_evt[BUTTON_NUM]);

void button_init(button_event_handler evt_handler);
void button_detect(void);


#endif /* MAIN_BUTTON_H_ */
