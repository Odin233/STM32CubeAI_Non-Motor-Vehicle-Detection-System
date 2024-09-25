#include "sccb.h"

void delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;				//LOAD的值	    	 
	ticks=nus*72; 						//需要的节拍数 
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}  
	};
}

void SCCB_Delay(uint8_t time)
{
	delay_us(5);
}

void SCCB_SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SDA_GPIO, &GPIO_InitStruct);
}

void SCCB_SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SDA_GPIO, &GPIO_InitStruct);
}

//SCCB起始信号
//当时钟为高的时候,数据线的高到低,为SCCB起始信号
//在激活状态下,SDA和SCL均为低电平
void SCCB_Start(void)
{
	HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_SET);     //数据线高电平	   
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);	    //在时钟线高的时候数据线由高至低
	SCCB_Delay(1);  
	HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_RESET);
	SCCB_Delay(1);	 
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET);	    //数据线恢复低电平，单操作函数必要	  
}

//SCCB停止信号
//当时钟为高的时候,数据线的低到高,为SCCB停止信号
//空闲状况下,SDA,SCL均为高电平
void SCCB_Stop(void)
{
	HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_RESET);
	SCCB_Delay(1);	 
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);
	SCCB_Delay(1); 
	HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_SET);
	SCCB_Delay(1);
}

//产生NA信号
void SCCB_No_Ack(void)
{
	SCCB_Delay(1);
	HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);
	SCCB_Delay(1);
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET);
	SCCB_Delay(1);
	HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_RESET);
	SCCB_Delay(1);
}

//SCCB,写入一个字节
//返回值:0,成功;1,失败. 
uint8_t SCCB_WR_Byte(uint8_t dat)
{
	uint8_t j,res;	 
	for(j=0;j<8;j++) //循环8次发送数据
	{
		if(dat&0x80)HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_SET);
		else HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_RESET);
		dat<<=1;
		SCCB_Delay(1);
		HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);
		SCCB_Delay(1);
		HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET);
	}			 
	SCCB_SDA_IN();		//设置SDA为输入 
	SCCB_Delay(1);
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);			//接收第九位,以判断是否发送成功
	SCCB_Delay(1);
	if(HAL_GPIO_ReadPin(SDA_GPIO,SDA_PIN))res=1;  //SDA=1发送失败，返回1
	else res=0;       //SDA=0发送成功，返回0
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET);
	SCCB_SDA_OUT();		//设置SDA为输出    
	return res;  
}

//SCCB 读取一个字节
//在SCL的上升沿,数据锁存
//返回值:读到的数据
uint8_t SCCB_RD_Byte(void)
{
	uint8_t temp=0,j;    
	SCCB_SDA_IN();		//设置SDA为输入  
	for(j=8;j>0;j--) 	//循环8次接收数据
	{		     	  
		SCCB_Delay(1);
		HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);
		temp=temp<<1;
		if(HAL_GPIO_ReadPin(SDA_GPIO,SDA_PIN))temp++;   
		SCCB_Delay(1);
		HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET);
	}	
	SCCB_SDA_OUT();		//设置SDA为输出    
	return temp;
}

//写寄存器
//返回值:0,成功;1,失败.
uint8_t SCCB_WR_Reg(uint8_t reg,uint8_t data)
{
	uint8_t res=0;
	SCCB_Start(); 					//启动SCCB传输
	if(SCCB_WR_Byte(SCCB_ID))res=1;	//写器件ID	  
	delay_us(100);
  	if(SCCB_WR_Byte(reg))res=1;		//写寄存器地址	  
	delay_us(100);
  	if(SCCB_WR_Byte(data))res=1; 	//写数据	 
  	SCCB_Stop();	  
  	return	res;
}		  					    
//读寄存器
//返回值:读到的寄存器值
uint8_t SCCB_RD_Reg(uint8_t reg)
{
	uint8_t val=0;
	SCCB_Start(); 				//启动SCCB传输
	SCCB_WR_Byte(SCCB_ID);		//写器件ID	  
	delay_us(100);	 
  	SCCB_WR_Byte(reg);			//写寄存器地址	  
	delay_us(100);	  
	SCCB_Stop();   
	delay_us(100);	   
	//设置寄存器地址后，才是读
	SCCB_Start();
	SCCB_WR_Byte(SCCB_ID|0X01);	//发送读命令	  
	delay_us(100);
  	val=SCCB_RD_Byte();		 	//读取数据
  	SCCB_No_Ack();
  	SCCB_Stop();
  	return val;
}
