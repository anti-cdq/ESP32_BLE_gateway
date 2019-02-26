/*
 * button.h
 *
 *  Created on: 2018��10��12��
 *      Author: Linuxer
 */

#ifndef MAIN_BUTTON_H_
#define MAIN_BUTTON_H_


#define	BUTTON_NUM							7


#define	BUTTON_UP							0
#define	BUTTON_DOWN							1
#define	BUTTON_LEFT							2
#define	BUTTON_RIGHT						3
#define	BUTTON_BACK							4
#define	BUTTON_BOOT							5
#define	BUTTON_MIDDLE						6


#define BUTTON_EVT_MASK						0x0F
#define	BUTTON_EVT_IDLE						0x00
#define	BUTTON_EVT_PRESSED_UP				0x10
#define	BUTTON_EVT_PRESSED_DOWN				0x20
#define	BUTTON_EVT_SINGLE_CLICK				0x01
#define	BUTTON_EVT_DOUBLE_CLICK				0x02
#define	BUTTON_EVT_TRIPLE_CLICK				0x03
#define	BUTTON_EVT_HOLD_DOWN				0xFF


typedef void (*button_event_handler)(uint8_t button_evt[BUTTON_NUM]);

void button_init(button_event_handler evt_handler);
void button_detect(void);


#endif /* MAIN_BUTTON_H_ */
