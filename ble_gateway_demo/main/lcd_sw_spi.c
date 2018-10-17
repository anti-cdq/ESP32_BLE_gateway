#include "driver/gpio.h"
#include "lcd.h"
#include "stdlib.h"
#include "lcd_font.h"
#include "rom/ets_sys.h"

#if !LCD_USE_HARDWARE_SPI

uint16_t BACK_COLOR = YELLOW, POINT_COLOR = BLACK;

void spi_rw(uint8_t cmd)
{
	uint8_t i=8;

	OLED_SCLK_Clr();
	while(i--)
	{
		if(cmd&0x80){OLED_SDIN_Set();}
		else{OLED_SDIN_Clr();}
		OLED_SCLK_Set();
		OLED_SCLK_Clr();
		cmd<<=1;
	}
}


void LCD_WR_DATA8(uint8_t value)
{
	OLED_DC_Set();
	spi_rw(value);
}

void LCD_WR_DATA(uint16_t value)
{
	OLED_DC_Set();
	spi_rw(value>>8);
	spi_rw(value);
}

void LCD_WR_REG(uint8_t value)
{
	OLED_DC_Clr();
	spi_rw(value);
}

void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{ 
	LCD_WR_REG(0x2a);
	LCD_WR_DATA8(x1>>8);
	LCD_WR_DATA8(x1);
	LCD_WR_DATA8(x2>>8);
	LCD_WR_DATA8(x2);

	LCD_WR_REG(0x2b);
	LCD_WR_DATA8(y1>>8);
	LCD_WR_DATA8(y1);
	LCD_WR_DATA8(y2>>8);
	LCD_WR_DATA8(y2);

	LCD_WR_REG(0x2C);					 						 
}

void lcd_init(void)
{
    gpio_set_direction(19, GPIO_MODE_OUTPUT);
    gpio_set_direction(27, GPIO_MODE_OUTPUT);
    gpio_set_direction(32, GPIO_MODE_OUTPUT);
    gpio_set_direction(33, GPIO_MODE_OUTPUT);

    gpio_set_pull_mode(19, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(27, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(32, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(33, GPIO_PULLUP_ONLY);

    OLED_SCLK_Set();
	OLED_SDIN_Set();
	OLED_RST_Set();
	OLED_DC_Set();

	ets_delay_us(10000);
	OLED_RST_Clr();
	ets_delay_us(10000);
	OLED_RST_Set();
	ets_delay_us(10000);

	/************* Start Initial Sequence **********/
	LCD_WR_REG(0x36); 
	LCD_WR_DATA8(0x70);

	LCD_WR_REG(0x3A); 
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33);

	LCD_WR_REG(0xB7); 
	LCD_WR_DATA8(0x35);  

	LCD_WR_REG(0xBB);
	LCD_WR_DATA8(0x19);

	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x2C);

	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x12);   

	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x20);  

	LCD_WR_REG(0xC6); 
	LCD_WR_DATA8(0x0F);    

	LCD_WR_REG(0xD0); 
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4C);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x23);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x51);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x23);

	LCD_WR_REG(0x21); 
	LCD_WR_REG(0x11);
	LCD_WR_REG(0x29);
}


void LCD_Clear(uint16_t color)
{
	uint16_t i,j;
	uint8_t lbyte = color>>8;
	uint8_t mbyte = color;
	
	Address_set(0,0,LCD_W-1,LCD_H-1);
	OLED_DC_Set();
	for(i=0;i<LCD_W;i++)
	{
		for (j=0;j<LCD_H;j++)
		{
			spi_rw(lbyte);
			spi_rw(mbyte);
		}
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


void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j;
	Address_set(xsta,ysta,xend,yend);
	for(i=ysta;i<=yend;i++)
	{													   	 	
		for(j=xsta;j<=xend;j++)LCD_WR_DATA(color);
	} 					  	    
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
    uint8_t temp;
    uint8_t pos,t;
	uint16_t x0=x;
	uint16_t colortemp=POINT_COLOR;
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
				colortemp = POINT_COLOR;
			else
				colortemp = BACK_COLOR;
			LCD_WR_DATA(colortemp);
			temp>>=1;
			x++;
		}
		x=x0;
		y++;
	}
}


uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;
	while(n--)result*=m;    
	return result;
}


void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;

	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow == 0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+8*t,y,' ');
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+8*t,y,temp+48);
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
