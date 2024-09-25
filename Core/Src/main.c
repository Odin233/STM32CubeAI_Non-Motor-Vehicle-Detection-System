/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "dcmi.h"
#include "dma.h"
#include "gpio.h"
#include "app_x-cube-ai.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "math.h"
#include "usart.h"

#include "ov2640.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

extern DMA_HandleTypeDef hdma_dcmi;
extern DCMI_HandleTypeDef hdcmi;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t hsync_int = 0;						//帧中断标志
uint8_t ov2640_mode=1;						//工作模式:0,RGB565模式;1,JPEG模式
#define jpeg_buf_size 10*1024  			//定义JPEG数据缓存jpeg_buf的大小(*4字节)
__align(4) uint32_t jpeg_buf[jpeg_buf_size];	//JPEG数据缓存buf
volatile uint32_t jpeg_data_len=0; 			//buf中的JPEG有效数据长度
volatile uint8_t jpeg_data_ok=0;				//JPEG数据采集完成标志
										//0,数据没有采集完;
										//1,数据采集完了,但是还没处理;
										//2,数据已经处理完成了,可以开始下一帧接收

//JPEG尺寸支持列表
const uint16_t jpeg_img_size_tbl[][2]=
{
	160,120,	//QQVGA
	176,144,	//QCIF
	320,240,	//QVGA
	400,240,	//WQVGA
	352,288,	//CIF
	640,480,	//VGA
	800,600,	//SVGA
	1024,768,	//XGA
	1280,800,	//WXGA
	1280,960,	//XVGA
	1440,900,	//WXGA+
	1280,1024,	//SXGA
	1600,1200,	//UXGA	
};

const uint8_t*EFFECTS_TBL[7]={"Normal","Negative","B&W","Redish","Greenish","Bluish","Antique"};	//7种特效
const uint8_t*JPEG_SIZE_TBL[13]={"QQVGA","QCIF","QVGA","WQVGA","CIF","VGA","SVGA","XGA","WXGA","XVGA","WXGA+","SXGA","UXGA"};//JPEG图片 13种size

uint32_t i; 
uint8_t *p;
uint8_t key;
uint8_t effect=0,saturation=3,contrast=2,brightness=4;
uint8_t size=0;		//ov2460的默认是QVGA 320*240
uint8_t msgbuf[15];	//消息缓存区

// 定义X-CUBE_AI相关变量
static ai_handle network = AI_HANDLE_NULL; // ai_handle即为void*

AI_ALIGNED(32)
static ai_u8 activations[AI_MODEL_YOLO_1_DATA_ACTIVATIONS_SIZE];

AI_ALIGNED(32)
static ai_float in_data[AI_MODEL_YOLO_1_IN_1_SIZE]; // AI_模型名_IN_1_SIZE*4即为输入的真实字节数。一维张量仅有通道channel，二维张量加上高度height，三位张量加上宽度width。批次数batch在三种维度都可以自行添加。

AI_ALIGNED(32)
static ai_float out_data[AI_MODEL_YOLO_1_OUT_1_SIZE];

static ai_buffer* ai_input; // 输入地址
static ai_buffer* ai_output; // 输出地址

/*
// AI_模型名_IN_NUM为宏定义，表述输入数据的数量，而不是size。当AI_模型名_IN_NUM大于1时，为多输入情况。
ai_float* aiInData[AI_MODEL_TEST1_IN_1_NUM]; // 指针数组，用于多输入情况，注意必须说明长度，不然可能卡死（预留地址不足）
ai_float* aiInData[AI_MODEL_TEST1_OUT_1_NUM];
*/
// volatile uint32_t dataRdyIntReceived;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*
// 导入X-Cube-AI库自带printf重定向。
int fputc(int ch, FILE *p)
{
	while((USART1->SR&0X40)==0);
	USART1->DR = ch;
	
	return ch;
}
*/

