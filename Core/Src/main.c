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

uint8_t hsync_int = 0;						//֡�жϱ�־
uint8_t ov2640_mode=1;						//����ģʽ:0,RGB565ģʽ;1,JPEGģʽ
#define jpeg_buf_size 10*1024  			//����JPEG���ݻ���jpeg_buf�Ĵ�С(*4�ֽ�)
__align(4) uint32_t jpeg_buf[jpeg_buf_size];	//JPEG���ݻ���buf
volatile uint32_t jpeg_data_len=0; 			//buf�е�JPEG��Ч���ݳ���
volatile uint8_t jpeg_data_ok=0;				//JPEG���ݲɼ���ɱ�־
										//0,����û�вɼ���;
										//1,���ݲɼ�����,���ǻ�û����;
										//2,�����Ѿ����������,���Կ�ʼ��һ֡����

//JPEG�ߴ�֧���б�
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

const uint8_t*EFFECTS_TBL[7]={"Normal","Negative","B&W","Redish","Greenish","Bluish","Antique"};	//7����Ч
const uint8_t*JPEG_SIZE_TBL[13]={"QQVGA","QCIF","QVGA","WQVGA","CIF","VGA","SVGA","XGA","WXGA","XVGA","WXGA+","SXGA","UXGA"};//JPEGͼƬ 13��size

uint32_t i; 
uint8_t *p;
uint8_t key;
uint8_t effect=0,saturation=3,contrast=2,brightness=4;
uint8_t size=0;		//ov2460��Ĭ����QVGA 320*240
uint8_t msgbuf[15];	//��Ϣ������

// ����X-CUBE_AI��ر���
static ai_handle network = AI_HANDLE_NULL; // ai_handle��Ϊvoid*

AI_ALIGNED(32)
static ai_u8 activations[AI_MODEL_YOLO_1_DATA_ACTIVATIONS_SIZE];

AI_ALIGNED(32)
static ai_float in_data[AI_MODEL_YOLO_1_IN_1_SIZE]; // AI_ģ����_IN_1_SIZE*4��Ϊ�������ʵ�ֽ�����һά��������ͨ��channel����ά�������ϸ߶�height����λ�������Ͽ��width��������batch������ά�ȶ�����������ӡ�

AI_ALIGNED(32)
static ai_float out_data[AI_MODEL_YOLO_1_OUT_1_SIZE];

static ai_buffer* ai_input; // �����ַ
static ai_buffer* ai_output; // �����ַ

/*
// AI_ģ����_IN_NUMΪ�궨�壬�����������ݵ�������������size����AI_ģ����_IN_NUM����1ʱ��Ϊ�����������
ai_float* aiInData[AI_MODEL_TEST1_IN_1_NUM]; // ָ�����飬���ڶ����������ע�����˵�����ȣ���Ȼ���ܿ�����Ԥ����ַ���㣩
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
// ����X-Cube-AI���Դ�printf�ض���
int fputc(int ch, FILE *p)
{
	while((USART1->SR&0X40)==0);
	USART1->DR = ch;
	
	return ch;
}
*/

//����JPEG����
//���ɼ���һ֡JPEG���ݺ�,���ô˺���,�л�JPEG BUF.��ʼ��һ֡�ɼ�.
void jpeg_data_process(void)
{
	if(ov2640_mode)// JPEG��ʽһ����ֱ����������ڽ���
	{
		if(jpeg_data_ok==0)	//jpeg���ݻ�δ�ɼ���
		{
			__HAL_DMA_DISABLE(&hdma_dcmi);//�ر�DMA
			while(DMA2_Stream1->CR&0X01);	//�ȴ�DMA2_Stream1�����ã�Ҳ�����������ռ���Ž�����һ����
			// __HAL_DMA_GET_COUNTER()���ص��ǣ���ͨ��__HAL_DMA_SET_COUNTER()�趨DMA��������ݳ����£���û�з���/���յ������ݵĳ��ȡ�
			// �����趨DMA�����յ����ݳ���Ϊjpeg_buf_size����������Ϊ�˴ν��յõ���jpegͼƬ�����ݳ��ȣ�������Ϊÿ��jpegͼƬ���Ȳ�ͬ������RGB565ͼƬ�����ǹ̶��ģ���
			jpeg_data_len=jpeg_buf_size - __HAL_DMA_GET_COUNTER(&hdma_dcmi);
			jpeg_data_ok=1; 				//���JPEG���ݲɼ��갴��,�ȴ�����������һ������
		}
		if(jpeg_data_ok==2)	//��һ�ε�jpeg�����Ѿ�����������������
		{
			__HAL_DMA_SET_COUNTER(&hdma_dcmi,jpeg_buf_size);// DMA���䳤��Ϊjpeg_buf_size*4�ֽ�
			__HAL_DMA_ENABLE(&hdma_dcmi); //��DMA
			jpeg_data_ok=0;						//�������δ�ɼ���
		}
	}
    else
    {
        hsync_int = 1;
    }
}

