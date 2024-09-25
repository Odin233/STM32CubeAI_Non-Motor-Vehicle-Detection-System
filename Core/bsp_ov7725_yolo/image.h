#ifndef IMAGE_H
#define IMAGE_H

#include "stm32f4xx.h"

#define IMAGE_WIDTH 56
#define IMAGE_HEIGHT 56

struct Image_t
{
	uint16_t width;
	uint16_t height;
};

extern struct Image_t Image;

extern uint16_t image_data[56*56];

void GetImage(uint16_t * image);

#endif
