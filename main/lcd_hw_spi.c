/*
 * lcd_hw_spi.c
 *
 *  Created on: 2018Äê10ÔÂ15ÈÕ
 *      Author: Anti-
 */


#include "driver/gpio.h"
#include "lcd.h"
#include "stdlib.h"
#include "string.h"
#include "lcd_font.h"
#include "rom/ets_sys.h"
#include "driver/spi_master.h"
#include "freertos/task.h"

#if LCD_USE_HARDWARE_SPI


uint16_t BACK_COLOR = BLACK, POINT_COLOR = YELLOW;
//To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
//but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
#define PARALLEL_LINES 16


spi_device_handle_t spi;


typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;


//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[]={
    {0x36, {0x70}, 1},
    {0x3A, {0x05}, 1},
    {0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
    {0xB7, {0x35}, 1},
    {0xBB, {0x19}, 1},
    {0xC0, {0x2C}, 1},
    {0xC2, {0x01}, 1},
    {0xC3, {0x12}, 1},
    {0xC4, {0x20}, 1},
    {0xC6, {0x0f}, 1},
    {0xD0, {0xA4, 0xA1}, 2},
    {0xE0, {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23}, 14},
    {0xE1, {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23}, 14},
    {0x21, {0}, 0x80},
    {0x11, {0}, 0x80},
    {0x29, {0}, 0x80},
    {0, {0}, 0xff}
};


//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}


void hw_spi_init(void)
{
    //Initialize non-SPI GPIOs
    gpio_set_direction(19, GPIO_MODE_OUTPUT);
    gpio_set_direction(27, GPIO_MODE_OUTPUT);
    gpio_set_direction(32, GPIO_MODE_OUTPUT);
    gpio_set_direction(33, GPIO_MODE_OUTPUT);

    gpio_set_pull_mode(19, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(27, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(32, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(33, GPIO_PULLUP_ONLY);

    gpio_set_level(PIN_NUM_MOSI, 1);
    gpio_set_level(PIN_NUM_CLK, 1);
    gpio_set_level(PIN_NUM_DC, 1);
    gpio_set_level(PIN_NUM_RST, 1);

    esp_err_t ret;
    spi_bus_config_t buscfg={
        .miso_io_num=-1,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=500
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=26*1000*1000,			//Clock out at 10 MHz
        .mode=3,								//SPI mode 0
        .spics_io_num=-1,						//CS pin
        .queue_size=7,							//We want to be able to queue 7 transactions at a time
        .pre_cb=lcd_spi_pre_transfer_callback,	//Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}


//Send a command to the LCD. Uses spi_device_transmit, which waits until the transfer is complete.
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//Send data to the LCD. Uses spi_device_transmit, which waits until the transfer is complete.
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}


void lcd_write_data(const uint8_t *data, int len)
{
	lcd_data(spi, data, len);
}

//Initialize the display
void lcd_init(void)
{
    int cmd=0;

    hw_spi_init();

    //Reset the display
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);

	printf("LCD ST7789V initialization started!\n");

    //Send all the commands
    while (st_init_cmds[cmd].databytes!=0xff)
    {
        lcd_cmd(spi, st_init_cmds[cmd].cmd);
        lcd_data(spi, st_init_cmds[cmd].data, st_init_cmds[cmd].databytes&0x1F);
        if (st_init_cmds[cmd].databytes&0x80)
        {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }
}


void LCD_WR_DATA8(uint8_t value)
{
    lcd_data(spi, &value, 1);
}

void LCD_WR_DATA(uint16_t value)
{
	uint8_t temp_data[2] = {value>>8, (uint8_t)value};
    lcd_data(spi, temp_data, 2);
}

void LCD_WR_REG(uint8_t value)
{
	lcd_cmd(spi, value);
}


void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{
	lcd_init_cmd_t temp_cmd;

	temp_cmd.cmd = 0x2a;
	temp_cmd.data[0] = x1>>8;
	temp_cmd.data[1] = x1;
	temp_cmd.data[2] = x2>>8;
	temp_cmd.data[3] = x2;
	temp_cmd.databytes = 4;
    lcd_cmd(spi, temp_cmd.cmd);
    lcd_data(spi, temp_cmd.data, temp_cmd.databytes);

	temp_cmd.cmd = 0x2b;
	temp_cmd.data[0] = y1>>8;
	temp_cmd.data[1] = y1;
	temp_cmd.data[2] = y2>>8;
	temp_cmd.data[3] = y2;
	temp_cmd.databytes = 4;
    lcd_cmd(spi, temp_cmd.cmd);
    lcd_data(spi, temp_cmd.data, temp_cmd.databytes);

    lcd_cmd(spi, 0x2C);
}


void LCD_Clear(uint16_t color)
{
	uint16_t i;
	uint8_t two_byte[480];

	Address_set(0,0,240-1,240-1);
	for(i=0;i<240;i++)
	{
		two_byte[2*i] = color>>8;
		two_byte[2*i+1] = color;
	}
	for(i=0;i<240;i++)
	{
		lcd_data(spi, two_byte, 480);
	}
}

void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{
	uint16_t i;
	uint16_t width = xend - xsta + 1;
	uint8_t two_byte[480];

	Address_set(xsta,ysta,xend,yend);
	for(i=0;i<width;i++)
	{
		two_byte[2*i] = color>>8;
		two_byte[2*i+1] = color;
	}
	for(i=ysta;i<yend+1;i++)
	{
		lcd_data(spi, two_byte, width*2);
	}
}


void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	Address_set(x,y,x,y);
	LCD_WR_DATA(POINT_COLOR);
}


void LCD_DrawPoint_big(uint16_t x,uint16_t y)
{
	LCD_Fill(x-1,y-1,x+1,y+1,POINT_COLOR);
}


void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t;
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;

	delta_x=x2-x1;
	delta_y=y2-y1;
	uRow=x1;
	uCol=y1;
	if(delta_x>0)incx=1;
	else if(delta_x==0)incx=0;
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if(delta_y==0)incy=0;
	else{incy=-1;delta_y=-delta_y;}
	if( delta_x>delta_y)distance=delta_x;
	else distance=delta_y;
	for(t=0;t<=distance+1;t++ )
	{
		LCD_DrawPoint(uRow,uCol);
		xerr+=delta_x ;
		yerr+=delta_y ;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}


void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}


void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;
	di=3-(r<<1);
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a);             //3
		LCD_DrawPoint(x0+b,y0-a);             //0
		LCD_DrawPoint(x0-a,y0+b);             //1
		LCD_DrawPoint(x0-b,y0-a);             //7
		LCD_DrawPoint(x0-a,y0-b);             //2
		LCD_DrawPoint(x0+b,y0+a);             //4
		LCD_DrawPoint(x0+a,y0-b);             //5
		LCD_DrawPoint(x0+a,y0+b);             //6
		LCD_DrawPoint(x0-b,y0+a);
		a++;

		if(di<0)di +=4*a+6;
		else
		{
			di+=10+4*(a-b);
			b--;
		}
		LCD_DrawPoint(x0+a,y0+b);
	}
}


