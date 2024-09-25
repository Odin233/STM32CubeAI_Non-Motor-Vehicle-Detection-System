#include "image.h"

#define min3v(v1, v2, v3)   ((v1)>(v2)? ((v2)>(v3)?(v3):(v2)):((v1)>(v3)?(v3):(v1)))//取最大
#define max3v(v1, v2, v3)   ((v1)<(v2)? ((v2)<(v3)?(v3):(v2)):((v1)<(v3)?(v3):(v1)))//取最小值

struct Image_t Image = {56,56};

uint16_t image_data[56*56] = {0};

void GetImage(uint16_t * image)
{
	uint16_t color;
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);//RRST
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);//RCK
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);//RCK
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);//RCK
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);//RRST
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);//RCK
	for(int i=Image.height-1;i>=0;i--)
	{
		for(uint8_t j=0;j<Image.width;j++)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);//RCK
			color=GPIOB->IDR&0XFF;	//读数据
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
			color<<=8;  
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
			color|=GPIOB->IDR&0XFF;	//读数据
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
			image[j+i*Image.width]=color;
		}
	}
}

uint32_t ReadColor(uint16_t x, uint16_t y)
{
	uint16_t color = image_data[x+y*Image.width];
	uint8_t r,g,b;
	r = (color&0xF800)>>9;
	g = (color&0x07E0)>>3;
	b = (color&0x001F)<<3;
	return (r<<16)|(g<<8)|b;
}
