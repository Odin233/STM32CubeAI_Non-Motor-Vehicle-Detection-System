#ifndef IPS_H
#define IPS_H

#include "stm32f4xx.h"

#define RES_GPIO GPIOA
#define RES_PIN GPIO_PIN_2
#define DC_GPIO GPIOA
#define DC_PIN GPIO_PIN_1

#define LCD_WIDTH   160 //LCD width
#define LCD_HEIGHT  80 //LCD height

/**
 * image color
**/

#define WHITE					0xFFFF
#define BLACK					0x0000	  
#define BLUE 					0x001F  
#define BRED 					0XF81F
#define GRED 					0XFFE0
#define GBLUE					0X07FF
#define RED  					0xF800
#define MAGENTA				0xF81F
#define GREEN					0x07E0
#define CYAN 					0x7FFF
#define YELLOW				0xFFE0
#define BROWN					0XBC40 
#define BRRED					0XFC07 
#define GRAY 					0X8430 
#define DARKBLUE			0X01CF	
#define LIGHTBLUE			0X7D7C	 
#define GRAYBLUE      0X5458 
#define LIGHTGREEN    0X841F 
#define LGRAY 			  0XC618 
#define LGRAYBLUE     0XA651
#define LBBLUE        0X2B12

void LCD_Init(void);
void LCD_SetCursor(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t  Yend);
void LCD_Clear(uint16_t Color);
void LCD_ShowString(uint16_t x, uint16_t y, uint8_t * ch, uint16_t color);
 void LCD_WriteData_Word(uint16_t da);

#endif