// ģ�ͳ�ʼ��
int AI_Init_for_model_yolo_1(void){
	ai_error err;
	
	const ai_handle act_addr[]={activations};
	
	// ʵ����������
  err = ai_model_yolo_1_create_and_init(&network, act_addr, NULL); // ���������ڳ�ʼ��������ֵ�����������ط�������C���Ա���ԭ������ġ�
  
	if (err.type != AI_ERROR_NONE)
  {
	  printf("E: AI error - type=%d code=%d\r\n", err.type, err.code); // �鿴ai_platform.h����֪���������庬�塣
		Error_Handler();
  }
	
	ai_input=ai_model_yolo_1_inputs_get(network,NULL);
  ai_output=ai_model_yolo_1_outputs_get(network,NULL);
	printf("Model Initialize Complete !!! \r\n");
	
	return 0;
}

// ����ģ��
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
		printf("E: AI error - type=%d code=%d\r\n", err.type, err.code); // ���������Ϣ��
		Error_Handler();
  }
	return 0;
}

/*
// ����ģ�ͣ������������
int AI_Run_for_model_test1(ai_float*pin[], ai_float*pout){

  for(int i=0; i<AI_MODEL_TEST1_IN_NUM; i++)
  {
		printf("Input Data Translation...\r\n");
	  ai_input[i].data = AI_HANDLE_PTR(pin); // AI_HANDLE_PTR��ai_handle���ͺ궨�壬����ai_float*ָ�룬������ת����ai_handle���͡�
  }
}
*/

// RGB565�и�����ɫ��ɫ�ײ�ͬ����ɫ����ɫɫ�׶���32��5bit������ɫΪ64��6bit����������Ҫͳһɫ�ף�����ɫ����ɫ��ɫֵ����2��������ɫɫֵ����2��Ȼ����ܽ��лҶȴ���
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
  __HAL_RCC_CRC_CLK_ENABLE(); // �� X-CUBE-AI �����ӵ���ǰ��Ŀʱ��CRC IP �Զ����á�
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
	while(OV2640_Init())//��ʼ��OV2640
	{
		delay_ms(200);
		HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_14);
		printf("OV2640 Initialize Retry... \r\n");
	}
	printf("OV2640 Initialze Success !!! \r\n");
	
	// OV2640_JPEG_Mode();		//JPEGģʽ
  OV2640_RGB565_Mode();  // RGB565ģʽ
	
	OV2640_Brightness(brightness);//����
	OV2640_Contrast(contrast);//�Աȶ�
	OV2640_Color_Saturation(saturation);//���Ͷ�
	OV2640_Special_Effects(effect);//������Ч
	OV2640_OutSize_Set(jpeg_img_size_tbl[size][0],jpeg_img_size_tbl[size][1]);//��������ߴ�
	
	HAL_DMA_Start(&hdma_dcmi,(uint32_t)&DCMI->DR,(uint32_t)&jpeg_buf,jpeg_buf_size); // ��Ҫ��ռ�ÿռ�����1&2������ռ�ô���Ϊ������jpeg_buf��
 	OV2640_OutSize_Set(jpeg_img_size_tbl[size][0],jpeg_img_size_tbl[size][1]);//��������ߴ�
  DCMI_Start(); // �������Ҳ�Ǳ�Ҫ�ģ���Ȼ�Ҳ������塣
	
	// dataRdyIntReceived=0; // ÿ��ѭ��ˢ��
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
		printf("While Begin... \r\n"); // ���Դ���
		// HAL_Delay(200); // ���Դ���	
		
		if(jpeg_data_ok==1)	//�Ѿ��ɼ���һ֡ͼ����
		{
      // printf("jpeg data ok! \r\n"); // �ɼ���ɣ����Դ���			
			p=(uint8_t*)jpeg_buf; // ��Ҫ��ռ�ÿռ�����1&2������ռ�ô���Ϊ������jpeg_buf��
      // jpegģʽ���Ȳ�����RGB565ģʽ��ͼ�����ݳ���+8=����ֱ�����������*2����λΪ�ֽڣ�����ÿ������2�ֽڡ�
			// ���ÿ������ռ��16��bit(2�ֽ�)��Ҳ����2����λʮ����������һ��һλ��ʮ����������ʾ0~15ռ0.5���ֽڣ�4��bit��һ����λ��ʮ����������ʾ0~255ռ1���ֽڣ�8��bit����
			 
			printf("Data Length: %d \r\n",jpeg_data_len);
			/*
			for(i=0;i<jpeg_data_len*4;i++)		//dma����1�ε���4�ֽ�,���Գ���4��
			{
				while((USART1->SR&0X40)==0);	//ѭ������,ֱ��������ϡ�ÿ�η���1�ֽڣ�Ҳ����1�����ء�
				USART1->DR=p[i];
			}
			*/

			// HAL_Delay(100); // ���Դ���
			jpeg_data_ok=2;	//���jpeg���ݴ�������,������DMAȥ�ɼ���һ֡��.
		}
		
		printf("Create The Input...\r\n");
		
		/*
	  for(uint32_t i=0;i<AI_MODEL_YOLO_1_IN_1_SIZE;i++){
			in_data[3*(i+1)-1]=(ai_float)1.6; // x
			in_data[3*(i+1)-2]=(ai_float)1.6; // y
			in_data[3*(i+1)-3]=(ai_float)1.6; // z
		} // x,y,z ��Ϊ 1.6 ʱ����ʹ��6��״̬���п��ܣ����ڲ��ԡ�
		*/
		
		// printf("%#X \r\n",p[0]); // �����ַ�ĸ�ʽ
		// printf("%d \r\n",p[0]); // ��������ĸ�ʽ
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
