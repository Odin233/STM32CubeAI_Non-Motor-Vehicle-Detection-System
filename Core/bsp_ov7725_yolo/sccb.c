#include "sccb.h"

void delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;				//LOAD��ֵ	    	 
	ticks=nus*72; 						//��Ҫ�Ľ����� 
	told=SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
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

//SCCB��ʼ�ź�
//��ʱ��Ϊ�ߵ�ʱ��,�����ߵĸߵ���,ΪSCCB��ʼ�ź�
//�ڼ���״̬��,SDA��SCL��Ϊ�͵�ƽ
void SCCB_Start(void)
{
	HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_SET);     //�����߸ߵ�ƽ	   
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);	    //��ʱ���߸ߵ�ʱ���������ɸ�����
	SCCB_Delay(1);  
	HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_RESET);
	SCCB_Delay(1);	 
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET);	    //�����߻ָ��͵�ƽ��������������Ҫ	  
}

//SCCBֹͣ�ź�
//��ʱ��Ϊ�ߵ�ʱ��,�����ߵĵ͵���,ΪSCCBֹͣ�ź�
//����״����,SDA,SCL��Ϊ�ߵ�ƽ
void SCCB_Stop(void)
{
	HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_RESET);
	SCCB_Delay(1);	 
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);
	SCCB_Delay(1); 
	HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_SET);
	SCCB_Delay(1);
}

//����NA�ź�
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

//SCCB,д��һ���ֽ�
//����ֵ:0,�ɹ�;1,ʧ��. 
uint8_t SCCB_WR_Byte(uint8_t dat)
{
	uint8_t j,res;	 
	for(j=0;j<8;j++) //ѭ��8�η�������
	{
		if(dat&0x80)HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_SET);
		else HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_RESET);
		dat<<=1;
		SCCB_Delay(1);
		HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);
		SCCB_Delay(1);
		HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET);
	}			 
	SCCB_SDA_IN();		//����SDAΪ���� 
	SCCB_Delay(1);
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);			//���յھ�λ,���ж��Ƿ��ͳɹ�
	SCCB_Delay(1);
	if(HAL_GPIO_ReadPin(SDA_GPIO,SDA_PIN))res=1;  //SDA=1����ʧ�ܣ�����1
	else res=0;       //SDA=0���ͳɹ�������0
	HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET);
	SCCB_SDA_OUT();		//����SDAΪ���    
	return res;  
}

//SCCB ��ȡһ���ֽ�
//��SCL��������,��������
//����ֵ:����������
uint8_t SCCB_RD_Byte(void)
{
	uint8_t temp=0,j;    
	SCCB_SDA_IN();		//����SDAΪ����  
	for(j=8;j>0;j--) 	//ѭ��8�ν�������
	{		     	  
		SCCB_Delay(1);
		HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);
		temp=temp<<1;
		if(HAL_GPIO_ReadPin(SDA_GPIO,SDA_PIN))temp++;   
		SCCB_Delay(1);
		HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET);
	}	
	SCCB_SDA_OUT();		//����SDAΪ���    
	return temp;
}

//д�Ĵ���
//����ֵ:0,�ɹ�;1,ʧ��.
uint8_t SCCB_WR_Reg(uint8_t reg,uint8_t data)
{
	uint8_t res=0;
	SCCB_Start(); 					//����SCCB����
	if(SCCB_WR_Byte(SCCB_ID))res=1;	//д����ID	  
	delay_us(100);
  	if(SCCB_WR_Byte(reg))res=1;		//д�Ĵ�����ַ	  
	delay_us(100);
  	if(SCCB_WR_Byte(data))res=1; 	//д����	 
  	SCCB_Stop();	  
  	return	res;
}		  					    
//���Ĵ���
//����ֵ:�����ļĴ���ֵ
uint8_t SCCB_RD_Reg(uint8_t reg)
{
	uint8_t val=0;
	SCCB_Start(); 				//����SCCB����
	SCCB_WR_Byte(SCCB_ID);		//д����ID	  
	delay_us(100);	 
  	SCCB_WR_Byte(reg);			//д�Ĵ�����ַ	  
	delay_us(100);	  
	SCCB_Stop();   
	delay_us(100);	   
	//���üĴ�����ַ�󣬲��Ƕ�
	SCCB_Start();
	SCCB_WR_Byte(SCCB_ID|0X01);	//���Ͷ�����	  
	delay_us(100);
  	val=SCCB_RD_Byte();		 	//��ȡ����
  	SCCB_No_Ack();
  	SCCB_Stop();
  	return val;
}
