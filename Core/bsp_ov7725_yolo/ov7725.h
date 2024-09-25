#ifndef OV7725_H
#define OV7725_H

#include "stm32f4xx.h"

#define OV7725_MID				0X7FA2    
#define OV7725_PID				0X7721

uint8_t OV7725_Init(void);
void OV7725_Window_Set(uint16_t width,uint16_t height,uint8_t mode);
void OV7725_Light_Mode(uint8_t mode);
void OV7725_Color_Saturation(int8_t sat);
void OV7725_Brightness(int8_t bright);
void OV7725_Contrast(int8_t contrast);
void OV7725_Special_Effects(uint8_t eft);

#endif