void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num)
{
	uint8_t spi_buffer_tx[256];
	uint8_t *p = spi_buffer_tx;
	uint8_t temp;
    uint8_t pos,t;
	uint16_t x0=x;
	uint16_t colortemp;
    if(x>LCD_W-16||y>LCD_H-16)
    	return;

	num=num-' ';
	Address_set(x,y,x+8-1,y+16-1);

	for(pos=0;pos<16;pos++)
	{
		temp=asc2_1608[(uint16_t)num*16+pos];
		for(t=0;t<8;t++)
		{
			if(temp&0x01)
				colortemp=POINT_COLOR;
			else
				colortemp=BACK_COLOR;
			*p++ = colortemp>>8;
			*p++ = colortemp;
			temp>>=1;
			x++;
		}
		x=x0;
		y++;
	}
	lcd_data(spi, spi_buffer_tx, 256);
}


uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;
	while(n--)
		result*=m;
	return result;
}


void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len)
{
	uint8_t t,temp;
	uint8_t enshow=0;

	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow == 0 && t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+8*t, y, ' ');
				continue;
			}
			else
				enshow=1;

		}
	 	LCD_ShowChar(x+8*t, y, temp+'0');
	}
}


void LCD_Show2Num(uint16_t x,uint16_t y,uint16_t num,uint8_t len)
{
	uint8_t t,temp;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
	 	LCD_ShowChar(x+8*t,y,temp+'0');
	}
}


void LCD_ShowString(uint16_t x,uint16_t y,const uint8_t *p)
{
    while(*p!='\0')
    {
        if(x>LCD_W-16){x=0;y+=16;}
        if(y>LCD_H-16){y=x=0;LCD_Clear(RED);}
        LCD_ShowChar(x,y,*p);
        x+=8;
        p++;
    }
}
#endif
