#include "example.h"
#include "ov2640.h"
#include "dcmi.h"


uint8_t ov2640_mode = 1;        //工作模式:0,RGB565模式;1,JPEG模式
#define jpeg_buf_size 30*1024            //定义JPEG数据缓存jpeg_buf的大小(*4字节)
__ALIGNED(4) uint32_t jpeg_buf[jpeg_buf_size];    //JPEG数据缓存buf
volatile uint32_t jpeg_data_len = 0;            //buf中的JPEG有效数据长度
volatile uint8_t jpeg_data_ok = 0;                //JPEG数据采集完成标志
const char *EFFECTS_TBL[7] = {"Normal", "Negative", "B&W", "Redish", "Greenish", "Bluish", "Antique"};     /* 7种特效 */
const char *JPEG_SIZE_TBL[13] = {"QQVGA", "QCIF", "QVGA", "WGVGA", "CIF", "VGA", "SVGA", "XGA", "WXGA", "SVGA", "WXGA+",
                                 "SXGA", "UXGA"}; /* JPEG图片 13种尺寸 */

const uint16_t jpeg_img_size_tbl[][2] =
        {
                160, 120,       /* QQVGA */
                176, 144,       /* QCIF */
                320, 240,       /* QVGA */
                400, 240,        /* WGVGA */
                352, 288,        /* CIF */
                640, 480,       /* VGA */
                800, 600,       /* SVGA */
                1024, 768,      /* XGA */
                1280, 800,      /* WXGA */
                1280, 960,      /* XVGA */
                1440, 900,      /* WXGA+ */
                1280, 1024,     /* SXGA */
                1600, 1200,     /* UXGA */
        };

void jpeg_data_process(void) {
    if (ov2640_mode)//只有在JPEG格式下,才需要做处理.
    {
        if (jpeg_data_ok == 0)    //jpeg数据还未采集完?
        {
            __HAL_DMA_DISABLE(&hdma_dcmi);//关闭DMA
            while (DMA2_Stream1->CR & 0X01);    //等待DMA2_Stream1可配置
            jpeg_data_len = jpeg_buf_size - __HAL_DMA_GET_COUNTER(&hdma_dcmi);//得到剩余数据长度
            jpeg_data_ok = 1;                //标记JPEG数据采集完按成,等待其他函数处理
        }
        if (jpeg_data_ok == 2)    //上一次的jpeg数据已经被处理了
        {
            __HAL_DMA_SET_COUNTER(&hdma_dcmi, jpeg_buf_size);//传输长度为jpeg_buf_size*4字节
            __HAL_DMA_ENABLE(&hdma_dcmi); //打开DMA
            jpeg_data_ok = 0;                        //标记数据未采集
        }
    }

}

extern void dcmi_start(void);


void jpeg_test(void) {
    uint32_t i;
    uint8_t *p;
    uint8_t effect = 0, saturation = 2, contrast = 2;
    uint8_t size = 2;        //默认是QVGA 320*240尺寸
    uint8_t msgbuf[15];    //消息缓存区
//    delay_init(168);
    while (ov2640_init());       /* 初始化OV2640 */

    ov2640_jpeg_mode(); /* JPEG模式 */
    ov2640_contrast(contrast);//对比度
    ov2640_color_saturation(saturation);//饱和度
    ov2640_special_effects(effect);//设置特效
    ov2640_outsize_set(jpeg_img_size_tbl[size][0], jpeg_img_size_tbl[size][1]);//设置输出尺寸
    HAL_DMA_Start(&hdma_dcmi, (uint32_t) &DCMI->DR, (uint32_t) &jpeg_buf, jpeg_buf_size);
    dcmi_start();


    while (1) {
        if (jpeg_data_ok == 1)    //已经采集完一帧图像了
        {
            p = (uint8_t *) jpeg_buf;

            for (i = 0; i < jpeg_data_len * 4; i++)        //dma传输1次等于4字节,所以乘以4.
            {
                while ((USART3->SR & 0X40) == 0);    //循环发送,直到发送完毕
                USART3->DR = p[i];
            }
            jpeg_data_ok = 2;    //标记jpeg数据处理完了,可以让DMA去采集下一帧了.
        }

    }
}

