#ifndef __LCD_H__
#define __LCD_H__

#include "driver/gpio.h"
#include "stdlib.h"	   
#include "stdint.h"	 

#define LCD_USE_HARDWARE_SPI		1


#define LCD_W		240
#define LCD_H		240


#define PIN_NUM_MOSI 33
#define PIN_NUM_CLK  32
#define PIN_NUM_DC   19
#define PIN_NUM_RST  27


//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000
#define BLUE         	 0x001F
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色

#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)


//-----------------OLED端口定义----------------  					   
#define OLED_SCLK_Clr()		GPIO.out1_w1tc.data = 1		//CLK
#define OLED_SCLK_Set()		GPIO.out1_w1ts.data = 1

#define OLED_SDIN_Clr()		GPIO.out1_w1tc.data = 2		//DIN
#define OLED_SDIN_Set()		GPIO.out1_w1ts.data = 2

#define OLED_RST_Clr()		GPIO.out_w1tc = 0x8000000	//RES
#define OLED_RST_Set()		GPIO.out_w1ts = 0x8000000

#define OLED_DC_Clr()		GPIO.out_w1tc = 0x80000		//DC
#define OLED_DC_Set()		GPIO.out_w1ts = 0x80000


extern  uint16_t BACK_COLOR, POINT_COLOR;   //背景色，画笔色

void lcd_init(void);
void LCD_Clear(uint16_t color);
void LCD_DrawPoint(uint16_t x,uint16_t y);//画点
void LCD_DrawPoint_big(uint16_t x,uint16_t y);//画一个大点
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		   
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num);//显示一个字符
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len);//显示数字
void LCD_Show2Num(uint16_t x,uint16_t y,uint16_t num,uint8_t len);//显示2个数字
void LCD_ShowString(uint16_t x,uint16_t y,const uint8_t *p);		 //显示一个字符串,16字体
 
void showhanzi(unsigned int x,unsigned int y,unsigned char index);




					  		 
#endif  
	 
	 