//处理JPEG数据
//当采集完一帧JPEG数据后,调用此函数,切换JPEG BUF.开始下一帧采集.
void jpeg_data_process(void)
{
	if(ov2640_mode)// JPEG格式一般能直接输出到串口解码
	{
		if(jpeg_data_ok==0)	//jpeg数据还未采集完
		{
			__HAL_DMA_DISABLE(&hdma_dcmi);//关闭DMA
			while(DMA2_Stream1->CR&0X01);	//等待DMA2_Stream1可配置，也就是数据已收集完才进行下一步。
			// __HAL_DMA_GET_COUNTER()返回的是，在通过__HAL_DMA_SET_COUNTER()设定DMA琉传输的数据长度下，还没有发送/接收到的数据的长度。
			// 而已设定DMA流接收的数据长度为jpeg_buf_size，因此相减即为此次接收得到的jpeg图片的数据长度（这是因为每次jpeg图片长度不同，但是RGB565图片长度是固定的）。
			jpeg_data_len=jpeg_buf_size - __HAL_DMA_GET_COUNTER(&hdma_dcmi);
			jpeg_data_ok=1; 				//标记JPEG数据采集完按成,等待其他函数进一步处理
		}
		if(jpeg_data_ok==2)	//上一次的jpeg数据已经被其他函数处理了
		{
			__HAL_DMA_SET_COUNTER(&hdma_dcmi,jpeg_buf_size);// DMA传输长度为jpeg_buf_size*4字节
			__HAL_DMA_ENABLE(&hdma_dcmi); //打开DMA
			jpeg_data_ok=0;						//标记数据未采集完
		}
	}
    else
    {
        hsync_int = 1;
    }
}

// 模型初始化
int AI_Init_for_model_yolo_1(void){
	ai_error err;
	
	const ai_handle act_addr[]={activations};
	
	// 实例化神经网络
  err = ai_model_yolo_1_create_and_init(&network, act_addr, NULL); // 声明必须在初始化区，赋值必须在其他地方，这是C语言编译原理决定的。
  
	if (err.type != AI_ERROR_NONE)
  {
	  printf("E: AI error - type=%d code=%d\r\n", err.type, err.code); // 查看ai_platform.h来得知错误代码具体含义。
		Error_Handler();
  }
	
	ai_input=ai_model_yolo_1_inputs_get(network,NULL);
  ai_output=ai_model_yolo_1_outputs_get(network,NULL);
	printf("Model Initialize Complete !!! \r\n");
	
	return 0;
}

// 运行模型
int AI_Run_for_model_yolo_1(const void*in_data, void*out_data){
	ai_i32 n_batch;
	ai_error err;
	
	ai_input[0].data = AI_HANDLE_PTR(in_data);
  ai_output[0].data = AI_HANDLE_PTR(out_data);
  
	printf("---------Running Network-------- \r\n");
  n_batch = ai_model_yolo_1_run(network, &ai_input[0], &ai_output[0]);
  printf("---------Running End-------- \r\n");
  if (n_batch != 1){
		err = ai_model_yolo_1_get_error(network);
		printf("E: AI error - type=%d code=%d\r\n", err.type, err.code); // 输出错误信息。
		Error_Handler();
  }
	return 0;
}

/*
// 运行模型（多输入情况）
int AI_Run_for_model_test1(ai_float*pin[], ai_float*pout){

  for(int i=0; i<AI_MODEL_TEST1_IN_NUM; i++)
  {
		printf("Input Data Translation...\r\n");
	  ai_input[i].data = AI_HANDLE_PTR(pin); // AI_HANDLE_PTR是ai_handle类型宏定义，传入ai_float*指针，将数据转换成ai_handle类型。
  }
}
*/

