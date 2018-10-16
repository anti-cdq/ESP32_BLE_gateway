/*
 * led_control.c
 *
 *  Created on: 2018Äê10ÔÂ12ÈÕ
 *      Author: Anti_CDQ
 */

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"


#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (21)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0

#define LEDC_TEST_CH_NUM       (4)
#define LEDC_TEST_DUTY         (200)
#define LEDC_TEST_FADE_TIME    (3000)

ledc_timer_config_t ledc_timer = {
	.duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
	.freq_hz = 5000,                      // frequency of PWM signal
	.speed_mode = LEDC_HS_MODE,           // timer mode
	.timer_num = LEDC_HS_TIMER            // timer index
};
ledc_channel_config_t ledc_channel =
{
	.channel    = LEDC_HS_CH0_CHANNEL,
	.duty       = 0,
	.gpio_num   = LEDC_HS_CH0_GPIO,
	.speed_mode = LEDC_HS_MODE,
	.timer_sel  = LEDC_HS_TIMER
};

void led_init()
{
    ledc_timer_config(&ledc_timer);
	ledc_channel_config(&ledc_channel);
    ledc_fade_func_install(0);
}


void led_off(void)
{
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}


void led_on(void)
{
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, LEDC_TEST_DUTY);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}


void led_breath(void)
{
    ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, LEDC_TEST_DUTY, LEDC_TEST_FADE_TIME);
    ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_NO_WAIT);


    ledc_set_fade_with_time(ledc_channel.speed_mode, ledc_channel.channel, 0, LEDC_TEST_FADE_TIME);
    ledc_fade_start(ledc_channel.speed_mode, ledc_channel.channel, LEDC_FADE_NO_WAIT);
}




