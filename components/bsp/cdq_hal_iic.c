#include "cdq_hal_iic.h"
#include "driver/gpio.h"
#include "stdlib.h"
#include "rom/ets_sys.h"


//IO方向设置
#define SDA_IN()				{gpio_set_direction(16, GPIO_MODE_INPUT);}
#define SDA_OUT()				{gpio_set_direction(16, GPIO_MODE_OUTPUT);}

//IO操作函数	 
#define IIC_SCL_H()				gpio_set_level(17, 1)
#define IIC_SCL_L()				gpio_set_level(17, 0)

#define IIC_SDA_H()				gpio_set_level(16, 1)
#define IIC_SDA_L()				gpio_set_level(16, 0)
#define READ_SDA()				(gpio_get_level(16))


//用于调试，查看从芯片是否产生应答
uint16_t ack_timeout_cnt = 0;
uint16_t ack_success_cnt = 0;


/**************************实现函数********************************************
*函数原型：void iic_gpio_init(void)
*功　　能：初始化IIC对应的引脚
*******************************************************************************/
void iic_gpio_init(void)
{			
    gpio_set_direction(17, GPIO_MODE_OUTPUT);
    gpio_set_direction(16, GPIO_MODE_OUTPUT);

    gpio_set_pull_mode(17, GPIO_FLOATING);
    gpio_set_pull_mode(16, GPIO_FLOATING);

	IIC_SCL_H();
	IIC_SDA_H();
}

/**************************实现函数********************************************
*函数原型：void iic_start(void)
*功　　能：产生IIC起始信号
*******************************************************************************/
void iic_start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA_H();
 	ets_delay_us(1);
	IIC_SCL_H();
	ets_delay_us(1);
 	IIC_SDA_L();	//START:when CLK is high,DATA change form high to low 
	ets_delay_us(1);
	IIC_SCL_L();	//钳住I2C总线，准备发送或接收数据 
}

/**************************实现函数********************************************
*函数原型：void iic_stop(void)
*功　　能：产生IIC停止信号
*******************************************************************************/	  
void iic_stop(void)
{
	SDA_OUT();	//sda线输出
	IIC_SCL_L();
	IIC_SDA_L();	//STOP:when CLK is high DATA change form low to high
 	ets_delay_us(1);
	IIC_SCL_H(); 
 	ets_delay_us(1);
	IIC_SDA_H();	//发送I2C总线结束信号
	ets_delay_us(1);
}


/**************************实现函数********************************************
*函数原型：void iic_ack(void)
*功　　能：产生ACK应答
*******************************************************************************/
void iic_ack(void)
{
	IIC_SCL_L();
	SDA_OUT();
	IIC_SDA_L();
	ets_delay_us(1);
	IIC_SCL_H();
	ets_delay_us(1);
	IIC_SCL_L();
}


/**************************实现函数********************************************
*函数原型：void iic_nack(void)
*功　　能：产生NACK应答
*******************************************************************************/	    
void iic_nack(void)
{
	IIC_SCL_L();
	SDA_OUT();
	IIC_SDA_H();
	ets_delay_us(1);
	IIC_SCL_H();
	ets_delay_us(1);
	IIC_SCL_L();
}					 				     


/**************************实现函数********************************************
*函数原型：uint8_t iic_wait_ack(void)
*功　　能：等待应答信号到来 
*返 回 值：DEBUG_ACK_TIMEOUT，接收应答失败
*          DEBUG_SUCCESS，接收应答成功
*******************************************************************************/
uint8_t iic_wait_ack(void)
{
	uint8_t ucErrTime=0;
	SDA_IN();	//SDA设置为输入  
	IIC_SDA_H();
	ets_delay_us(1);
	IIC_SCL_H();
	ets_delay_us(1);
	while(READ_SDA())
	{
		ucErrTime++;
		if(ucErrTime>100)
		{
			iic_stop();
			ack_timeout_cnt++;
			return IIC_ACK_TIMEOUT;
		}
		ets_delay_us(1);
	}
	IIC_SCL_L();	//时钟输出0 

	ack_success_cnt++;
	return IIC_SUCCESS;  
} 


/**************************实现函数********************************************
*函数原型：void iic_send_8bits(uint8_t txd)
*功　　能：IIC发送一个字节
*******************************************************************************/		  
void iic_send_8bits(uint8_t txd)
{                        
	uint8_t t;   
	SDA_OUT(); 	    
	IIC_SCL_L();	//拉低时钟开始数据传输
	for(t=0;t<8;t++)
	{              
		if( txd & 0x80 ) 
			IIC_SDA_H();
		else 
			IIC_SDA_L();
		txd <<= 1; 	  
		ets_delay_us(1);
		IIC_SCL_H();
		ets_delay_us(1);
		IIC_SCL_L();	
		ets_delay_us(1);
	}
} 	 