// RGB565中各个基色的色阶不同，红色和蓝色色阶都是32（5bit），绿色为64（6bit），所以先要统一色阶，即红色和蓝色的色值乘以2，或者绿色色值除以2，然后才能进行灰度处理
float RGB565_to_Gray(int color){
	uint8_t R=(uint8_t)(color & 0xf800) >> 11; // 11111000 00000000
	uint8_t G=(uint8_t)(color & 0x07e0) >> 5; // 00000111 11100000
	uint8_t B=(uint8_t)(color & 0x001f); // 00000000 00011111
	// printf("color: %d \r\n",color);
	// printf("R: %d \r\n",R);
	// printf("G: %d \r\n",G);
	// printf("B: %d \r\n",B);
  // int Gray = (R*38 + G*75 + B*15) >> 7;
	float Gray = (R*2+G+B*2)/189.0;
	// printf("Gray: %f \r",Gray);
	// printf("Combine: %d \r\n",(0xFF<<8)|0xFF);
	return Gray;
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  __HAL_RCC_CRC_CLK_ENABLE(); // 当 X-CUBE-AI 组件添加到当前项目时，CRC IP 自动启用。
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CRC_Init();
  MX_DCMI_Init();
  MX_X_CUBE_AI_Init();
  /* USER CODE BEGIN 2 */
	
	printf("Initialize Begin ... \r\n");
	
	AI_Init_for_model_yolo_1();
  
	printf("OV2640 Initialize Begin... \r\n");
	while(OV2640_Init())//初始化OV2640
	{
		delay_ms(200);
		HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_14);
		printf("OV2640 Initialize Retry... \r\n");
	}
	printf("OV2640 Initialze Success !!! \r\n");
	
	// OV2640_JPEG_Mode();		//JPEG模式
  OV2640_RGB565_Mode();  // RGB565模式
	
	OV2640_Brightness(brightness);//亮度
	OV2640_Contrast(contrast);//对比度
	OV2640_Color_Saturation(saturation);//饱和度
	OV2640_Special_Effects(effect);//设置特效
	OV2640_OutSize_Set(jpeg_img_size_tbl[size][0],jpeg_img_size_tbl[size][1]);//设置输出尺寸
	
	HAL_DMA_Start(&hdma_dcmi,(uint32_t)&DCMI->DR,(uint32_t)&jpeg_buf,jpeg_buf_size); // 主要的占用空间的语句1&2，有则占用大。因为调用了jpeg_buf。
 	OV2640_OutSize_Set(jpeg_img_size_tbl[size][0],jpeg_img_size_tbl[size][1]);//设置输出尺寸
  DCMI_Start(); // 这个函数也是必要的，虽然找不到定义。
	
	// dataRdyIntReceived=0; // 每个循环刷新
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
		printf("While Begin... \r\n"); // 测试代码
		// HAL_Delay(200); // 测试代码	
		
		if(jpeg_data_ok==1)	//已经采集完一帧图像了
		{
      // printf("jpeg data ok! \r\n"); // 采集完成，测试代码			
			p=(uint8_t*)jpeg_buf; // 主要的占用空间的语句1&2，有则占用大。因为调用了jpeg_buf。
      // jpeg模式长度不定。RGB565模式，图像数据长度+8=输出分辨率总像素数*2，单位为字节，符合每个像素2字节。
			// 因此每个像素占有16个bit(2字节)，也就是2个两位十六进制数（一个一位的十六进制数表示0~15占0.5个字节，4个bit。一个二位的十六进制数表示0~255占1个字节，8个bit）。
			 
			printf("Data Length: %d \r\n",jpeg_data_len);
			/*
			for(i=0;i<jpeg_data_len*4;i++)		//dma传输1次等于4字节,所以乘以4。
			{
				while((USART1->SR&0X40)==0);	//循环发送,直到发送完毕。每次发送1字节，也就是1个像素。
				USART1->DR=p[i];
			}
			*/

			// HAL_Delay(100); // 测试代码
			jpeg_data_ok=2;	//标记jpeg数据处理完了,可以让DMA去采集下一帧了.
		}
		
		printf("Create The Input...\r\n");
		
		/*
	  for(uint32_t i=0;i<AI_MODEL_YOLO_1_IN_1_SIZE;i++){
			in_data[3*(i+1)-1]=(ai_float)1.6; // x
			in_data[3*(i+1)-2]=(ai_float)1.6; // y
			in_data[3*(i+1)-3]=(ai_float)1.6; // z
		} // x,y,z 均为 1.6 时可以使得6种状态均有可能，用于测试。
		*/
		
		// printf("%#X \r\n",p[0]); // 输出地址的格式
		// printf("%d \r\n",p[0]); // 输出整数的格式
		for(i=0;i<AI_MODEL_YOLO_1_IN_1_SIZE;i++)
		{
			in_data[i]=RGB565_to_Gray((p[i*2]<<8)|p[i*2+1]);
		}		
		
		/*
		for(i=0;i<AI_MODEL_YOLO_1_IN_1_SIZE;i++)
		{
				in_data[2*(i+1)-1]=0.256;
			  in_data[2*(i+1)-2]=0.347;
		}
		*/
		
		printf("Run The Model...\r\n");
    AI_Run_for_model_yolo_1(in_data,out_data);
		
		printf("Output... \r\n");
	  for(uint32_t i=0; i<AI_MODEL_YOLO_1_OUT_1_SIZE;i++){
			printf("OutPut Probability %d:%f \r\n",i,out_data[i]);
		}
		
    /* USER CODE END WHILE */

  // MX_X_CUBE_AI_Process();
    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
		HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_13);
		HAL_Delay(500);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
