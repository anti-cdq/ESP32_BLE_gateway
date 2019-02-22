/*
 * ws2818.c
 *
 *  Created on: 2019Äê2ÔÂ23ÈÕ
 *      Author: Anti-
 */
#include "driver/gpio.h"
#include "stdlib.h"
#include "string.h"
#include "rom/ets_sys.h"
#include "driver/spi_master.h"
#include "ws2818.h"


spi_device_handle_t ws2818_spi;

static void ws2818_spi_pre_transfer_callback(spi_transaction_t *t)
{

}

void ws2818_spi_deinit(void)
{
	spi_bus_remove_device(ws2818_spi);
	spi_bus_free(VSPI_HOST);
}

void ws2818_spi_init(void)
{
    //Initialize non-lcd_spi GPIOs
    gpio_set_direction(18, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(18, GPIO_PULLUP_ONLY);
    gpio_set_level(18, 1);

    esp_err_t ret;
    spi_bus_config_t buscfg={
        .miso_io_num=-1,
        .mosi_io_num=18,
        .sclk_io_num=-1,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=500
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=6*1000*1000,			//Clock out at 10 MHz
        .flags=SPI_DEVICE_NO_DUMMY,				//ignore errors
		.mode=3,								//spi mode 0
        .spics_io_num=-1,						//CS pin
        .queue_size=7,							//We want to be able to queue 7 transactions at a time
        .pre_cb=ws2818_spi_pre_transfer_callback,	//Specify pre-transfer callback to handle D/C line
    };
    //Initialize the lcd_spi bus
    ret=spi_bus_initialize(VSPI_HOST, &buscfg, 2);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the lcd_spi bus
    ret=spi_bus_add_device(VSPI_HOST, &devcfg, &ws2818_spi);
    ESP_ERROR_CHECK(ret);
}


void ws2818_spi_send(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}


void ws2818_update(rgb_t *rgb, uint8_t num)
{
	uint8_t *data;
	uint8_t i, j, temp;

	data = (uint8_t *)malloc(num*24);
	for(i=0;i<num;i++)
	{
		temp = rgb[i].green;
		for(j=0;j<8;j++)
		{
			if((temp>>j) & 0x01)
				data[i*24+7-j] = 0xF8;
			else
				data[i*24+7-j] = 0xC0;
		}
		temp = rgb[i].red;
		for(j=0;j<8;j++)
		{
			if((temp>>j) & 0x01)
				data[i*24+15-j] = 0xF8;
			else
				data[i*24+15-j] = 0xC0;
		}
		temp = rgb[i].blue;
		for(j=0;j<8;j++)
		{
			if((temp>>j) & 0x01)
				data[i*24+23-j] = 0xF8;
			else
				data[i*24+23-j] = 0xC0;
		}
	}

	ws2818_spi_send(ws2818_spi, data, num*24);
	free(data);
}