/**************************实现函数********************************************
*函数原型：uint8_t iic_read_8bits(uint8_t ack)
*功　　能：读1个字节
*          ack = SEND_ACK时，发送ACK
*          ack = SEND_NACK时，发送NACK
*******************************************************************************/  
uint8_t iic_read_8bits(uint8_t ack)
{
	uint8_t i,receive = 0;
	SDA_IN();	//SDA设置为输入
	for(i=0;i<8;i++ )
	{
		IIC_SCL_L(); 
		ets_delay_us(1);
		IIC_SCL_H();
		receive <<= 1;
		if(READ_SDA())
			receive++;   
		ets_delay_us(1);
	}					 
	if (ack)
		iic_ack();	//发送ACK 
	else
		iic_nack();	//发送nACK  
	return receive;
}


/**************************实现函数********************************************
*函数原型：uint8_t iic_read_bytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
*功    能：读取指定设备 指定寄存器的 length个值
*输    入：dev    目标设备地址
*          reg    寄存器地址
*          length 要读的字节数
*          *data  读出的数据将要存放的指针
*返    回：读出来的字节数量
*******************************************************************************/ 
uint8_t iic_read_bytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
{
	uint8_t count = 0;

	iic_start();
	iic_send_8bits(dev);	//发送写命令
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;
	iic_send_8bits(reg);	//发送地址
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;
	iic_start();
	iic_send_8bits(dev+1);	//进入接收模式	
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;

	for(count=0;count<length;count++)
	{
		if(count != length-1)	
			data[count] = iic_read_8bits(SEND_ACK);	//带ACK的读数据
		else	
			data[count] = iic_read_8bits(SEND_NACK);	//最后一个字节NACK
	}
	iic_stop();	//产生一个停止条件

	return IIC_SUCCESS;
}


/**************************实现函数********************************************
*函数原型：void iic_read_byte(uint8_t dev, uint8_t reg, uint8_t data)
*功    能：读取指定设备 指定寄存器的 length个值
*输    入：dev  目标设备地址
*          reg  寄存器地址
*          data 存放读出的数据
*返    回：void
*******************************************************************************/ 
uint8_t iic_read_byte(uint8_t dev, uint8_t reg, uint8_t *data)
{	
	iic_start();
	iic_send_8bits(dev);	//发送写命令
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;
	iic_send_8bits(reg);	//发送地址
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;
	iic_start();
	iic_send_8bits(dev+1);	//进入接收模式	
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;

	*data = iic_read_8bits(SEND_NACK);	//最后一个字节NACK

	iic_stop();	//产生一个停止条件
	return IIC_SUCCESS;
}


/**************************实现函数********************************************
*函数原型：uint8_t iic_write_bytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
*功　　能：将多个字节写入指定设备 指定寄存器
*输    入：dev    目标设备地址
*          reg    寄存器地址
*          length 要写的字节数
*          *data  将要写的数据的首地址
*返    回：返回是否成功
*******************************************************************************/ 
uint8_t iic_write_bytes(uint8_t dev, uint8_t reg, uint8_t length, uint8_t *data)
{
 	uint8_t count = 0;
	uint8_t *p = data;

	iic_start();
	iic_send_8bits(dev);	//发送写命令
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;
	iic_send_8bits(reg);	//发送地址
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;
	for(count=0;count<length;count++)
	{
		iic_send_8bits( *p ); 
		if(iic_wait_ack())
			return IIC_ACK_TIMEOUT;
		p++;
	}
	iic_stop();	//产生一个停止条件

	return IIC_SUCCESS;
}

/**************************实现函数********************************************
*函数原型：void iic_write_byte(uint8_t dev, uint8_t reg, uint8_t data)
*功　　能：将一个字节写入指定设备 指定寄存器
*输    入：dev    目标设备地址
*          reg    寄存器地址
*          data   将要写的数据
*返    回：返回是否成功
*******************************************************************************/ 
uint8_t iic_write_byte(uint8_t dev, uint8_t reg, uint8_t data)
{
	iic_start();
	iic_send_8bits(dev);	//发送写命令
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;
	iic_send_8bits(reg);	//发送地址
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;
	iic_send_8bits(data);	//发送数据
	if(iic_wait_ack())
		return IIC_ACK_TIMEOUT;

	iic_stop();	//产生一个停止条件
	return IIC_SUCCESS;
}


/**************************实现函数********************************************
*函数原型：void iic_addr_test(uint8_t reg, uint8_t *data)
*功　　能：尝试读写地址i上的reg
*输    入：reg    目标设备地址
*          addr    寄存器地址
*          addr_num   将要写的数据
*返    回：返回是否成功
*******************************************************************************/ 
void iic_addr_test(uint8_t reg, uint8_t *addr, uint8_t addr_num)
{
	uint8_t i;
	uint8_t temp;

	for(i=0;i!=255;i++)
	{
		if(iic_read_byte(i, reg, &temp) == IIC_SUCCESS)
			addr[(addr_num--)-1] = i;
		if(addr_num == 0)
			break;
	}
}

//------------------End of File----------------------------
