#include "ips_gui.h"
#include "image.h"

extern uint16_t image_data[56*56];

void GUI_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	image_data[y+(55-x)*Image.width] = color;
}

void GUI_HLine(uint8_t x0, uint8_t y0, uint8_t x1, uint16_t color)
{
	uint8_t  temp;
	if(x0>x1)
	{
		temp = x1;
		x1 = x0;
		x0 = temp;
	}
	do
	{
		GUI_DrawPoint(x0, y0, color); 
		x0++;
	}
	while(x1>=x0);
}

void GUI_RLine(uint8_t x0, uint8_t y0, uint8_t y1, uint16_t color)
{
	uint8_t  temp;
	if(y0>y1)
	{
		temp = y1;
		y1 = y0;
		y0 = temp;
	}
	do
	{
		GUI_DrawPoint(x0, y0, color);
		y0++;
	}
	while(y1>=y0);
}

void GUI_Line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
{
	signed   short   dx;
	signed   short   dy;
	signed   char    dx_sym;
	signed   char    dy_sym;
	signed   short   dx_x2;
	signed   short   dy_x2;
	signed   char   di;

	dx = x1-x0;
	dy = y1-y0;

	if(dx>0)
	{
	  dx_sym = 1;
	}
	else
	{
	  if(dx<0)
		{
		  dx_sym = -1;
		}
		else
		{
			GUI_RLine(x0, y0, y1, color);
			return;
		}
	}

	if(dy>0)
	{
	  dy_sym = 1;
	}
	else
	{
	  if(dy<0)
		{
		 dy_sym = -1;
		}
		else
		{
			GUI_HLine(x0, y0, x1, color);
			return;
		}
	}


	dx = dx_sym * dx;
	dy = dy_sym * dy;

	dx_x2 = dx*2;
	dy_x2 = dy*2;

	if(dx>=dy)
	{
		di = dy_x2 - dx;
		while(x0!=x1)
		{
			GUI_DrawPoint(x0, y0, color);
			x0 += dx_sym;
			if(di<0)
			{
				di += dy_x2;
			}
			else
			{
				di += dy_x2 - dx_x2;
				y0 += dy_sym;
			}
		}
		GUI_DrawPoint(x0, y0, color);
	}
	else
	{
		di = dx_x2 - dy;
		while(y0!=y1)
		{
			GUI_DrawPoint(x0, y0, color);
			y0 += dy_sym;
			if(di<0)
			{
				di += dx_x2;
			}
			else
			{
				di += dx_x2 - dy_x2;
				x0 += dx_sym;
			}
		}
		GUI_DrawPoint(x0, y0, color);
	}
}

void GUI_Rectangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
{
	GUI_HLine(x0, y0, x1, color);
	GUI_HLine(x0, y1, x1, color);
	GUI_RLine(x0, y0, y1, color);
	GUI_RLine(x1, y0, y1, color);
}
