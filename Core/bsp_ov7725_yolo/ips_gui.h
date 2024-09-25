#ifndef IPS_GUI_H
#define IPS_GUI_H

#include "stm32f4xx.h"

void GUI_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void GUI_HLine(uint8_t x0, uint8_t y0, uint8_t x1, uint16_t color);
void GUI_RLine(uint8_t x0, uint8_t y0, uint8_t y1, uint16_t color);
void GUI_Line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void GUI_Rectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);

#endif
