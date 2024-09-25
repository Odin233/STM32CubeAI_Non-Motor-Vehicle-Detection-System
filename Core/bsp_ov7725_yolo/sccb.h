#ifndef SCCB_H
#define SCCB_H

#include "stm32f4xx.h"

#define SDA_GPIO GPIOB
#define SDA_PIN GPIO_PIN_8
#define SCL_GPIO GPIOB
#define SCL_PIN GPIO_PIN_9

#define SCCB_ID   			0X42  			//OV7670µÄID

void SCCB_Start(void);
void SCCB_Stop(void);
void SCCB_No_Ack(void);
uint8_t SCCB_WR_Byte(uint8_t dat);
uint8_t SCCB_RD_Byte(void);
uint8_t SCCB_WR_Reg(uint8_t reg,uint8_t data);
uint8_t SCCB_RD_Reg(uint8_t reg);

#endif
