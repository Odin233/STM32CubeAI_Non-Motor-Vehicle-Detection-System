## 目录

    - OV7725摄像头的使用（带FIFO）（转载未确认的参考资料）
      - 时钟配置
      - DCMI
      - DMA
      - 相关HAL库函数
      - OV7725引脚定义
      - LTDC（STM32F429官方板子自带显示屏）（STM32F429ZI6的LTDC与DCMI冲突）
      - 摄像头采集数据过程
  - 基于STMF429ZIT6（STMF429I-Disco）的非机动车目标检测
    - 通过X-Cube-AI部署HAR/HPE模型例程（STM32f429ZIT6）
    - STM32CubeMX、STM32CubeIDE进行X-Cube-AI相关环境搭建问题
    - X-Cube-AI用到的Python库安装
    - HAL库的I2C相关函数
    - OV7725摄像头的使用（带FIFO）（未检验）
      - 通讯方法
      - 引脚设定与接线
    - OV2640摄像头使用（不带FIFO）（已验证）
      - 硬件接线与CubeMX配置
    - TFT触摸液晶屏（屏幕和触摸功能均通过SPI驱动）（带SD卡槽，2.4寸，驱动IC为ILI9341）
      - 注意事项
    - 逻辑分析仪的使用
    - SDRAM的使用（ST官方STM32F429I-DISCO开发板自带IS42S16400J芯片）（用于显示图像，也就是将SDRAM作为显存使用）
      - FMC/FSMC-SDRAM关键参数
      - STM32CubeMX配置方法
      - STM32CubeMX工程修改
    - 注意事项
  - TinyML-模型压缩方法
    - 剪枝
    - 量化
    - 知识蒸馏
    - 聚类

### OV7725摄像头的使用（带FIFO）（转载未确认的参考资料）

#### 时钟配置

![f04624f0c07bbd48111dc3aafb7e7fc7.png](/_resources/f04624f0c07bbd48111dc3aafb7e7fc7.png)

#### DCMI

- STM32F429具备DCMI摄像头接口，可利用DMA功能实现快速的硬件级图像采集存储，大大降低了纯软件开发的难度。

设置DCMI：

![65a3fd44fc33bb80424dc74005bc2b5f.png](/_resources/65a3fd44fc33bb80424dc74005bc2b5f.png)

- 不同的mode代表不同的数据格式。8bits Embedded Synchro是指码流中嵌入同步码元的编码形式，其他的External Synchro都是通过单独的硬线提供同步信号。8、10、12、14是指数据位宽，像OV7725是8位宽数据+硬线同步，所以应该选择Slave 8 bits External Synchro。

GPIO Settings：

![08df91a994e81c404176ba9f21842e78.png](/_resources/08df91a994e81c404176ba9f21842e78.png)

![ec44c85507deffdb148ac9b5b5bf6208.png](/_resources/ec44c85507deffdb148ac9b5b5bf6208.png)

Parameter Setting：

![c235cbb0f5b9476888993676f7ddd98d.png](/_resources/c235cbb0f5b9476888993676f7ddd98d.png)

- 前3个参数是和所接摄像头参数对应的，要保持一致，分别是指PCLK触发沿选择、垂直同步信号VSYNC极性、水平同步信号HREF极性，注意OV7725提供的水平同步信号是HREF而不是HSYNC。
从下图OV7725时序图可以看出，PCLK上升沿有效，所以选Active on Rising edge。

![2e18869608f5fa5d11be28b57b296e49.png](/_resources/2e18869608f5fa5d11be28b57b296e49.png)

- 但是，从下图帧时序图中看到，当VSYNC低电平，且HREF、HSYNC高电平时，才传输有效数据，DCMI配置选项中的Active High是指高电平时进行同步，官方称为消隐信号，也就是低电平时进行传数。这个一定要选对，不然接收到的数据永远都是0x00。对应OV7725的正确选择应该是：V=Active high，H=Active low。

![5097c2cd5b59d654aaefc11ad3aefcc8.png](/_resources/5097c2cd5b59d654aaefc11ad3aefcc8.png)

- 接下来是“Frequency of frame capture”，这个是指DCMI接收处理的帧频率，通俗点就是说摄像头在一直发送图像数据，可能帧频率是60fps，但我们实际不需要这么高的帧率，可以通过这个选项选择“每处理1帧，丢弃3帧”、“每处理1帧，丢弃1帧”、“全部处理”。根据实际需要选择。
- 最后一个JPEG mode是指接收的数据流传输的是否为JPEG压缩数据。OV7725输出的VGA、QVGA、CIF格式都是非压缩数据，所以这里要选Disabled。

#### DMA

- DMA mode有normal和circular两个选项，normal指执行一次DMA传输后停止，circular指连续循环执行数据搬移。
Data Width的选择应与实际一致，我们要将OV7725通过DCMI传入的外设数据搬移到内部存储器中，OV7725在传给DCMI时一次传8bits，但DCMI内部会将接收到的摄像头数据放到一个 32 位数据寄存器(DCMI_DR)中，然后通过通用DMA 进行传输。也就是说DCMI接收处理4次放满32bits数据后才会发起一次DMA传输。如果想使用DMA的FIFO，可以选择Use Fifo，并选定Threshold参数。

- 此处我们的图像编码格式是RGB565，按照上述说明，DCMI输出的32位数据应该是下图这种排列方式，当LTDC读取时也是这样的格式，通过实现发现是可以直接解码的，不需要进行位变换，但这部分内部原理还需要摸清楚，这里暂时不展开。

![ac51533877cef4f9fd89ba05eec23532.png](/_resources/ac51533877cef4f9fd89ba05eec23532.png)

![5aafd5f75f643dfe89c2af18f9cb9f0b.png](/_resources/5aafd5f75f643dfe89c2af18f9cb9f0b.png)

#### 相关HAL库函数

- 启动DCMI：HAL_StatusTypeDef HAL_DCMI_Start_DMA (DCMI_HandleTypeDef * hdcmi, uint32_t DCMI_Mode, uint32_t pData, uint32_t Length)

- 结束DCMI：HAL_StatusTypeDef HAL_DCMI_Stop(DCMI_HandleTypeDef * hdcmi)

- 行接收完毕中断：void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef * hdcmi)

- 帧接收完毕中断：void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef * hdcmi) 或
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)

#### OV7725引脚定义

![0758c2b96f808f6d345b36ad9a73f321.png](/_resources/0758c2b96f808f6d345b36ad9a73f321.png)

- 但是他的参数设置可是多的一批，一共有0x00-0xac个8位寄存器，要想获得优秀画质，就得摸透这些寄存器。下面是官方给出的有用的寄存器配置方案，具体应用时可适当调整：
【注意】：硬件I2C有一些bug，不太好用，总是busy状态，推荐使用软件I2C进行配置通信。

```
	//输出窗口设置
	{COM7,      0x46}, //QVGA RGB565
	{HSTART,    0x3f}, //水平起始位置
	{HSIZE,     0x50}, //水平尺寸
	{VSTRT,     0x03}, //垂直起始位置
	{VSIZE,     0x78}, //垂直尺寸
	{HREF,      0x00},
	{HOutSize,  0x50}, //输出尺寸高,QVGA320填0x50; VGA填0xA0;
	{VOutSize,  0x78}, //输出尺寸宽,QVGA480填0x78; VGA填0xF0;

	//帧率
	//30 fps, PCLK = 12Mhz
	{CLKRC, 	0x01}, //CLKRC, F/2/2;F(internal clock) = F(input clock)/(Bit[5:0]+1)/2
	{COM4, 		0x41}, //COM4, PLL 4倍频
	{EXHCH, 	0x00},
	{EXHCL, 	0x00},
	{DM_LNL, 	0x00},  //DM_LNL, Dummy Row Low 8 Bits
	{DM_LNH, 	0x00}, //DM_LNH, Dummy Row High 8 Bits
	{ADVFL, 	0x00},
	{ADVFH, 	0x00},
	{COM5, 		0xf5},//夜晚模式下自动帧率控制开启

	//DSP control
	{TGT_B,     0x80},//{TGT_B,     0x7f},
	{FixGain,   0x00},//0x09
	{AWB_Ctrl0, 0xf0},//0xe0
	{DSP_Ctrl1, 0x1f},//0xff
	{DSP_Ctrl2, 0x00},
	{DSP_Ctrl3, 0x10},
	{DSP_Ctrl4, 0x00},

	//AGC AEC AWB
	{COM8,		0x8f},//0xf0
	{COM4,		0x41}, //Pll AEC CONFIG
	{COM6,		0x43},//0xc5
	{COM9,		0x4a},//0x11
	{BDBase,	0xfF},//0x7f
	{BDMStep,	0x01},//0x03
	{AEW,		0x40},
	{AEB,		0x30},
	{VPT,		0xa1},
	{EXHCL,		0x9e},
	{AWBCtrl3,  0xaa},
	{COM8,		0xff},

	//matrix shapness brightness contrast
	{EDGE1,		0x08},
	{DNSOff,	0x01},
	{EDGE2,		0x03},
	{EDGE3,		0x00},
	{MTX1,		0xb0},
	{MTX2,		0x9d},
	{MTX3,		0x13},
	{MTX4,		0x16},
	{MTX5,		0x7b},
	{MTX6,		0x91},
	{MTX_Ctrl,  0x1e},
	{BRIGHT,	0x08},
	{CNST,		0x20},
	{UVADJ0,	0x81},
	{SDE, 		0X06},
	{USAT,		0x65},
	{VSAT,		0x65},
	{HUECOS,	0X80},
	{HUESIN, 	0X80},

    //GAMMA config
	{GAM1,		0x0c},
	{GAM2,		0x16},
	{GAM3,		0x2a},
	{GAM4,		0x4e},
	{GAM5,		0x61},
	{GAM6,		0x6f},
	{GAM7,		0x7b},
	{GAM8,		0x86},
	{GAM9,		0x8e},
	{GAM10,		0x97},
	{GAM11,		0xa4},
	{GAM12,		0xaf},
	{GAM13,		0xc5},
	{GAM14,		0xd7},
	{GAM15,		0xe8},
	{SLOP,		0x20},

	{COM3,		0x40},//Horizontal mirror image；默认0x10,即改变YUV为UVY格式。但是摄像头不是芯片而是模组时，要将0X10中的1变成0，即设置YUV格式

	{COM10,		0x00}, //默认VSYNC 低电平有效。如果要兼容OV2640 DCMI的配置这里需要VSYNC 高电平有效
	{COM2,		0x01}, //设置输出驱动能力为2倍
```

#### LTDC（STM32F429官方板子自带显示屏）（STM32F429ZI6的LTDC与DCMI冲突）

- LTDC是一种TFT显示屏接口，全称为LCD-TFT display controller，属于显示像素接口的一种，显示控制器提供了一个并行的数字RGB（红、绿、蓝）信号、以及水平/垂直同步信号、像素时钟作为输出，直接与各种LCD和TFT面板连接，且显示面板不需要缓存。一帧开始后，从左向右、从上向下一个像素一个像素输出RGB值，类似VGA逐行扫描刷新。
STM32的LTDC使用非常简单，完成参数配置后，只要定时向图层句柄设置图像数据地址即可，代码量非常少，基本可以看成STM32的显卡。

- LTDC主要的接口IO有像素时钟LCD_CLK、水平同步HSYNC、垂直同步VSYNC、数据有效DE和3组RGB数据信号并行线，STM32F429最大支持RGB888显示输出，Display Type选项要根据所使用的显示屏支持的数据格式进行选择。

![56b42b5f8c35b97223d1c58de9ddf762.png](/_resources/56b42b5f8c35b97223d1c58de9ddf762.png)

- configuration中的parameter settings是最核心的配置，其中下图红框中的参数是由所使用的显示屏决定的，通常显示屏datasheet中都会给出，比如我使用的TM043NDH02给出的配置参数见第二张图，只要一一对应匹配即可。

![0ed8c648e65bb52d9d73843ca0579bdb.png](/_resources/0ed8c648e65bb52d9d73843ca0579bdb.png)

![bd092ae91a049cf1b460dc1799d039ed.png](/_resources/bd092ae91a049cf1b460dc1799d039ed.png)

- signal polarity有效电平配置。这部分有效电平的设置一定要和使用的显示屏相符合，下图为我是用的显示屏datasheet对IO电平的要求。

![db3d98169e77a25c75173c933b75f20d.png](/_resources/db3d98169e77a25c75173c933b75f20d.png)

- layer settings主要是对图层参数进行设置。STM32F429共提供两个图层，每个图层的配置基本相似，下面是单个图层的配置说明。

![47d8bb81d1df859e76df25cf128ca01e.png](/_resources/47d8bb81d1df859e76df25cf128ca01e.png)

- 完成以上配置后，LTDC就可以工作了。当我们需要把OV7725采集的图像进行显示时，只要在DCMI的帧中断或垂直同步中断中，把帧图像缓存地址向LTDC的图层句柄的起始地址赋值，然后调用一次配置函数即可，这样每接收完一帧图像，即触发一次显示刷新。

```
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	pLayerCfg.FBStartAdress =  IMG_ADDR;
	if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
	{
		Error_Handler();
	}
}
```

#### 摄像头采集数据过程

(1) 利用 SIO_C、SIO_D 引脚通过 SCCB 协议向 OV7725 的寄存器写入初始化配置；
(2) 初始化完成后，OV7725 传感器会使用 VGA 时序输出图像数据，它的 VSYNC 会首先输出帧有效信号（低电平跳变），当外部的控制器（如 STM32）检测到该信号时，把 WEN 引脚设置为高电平，并且使用 WRST 引脚复位 FIFO 的写指针到 0 地址；
(3) 随着 OV7725 继续按 VGA 时序输出图像数据，它在传输每行有效数据时， HREF引脚都会持续输出高电平，由于 WEN 和 HREF 同时为高电平输入至与非门，使得其连接到 FIFO WE 引脚的输出为低电平，允许向 FIFO 写入数据，所以在这期间，OV7725 通过它的 PCLK 和 D[0:7]信号线把图像数据存储到 FIFO 中，由于前面复位了写指针，所以图像数据是从 FIFO 的 0 地址开始记录的；
(4) 各行图像数据持续传输至 FIFO，受 HREF 控制的 WE 引脚确保了写入到 FIFO 中的都是有效的图像数据，OV7725 输出完一帧数据时，VSYNC 会再次输出帧有效信号，表示一帧图像已输出完成；
(5) 控制器检测到上述 VSYNC 信号后，可知 FIFO 中已存储好一帧图像数据，这时控制 WEN 引脚为低电平，使得 FIFO 禁止写入，防止 OV7725 持续输出的下一帧数据覆盖当前 FIFO 数据；
(6) 控制器使用RRST复位读指针到FIFO的0地址，然后通过FIFO的RCLK和DO[0:7]引脚，从 0 地址开始把 FIFO 缓存的整帧图像数据读取出来。在这期间，OV7725是持续输出它采集到的图像数据的，但由于禁止写入 FIFO，这些数据被丢弃了；
(7) 控制器使用 WRST 复位写指针到 FIFO 的 0 地址，然后等待新的 VSYNC 有效信号到来，检测到后把 WEN 引脚设置为高电平，恢复 OV7725 向 FIFO 的写入权限，OV7725 输出的新一帧图像数据会被写入到 FIFO 的 0 地址中，重复上述过程。摄像头的例程一般很容易找到，我主要说一下调试摄像头时遇到的问题。

1、因为之前就没有接触过摄像头，刚开始时LCD屏上显示的视频和图片都很模糊，以为是自己的代码写错了，分辨率设的太低，但是VGA和QVGA都试了，结果都是一样的。最后才发现这个摄像头可以调焦，被自己傻哭。

2、因为这个项目的按键一是LCD模式转换，一开始设置的是只显示距离，然后按键转成显示视频，但转成视频模式后就再也转不过来了，一直显示的是视频。经过查阅资料后才知道要先关闭视频后才能转换，于是在按键一的模式二标志位（change）加了关闭摄像头（也就是让摄像头不采集也不显示）代码：

```
if(change==0)

            {               

                  ImagDisp(cam_mode.lcd_sx,cam_mode.lcd_sy,cam_mode.cam_width,cam_mode.cam_height);        //采集并显示   

            }

        else if(change==1)

                break; 
```

## 基于STMF429ZIT6（STMF429I-Disco）的非机动车目标检测

### 通过X-Cube-AI部署HAR/HPE模型例程（STM32f429ZIT6）

为什么可以在STM32上面跑神经网络？

简而言之就是使用STM32CubeMX中的X-Cube-AI扩展包将当前比较热门的AI框架进行C代码的转化，以支持在嵌入式设备上使用。

目前使用X-Cube-AI需要在STM32CubeMX版本5.0以上，支持转化的模型有Keras、TFlite、ONNX、Lasagne、Caffe、ConvNetJS。

Cube-AI把模型转化为一堆数组，而后将这些数组内容解析成模型，和Tensorflow里的模型转数组后使用原理是一样的。

一、环境安装和配置

1.  STM32CubeMX
    
2.  MDK/IAR/STM32CubeIDE
    
3.  F4/H7/MP157开发板
    

***02***

**AI神经网络模型搭建**

这里使用官方提供的模型进行测试，用`keras`框架训练：

<img width="962" height="414" src="/_resources/bd5c115726b3c876615807998d03feba_c7de17b88f9c4ab1b.png"/>

```
https://github.com/Shahnawax/HAR-CNN-Keras
```

2.1 模型介绍

在Keras中使用CNN进行人类活动识别：此存储库包含小型项目的代码。该项目的目的是创建一个简单的基于卷积神经网络(CNN)的人类活动识别(HAR)系统。该系统使用来自3D加速度计的传感器数据，并识别用户的活动。

例如：前进或后退。HAR意为Human Activity Recognition(HAR)system，即人类行为识别。

这个模型是根据人一段时间内的3D加速度数据，来判断人当前的行为，比如走路，跑步，上楼，下楼等，很符合Cortex-M系列MCU的应用场景。使用的数据如下图所示。

![8a510ae6ae14273e3b5d654885049278.png](/_resources/8a510ae6ae14273e3b5d654885049278_74bd12ecc2484b5c8.png)

HAR用到的原始数据

存储库包含以下文件

1.  HAR.py，Python脚本文件，包含基于CNN的人类活动识别（HAR）模型的Keras实现，
    
2.  actitracker_raw.txt、包含此实验中使用的数据集的文本文件，
    
3.  `model.h5`，一个预训练模型，根据训练数据进行训练，
    
4.  evaluate_model.py、Python 脚本文件，其中包含评估脚本。此脚本在提供的 testData 上评估预训练 netowrk 的性能，
    
5.  testData.npy，Python 数据文件，包含用于评估可用预训练模型的测试数据，
    
6.  groundTruth.npy，Python 数据文件，包含测试数据的相应输出的地面真值和
    
7.  README.md.
    

这么多文件不要慌，模型训练后得到`model.h5`模型，才是我们需要的。

***03***

**新建工程**

1\.  这里默认大家都已经安装好了STM32CubeMX软件。

在STM32上验证神经网络模型(HAR人体活动识别)，一般需要STM32F3/F4/L4/F7/L7系列高性能单片机，运行网络模型一般需要3MB以上的闪存空间，一般的单片机不支持这么大的空间。

CUBEMX提供了一个压缩率的选项，可以选择合适的压缩率，实际是压缩神经网络模型的权重系数，使得网络模型可以在单片机上运行，压缩率为8，使得模型缩小到366KB，验证可以通过；

<img width="962" height="543" src="/_resources/741a3c340fb9ae55dd5e384581022d31_87375159f2214edeb.png"/>

然后按照下面的步骤安装好CUBE.AI的扩展包

![b0193f52dba2c956a53b8039f241adcd.png](/_resources/b0193f52dba2c956a53b8039f241adcd_f0058d52c8404e098.png)

这个我安装了三个，安装最新版本的一个版本就可以。

![23b8d856c72fd0ed423b34a231212c39.png](/_resources/23b8d856c72fd0ed423b34a231212c39_6757f1b4ea9d4ea2b.png)

接下来就是熟悉得新建工程了

<img width="962" height="431" src="/_resources/68dd987ce3fce5eb3ea132d678f54b0e_30788c09add94a9ab.png"/>

因为安装了AI的包，所以在这个界面会出现`artificial intelligence`这个选项，点击`Enable`可以查看哪一些芯片支持`AI`

<img width="962" height="588" src="/_resources/c4fc61300f470d18d535a155a9274a24_953dc037200c4ffdb.png"/>

接下来就是配置下载接口和外部晶振了。

<img width="962" height="507" src="/_resources/9bba3a1f731cd5cbf0b9bec5c6783692_f10b3180e2ba4cb1a.png"/><img width="962" height="641" src="/_resources/41bcef1e606d090ed6296f75c68c77ac_2d0c91cb59b54a39a.png"/>

然后记得要选择一个串口作为调试信息打印输出。

<img width="962" height="660" src="/_resources/6f459bd1c34e35a0a676436634332b93_a770c2028afd49f2a.png"/>

选择`Software Packs`，进入后把`AI`相关的两个包点开，第一个**打上勾**，第一个选择`Validation`。

<img width="962" height="236" src="/_resources/e7e1f4f574e5c684009757c0f364c749_1e472de835ce47cbb.png"/><img width="962" height="435" src="/_resources/5675a1671f06d1fbf78b961844425a76_b8f01b52ee3a4689b.png"/>

- System Performance工程：整个应用程序项目运行在STM32MCU上，可以准确测量NN推理结果，CP∪U负载和内存使用情况。使用串行终端监控结果（e.g.Tera Term）
    
- Validation工程：完整的应用程序，在桌面PC和基于STM32 Arm Cortex-m的MCU嵌入式环境中，通过随机或用户测试数据，递增地验证NN返回的结果。与 X-CUBE-A验证工具一起使用。
    
- Application Template工程：允许构建应用程序的空模板项目，包括多网络支持。
    

之后左边栏中的`Software Packs`点开，选择其中的`X-CUBE-AI`，弹出的`Mode`窗口中两个复选框都打勾，`Configuration`窗口中，点开`network`选项卡。

<img width="962" height="641" src="/_resources/8e77aec52665650450313524e68c4ae0_902bb84c95854630a.png"/>

选择刚刚配置的串口作为调试用。

<img width="962" height="586" src="/_resources/ec06f79c9f23a1f0be2aa41c294f3c1b_79c6bde424c44a43a.png"/>

点击`add network`,选择上述下载好的`model`点h5模型，选择压缩倍数8；

<img width="962" height="734" src="/_resources/c74642a2f73eee4cefefd3fd10b0aa3b_55fe22e0194646b6a.png"/>

点击分析，可从中看到模型压缩前后的参数对比

<img width="962" height="532" src="/_resources/d48e4a175385ecd2f8b749a15e9cf3a5_ad7dffd00bef4693b.png"/>

点击validation on desktop 在PC上进行模型验证，包括原模型与转换后模型的对比，下方也会现在验证的结果。

<img width="962" height="520" src="/_resources/c4445a1160eff5baeb72d4d9693bde5f_d4c361dac99d4988a.png"/>

致此，模型验证完成，下面开始模型部署

***04***

**模型转换与部署**

时钟配置，系统会自动进行时钟配置。按照你单片机的实际选型配置时钟就可以了。

<img width="962" height="457" src="/_resources/193c22386b185217bb09de3c21895c83_98d37614603b49a7b.png"/><img width="962" height="477" src="/_resources/dd043ace0fcaf9a91aef9a659f851290_b6ea35e0c4fa47419.png"/><img width="962" height="386" src="/_resources/cd4d67ba808544edbb4aacefaf5df5b8_9ba2d6e9512446dc8.png"/>

最后点击`GENERATE CODE`生成工程。

![271d53eceaee34253b60613e3e6712bb.png](/_resources/271d53eceaee34253b60613e3e6712bb_6bdafda3d4be46a7a.png)

然后在MDK中编译链接。

![019c17b86dc8c60e5d8485da83aa44b7.png](/_resources/019c17b86dc8c60e5d8485da83aa44b7_3f148c86ed1c418b8.png)

选择好下载器后就可以下载代码了。

![bf02598d4466c3b93cfe3837a2b3a85f.png](/_resources/bf02598d4466c3b93cfe3837a2b3a85f_48bd781a34094f648.png)![27d1d89601259bb1b03c94b8e0a46f0b.png](/_resources/27d1d89601259bb1b03c94b8e0a46f0b_2ecbc1c2c1474c79b.png)

然后打开串口调试助手，按下STM32板子的RESET，就可以看到一系列的打印信息了。

![b3e52c5aafef308ff9a1fa73a8c9ff89.png](/_resources/b3e52c5aafef308ff9a1fa73a8c9ff89_b708b000b1c8457ea.png)

代码烧写在芯片里后，回到CubeMX中下图所示位置，我们点击`Validate on target`，在板上运行验证程序，效果如下图，可以工作，证明模型成功部署在MCU中。

![5b8dfbb42b121c4493e27a241c400ae9.png](/_resources/5b8dfbb42b121c4493e27a241c400ae9_2d45b26d0b754796b.png)<img width="962" height="476" src="/_resources/815a40d902cb6eff06bf61f3111dc698_cb322e810f7941c5b.png"/>

### STM32CubeMX、STM32CubeIDE进行X-Cube-AI相关环境搭建问题

- cubemx中通过X-Cube-AI对已经训练好的.h5模型进行analyze需要下载GNU tools for stm32会卡住，需要在Help-Undater Settings...中设置为不使用No Proxy，并且使用手机数据网络而不是校园网。
- cubeide安装自带.h5模型进行analyze需要的组件，cubeide也自带cubemx的功能，但缺点是不能导出除cubeide以外的代码项目。
- cubeide可以直接打开已存在的cubemx工程文件（.ioc）。
- 串口接收到乱码就一种可能，波特率不对。如果有示波器可以用示波器看一下。收一个最小的脉冲，他的脉宽就是实际的波特率的倒数。时钟树的配置使用的是HSE时钟时，在keil中stm32f4xx_it.c-stm32f4xx_hal_conf.h文件可以设置HSE时钟的频率（将25000000改为8000000）。
- cubemx进行validate on target进度条会循环卡死，建议使用cubeide打开对应工程进行。
- 进行validate on target时不要用ST-LINK Utility和MCU进行连接，也不要用XCOM连接串口，会占用串口。
- 完整流程：量化（例如int8量化）工具：NCNN、TensorRT、CMSIS-NN。模型转换：yolo -> keras -> tflite。模型导入：X-CUBE-AI。

X-cube-AI支持直接导入的模型文件后缀名：

- .h5
- .tflite
- .onnx

常见神经网络、深度学习和机器学习的模型文件后缀名：

- ONNX (.onnx, .pb, .pbtxt)
- Keras (.h5, .keras)
- CoreML (.mlmodel)
- Caffe2 (predict_net.pb, predict_net.pbtxt)
- MXNet (.model, -symbol.json)
- TensorFlow Lite (.tflite)

常见深度学习框架的文件后缀名：

- Caffe (.caffemodel, .prototxt)
- PyTorch (.pth)
- Torch (.t7)
- CNTK (.model, .cntk)
- PaddlePaddle(`dw__model__`)
- Darknet (.cfg)

### X-Cube-AI用到的Python库安装

CMD中对Python环境安装包/库：

```
conda activate 环境名
conda install 库名
:: anconda方法。

activate 环境名
pip install 库名 -i 镜像源地址
:: pip方法。
```

使用pip安装以下库：

```
pip install torch torchvision torchaudio -i https://pypi.tuna.tsinghua.edu.cn/simple some-package
:: 使用清华源，torch torchvision torchaudio三个库。
pip install typing-extensions==4.3.0
:: 解决报错，安装typing-extensions库。
```

### HAL库的I2C相关函数

IIC主要函数和串口等通讯协议主要函数基本相同，一个是发送，一个是接受，在HAL库中，发送和读取主要有三个方式，第一种读写是超时读写，第二种是中断读写，第三个是DMA中断读写，其中第一种阻塞方式发送，CPU资源占有较大，后面两种与中断结合发送接收，CPU资源使用较少，常用函数接口代码介绍如下：

```
// 阻塞IIC发送、接受代码原型
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);

// 非阻塞普通中断IIC发送、接受代码原型
HAL_StatusTypeDef HAL_I2C_Master_Transmit_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Master_Receive_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);

HAL_StatusTypeDef HAL_I2C_Mem_Write_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);

// 非阻塞DMA中断IIC发送、接受代码原型
HAL_StatusTypeDef HAL_I2C_Master_Transmit_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Master_Receive_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);

HAL_StatusTypeDef HAL_I2C_Mem_Write_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Read_DMA(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
```

HAL_I2C_Master_Transmit（）主模式下以阻塞模式传输大量数据。

传入参数：hi2c—使用的IIC设备
DevAddress—从机地址
pData—写入数据的地址
Size—写入数据长度
Timeout—传输时间上限
HAL_I2C_Master_Receive（）主模式下以阻塞模式接受大量数据。

传入参数：hi2c—使用的IIC设备
DevAddress—从机地址
pData—读入数据存储首地址
Size—读取数据长度
Timeout—读取时间上限
HAL_I2C_Mem_Write（）将阻塞模式下的大量数据写入从机特定的内存地址

传入参数：hi2c—使用的IIC设备

DevAddress—从机地址

MemAddress—写入内存首地址（写入过程中会自加）

MemAddSize—写入内存的大小（8位或16位）

pData—指向数据缓冲区的指针

Size—要发送的数据量

Timeout—超时持续时间

HAL_I2C_Mem_Read（）将阻塞模式下的大量数据写入从机特定的内存地址

传入参数：hi2c—使用的IIC设备

DevAddress—从机地址

MemAddress—读取内存首地址（读取过程中会自加）

MemAddSize—读取内存的大小（8位或16位）

pData—指向数据缓冲区的指针

Size—要发送的数据量

Timeout—超时持续时间

HAL_I2C_IsDeviceReady（）检查目标设备是否准备好通信

传入参数：hi2c—使用的IIC设备
DevAddress—从机地址
Trials—测试次数
Timeout—超时持续时间

三种方式例程：

```
/* USER CODE BEGIN PV */
//读写地址
#define AT24C02_Write 0xA0
#define AT24C02_Read  0xA1
//三次写入的字符串
unsigned char str1[]={"jeck666"};
unsigned char str2[]={"1234567"};
unsigned char str3[]={"abcdefg"};
//读取缓存区
uint8_t ReadBuffer[50];
/* USER CODE END PV */

  /* USER CODE BEGIN 2 */
	HAL_UART_Transmit_IT(&huart1,"Init Ok!\r\n",sizeof("Init Ok!\r\n"));
	HAL_Delay(100);
	//阻塞方式写入读取
	if(HAL_I2C_Mem_Write(&hi2c1,AT24C02_Write,0,I2C_MEMADD_SIZE_8BIT,str1,sizeof(str1),1000)==HAL_OK)
		HAL_UART_Transmit_IT(&huart1,"STR1_Write_OK\r\n",sizeof("STR1_Write_OK\r\n"));
	HAL_Delay(1000);
	HAL_I2C_Mem_Read(&hi2c1,AT24C02_Read,0,I2C_MEMADD_SIZE_8BIT,ReadBuffer,sizeof(str1),1000);
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&huart1,ReadBuffer,sizeof(str1));
	HAL_Delay(1000);
	//中断方式写入读取
	if(HAL_I2C_Mem_Write_IT(&hi2c1,AT24C02_Write,0,I2C_MEMADD_SIZE_8BIT,str2,sizeof(str2))==HAL_OK)
		HAL_UART_Transmit_IT(&huart1,"STR2_Write_OK\r\n",sizeof("STR2_Write_OK\r\n"));
	HAL_Delay(1000);
	HAL_I2C_Mem_Read_IT(&hi2c1,AT24C02_Read,0,I2C_MEMADD_SIZE_8BIT,ReadBuffer,sizeof(str2));
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&huart1,ReadBuffer,sizeof(str2));
	HAL_Delay(1000);
	//DMA中断方式写入读取
	if(HAL_I2C_Mem_Write_DMA(&hi2c1,AT24C02_Write,0,I2C_MEMADD_SIZE_8BIT,str3,sizeof(str3))==HAL_OK)
		HAL_UART_Transmit_IT(&huart1,"STR3_Write_OK\r\n",sizeof("STR3_Write_OK\r\n"));
	HAL_Delay(1000);
	HAL_I2C_Mem_Read_DMA(&hi2c1,AT24C02_Read,0,I2C_MEMADD_SIZE_8BIT,ReadBuffer,sizeof(str3));
	HAL_Delay(1000);
	HAL_UART_Transmit_IT(&huart1,ReadBuffer,sizeof(str3));
	HAL_Delay(1000);
  /* USER CODE END 2 */
```

### OV7725摄像头的使用（带FIFO）（未检验）

#### 通讯方法

- 外部控制器对 OV7725 寄存器的配置参数是通过串行摄像头控制总线 SCCB（Serial Camera Control Bus，SCCB） 总线传输过去的，SCCB 总线跟 I2C 十分类似，所以在 STM32 驱动中可以直接使用片上 I2C 外设与它通讯。SCCB 的起始信号、停止信号及数据有效性与 I2C 完全一样，在 SCCB 协议中定义的读写操作与 I2C 也是一样的。
- OV7725输出图像时则使用 VGA 或QVGA 时序。 VGA 在输出图像分辨率为 480 * 640（30w摄像头），则一帧图像占的字节数为：480 * 640 * 2(一个像素占两字节) = 614400。QVGA 是 Quarter VGA，其输出分辨率为 240 * 320，则一帧图像占的字节数为：240 * 320 * 2(一个像素占两字节) = 153600。153600/1024 = 150.0 KB。STM32自带的RAM空间一般连一帧图像的大小都没有。因为 OV系列摄像头模块的像素时钟PCLK最高可达 24Mhz，我们用 STM32的 IO 口直接抓取，是非常困难的，也十分占耗 CPU（可以通过降低 PCLK 输出频率，来实现 IO 口抓取，但是不推荐这样操作）。所以，我们并不是采取直接抓取来自 OV7670 的数据，而是通过 FIFO 读取，PZ-OV7670 摄像头模块自带了一个 FIFO 芯片，用于暂存图像数据，有了这个芯片，我们就可以很方便的获取图像数据了。
- FIFO（First In First Out，FIFO）本质是一种RAM存储器，支持同时写入和读出数据。AL422B-FIFO芯片的容量大小为393216字节，至少能够缓存两帧 240 * 320 的图像，但一帧 480 * 640的图像都无法缓存。
- FIFO用于主频不够的MCU读取高速传输的数据时的中转暂存，也可以用于RAM不够的MCU分行读取数据时的暂存。
- F4以上MCU可以使用数字摄像头接口DCMI（Digital camera interface）。DCMI提供的外部接口都是输入的，有四种：DCMI_D#是数据接口，DCMI_PIXCLK是像素时钟（接摄像头模块的PCLK），DCMI_HSYNC是行同步信号/水平同步信号（接摄像头模块的HSYNC/HERF），DCMI_VSYNC是帧同步信号/垂直同步信号（接摄像头模块的VSYNC）。
- DCMI方案和FIFO方案，可以一起使用（也是走DCMI接口）或者单用。DCMI方案需要单片机有DCMI外设支持，引脚接线固定。而FIFO方案则需要摄像头有FIFO芯片支持，但是引脚均可以以普通GPIO实现功能。
- 因为OV摄像头本身可能没有晶振，如果没有晶振，则需要外接晶振或者其他时钟信号给XCLK引脚提供24MHz的时钟信号，可以使用STM32的MCO1（在RCC中配置）。

#### 引脚设定与接线

OV7725引脚：

![205b39c910235ec4c2e683436af1efe8.png](/_resources/205b39c910235ec4c2e683436af1efe8.png)

![9f83987b78cb642c6ddd70868cdc4f41.png](/_resources/9f83987b78cb642c6ddd70868cdc4f41.png)

- PCLK是像素时钟，也就是OV7725的获取像素并输出的时钟频率。
- XCLK是外部时钟的输入引脚，可接外部晶振。
- VSYNC（场同步信号引脚）使能一次代表一次将图像写入到FIFO的过程结束。此时STM32可以才根据自己的时钟频率（而不是OV7725的PCLK的较快的时钟频率）来较慢地（相对于OV7725的工作频率）且一行一行地（避免RAM空间不够）读取FIFO中的数据，以使用显示屏或者SD卡等。
- HERF/HSYNC（行同步信号引脚）。
- NC（no connect）表示没有引脚。

FIFO引脚（名称中的`/`代表低电平有效）：

![6a9af220f7b6e6c73c344979984e1e6d.png](/_resources/6a9af220f7b6e6c73c344979984e1e6d.png)

- /WE引脚为低电平时才能对FIFO芯片进行写入操作。/WE为高电平时为无效数据。
- /WRST引脚为低电平时才能使得每次对FIFO芯片的写入都从起始位的地址开始，这样可以使得摄像头工作时对FIFO缓存的写入是覆盖的，更符合摄像头工作的流程。
- DI是输入引脚，DO是输出引脚。DI接口一般在内部已经连接到OV7725的D#数据输出口了。DO引脚一般连接MCU以使得MCU可以获取OV7725模块得到的数据。
- WCK和RCK分别为写入和输出/读取操作的时钟，指示工作间隔，一般由连接的MCU提供。
- /RE引脚和/OE引脚均为低电平时才能使FIFO芯片进行输出/读取操作，两者近乎等同。
- /RRST引脚为低电平时才能使得FIFO芯片每次进行输出/读取操作时都从起始位的地址开始，和/WRST对应。
- OV7725的PCLK引脚应该连接FIFO的WCK引脚，因为FIFO的写入频率和OV7725的像素频率应当一致，在原理图中可以看到相互连接。而MCU仅需要读取FIFO中的数据，因此仅引出了RCK引脚。
- OV7725的HREF引脚应该连接FIFO的/WE引脚，因为FIFO的写入周期长度应该和OV7725的HREF周期长度应当一致，在原理图中可以看到与WEN引脚一起与/WE引脚连接，因此引出WEN和HREF引脚，或仅引出WEN引脚（不需要HERF引脚，其默认为高电平）。WEN即为/WE引脚取反。因此当WEN和HREF引脚均为高电平时（通过与非门共同控制），代表/WE引脚为低电平。
- SIO_C即为SCL引脚，SIO_D即为SDA引脚，构成SCCB总线。

![62f0bf91de551b0cf46211eda9049024.png](/_resources/62f0bf91de551b0cf46211eda9049024.png)

![9ec0629b96efa37d2eff0cf92eace6a0.png](/_resources/9ec0629b96efa37d2eff0cf92eace6a0.png)

OV7725和AL422B原理图：

![f9dbb93c1b14c1d9bfb6cb3590f2e29e.png](/_resources/f9dbb93c1b14c1d9bfb6cb3590f2e29e.png)

带有AL422B-FIFO芯片的OV7725模块的引脚（包含了部分AL422B和部分OV7725的引脚）：

![3da5a00901f74d118417f0c196a3a204.png](/_resources/3da5a00901f74d118417f0c196a3a204.png)

参考接线：

![ea9d3cffa8e2728733ff1c028a7ef401.png](/_resources/ea9d3cffa8e2728733ff1c028a7ef401.png)

- 所有接口均可以使用自定义IO口来实现。
- 使用CubeMX进行配置时，使用DCMI（对于8位宽摄像头，有8+3个IO口自动配置好）和I2C配置IO口（SCL和SDA，2个IO口自动配置好），然后再自定义RCC的MCO（master clock output）口提供XCLK时钟（1个IO口）。再加上供电的3.3V和GND。

### OV2640摄像头使用（不带FIFO）（已验证）

#### 硬件接线与CubeMX配置

![25B3F06E432C5110E661DB16C55EBE1F.png](/_resources/25B3F06E432C5110E661DB16C55EBE1F.png)

使用杜邦线按照名称一一连接，并注意把排线撕开以免产生信号干扰：

![bc679ac3858f1523a78adf9a191241a1.png](/_resources/bc679ac3858f1523a78adf9a191241a1.png)

- OV2640完全能够由板子的3.3V供电（官方板子供电电压不足额，用万用表测试得到）正常工作（至少SCCB匹配能够成功），注意如果使用其他供电源GND需要与3.3V共地。但是不同供电电压的表现不同，在高波特率下，SCCB匹配仍能成功，但图像传输的数据在使用外部标准3.3V供电时会出现错误使得无法通过串口助手的JPEG编码转换。

- OV2640对应的DCMI配置，VSYNC帧同步信号高电平有效，HREF行同步信号高电平有效，输出数据在PCLK的下降沿输出（即上升沿的时候，MCU才可以采集）
这样，STM32F4的 DCMI 接口就必须设置为：VSYNC帧同步信号低电平有效，HSYNC行同步信号低电平有效和PIXCLK上升沿有效。
也就是说：STM32通过DCMI接口和OV2640进行通讯时，OV2640负责输出采集到的帧，也就是图片，STM32负责采集；所以双方的设置应该恰好相反，OV2640设置PCLK下降沿输出，那么与此同时STM32就必须设置上升沿捕获：

![a850cd2f9e8da7b30c75ee07f21dda3d.png](/_resources/a850cd2f9e8da7b30c75ee07f21dda3d.png)

![372b491041f2faffcbb0d5ab44c7bd9a.png](/_resources/372b491041f2faffcbb0d5ab44c7bd9a.png)

- 配置DCMI完成，获得对应的引脚：

![418d27fe739dc617aad438a7b18aa111.png](/_resources/418d27fe739dc617aad438a7b18aa111.png)

- PWDN和RESET引脚使用普通输出GPIO（设置为推挽输出）即可。不使用硬件I2C，使用普通输出GPIO进行模拟SCCB，SCCB的SCL引脚使用普通输出GPIO（推挽输出），SCCB的SDA引脚使用普通输出（一定需要开漏输出）。SCCB_SCL引脚接OV2640的SCL，SCCB_SDA引脚接OV2640的SDA：

![363068422711b65e9c4ff86e2a911d94.png](/_resources/363068422711b65e9c4ff86e2a911d94.png)

- RCC配置（配置了MCO可以用于为不内置时钟的摄像头提供时钟，本次项目OV2640自带时钟）：

![093656dd8f33ee6adaacd6dc1510613c.png](/_resources/093656dd8f33ee6adaacd6dc1510613c.png)

![cdbe8d3498ca674384581fdc3c685df7.png](/_resources/cdbe8d3498ca674384581fdc3c685df7.png)

DMA设置：

![3d63304a22f6d059490e090e6d529748.png](/_resources/3d63304a22f6d059490e090e6d529748.png)

USART设置：

![3d1d00d3e3a8dd791c019e6655362ae8.png](/_resources/3d1d00d3e3a8dd791c019e6655362ae8.png)

### TFT触摸液晶屏（屏幕和触摸功能均通过SPI驱动）（带SD卡槽，2.4寸，驱动IC为ILI9341）

STM32CubeMX的SPI配置为全双工主机。

SPI 协议是由摩托罗拉公司提出的通讯协议 (Serial Peripheral Interface)，即串行外围设备接口，是一种高速全双工的通信总线。它被广泛地使用在 ADC设备、LCD 等设备与 MCU 间，要求通讯速率较高的场合。

SPI 通讯使用3条总线及片选线，3 条总线分别为 SCK、MOSI、MISO，片选线为 SS/CS ，它们的作用介绍如下：

(1) SS/CS（Slave Select）

从设备选择信号线，常称为片选信号线，也称为 NSS、CS，以下用 NSS表示。当有多个 SPI 从设备与 SPI 主机相连时，设备的其它信号线 SCK、MOSI 及 MISO同时并联到相同的 SPI 总线上，即无论有多少个从设备，都共同只使用这 3 条总线；而每个从设备都有独立的这一条 NSS 信号线，本信号线独占主机的一个引脚，即有多少个从设备，就有多少条片选信号线。I2C 协议中通过设备地址来寻址、选中总线上的某个设备并与其进行通讯；

而SPI 协议中没有设备地址，它使用 NSS 信号线来寻址，当主机要选择从设备时，把该从设备的 NSS 信号线设置为低电平，该从设备即被选中，即片选有效，接着主机开始与被选中的从设备进行 SPI 通讯。所以 SPI 通讯以 NSS 线置低电平为开始信号，以NSS 线被拉高作为结束信号。（在LCD中，片选线有很多名称，CS，SS，NSS都是指片选）

(2) SCK (Serial Clock)：

时钟信号线，用于通讯数据同步。它由通讯主机产生，决定了通讯的速率，不同的设备支持的最高时钟频率不一样，如 STM32 的 SPI 时钟频率最大为 fpclk/2，两个设备之间通讯时，通讯速率受限于低速设备。

(3) MOSI (Master Output，Slave Input)：

主设备输出/从设备输入引脚。主机的数据从这条信号线输出，从机由这条信号线读入主机发送的数据，即这条线上数据的方向为主机到从机。 （与IIC相比，这个就是信号线，由主机向从机发送数据，即SDA）

(4) MISO(Master Input,，Slave Output)：

主设备输入/从设备输出引脚。主机从这条信线读入数据，从机的数据由这条信号线输出到主机，即在这条线上数据的方向为从机到主机。 (从机向主机发送数据，使用触摸屏时需要这根线。如果单纯使用LCD来显示，这根线可以不接)。

多设备的SPI通讯接线：

![9e0cc5b3d8477f8ad237c7e2b24df601.png](/_resources/9e0cc5b3d8477f8ad237c7e2b24df601.png)

SPI时序图：

![12dd6fcef97d382f76e3b139e25b96a4.png](/_resources/12dd6fcef97d382f76e3b139e25b96a4.png)

SPI 有四种工作模式，通过串行时钟极性(CPOL)和相位(CPHA)的搭配来得到四种工作模式：
        ①、CPOL=0，串行时钟空闲状态为低电平。
        ②、CPOL=1，串行时钟空闲状态为高电平，此时可以通过配置时钟相位(CPHA)来选择具
体的传输协议。
        ③、CPHA=0，串行时钟的第一个跳变沿(上升沿或下降沿)采集数据。
        ④、CPHA=1，串行时钟的第二个跳变沿(上升沿或下降沿)采集数据。
		
#### 注意事项

- 按照名称一一接线，其中LED、DC/RS、RESET、CS需要自定义GPIO，速度为High，其他默认即可。
- 通电VCC和GND是不会立刻亮的，LED背光引脚高电平后，会一直亮白光（背光），而控制背光前面的液晶才能使得其显示不同颜色的光。
- TFT面板和LED背光：
- - 先说TFT：TFT是指TFT（ThinFilmTransistor）是指薄膜晶体管，意即每个液晶像素点都是由集成在像素点后面的薄膜晶体管来驱动。是主动驱动的现在一种。与之对应的是很早那种黑白显示为被动驱动方式。现在基本上解析度高一点彩屏的均是使用的TFT-LCD。
- - 再说LED背光，由于液晶显示是一种非主动发光的显示技术，也就是说，液晶的面板只是一个光开关，控制每个像素点的开关来显示图像的。那在这个光开关背后需要一个面光源来发光。这个面光源就被叫做背光。背光的光源一般有两种，一种是FCCL（冷阴极管）和LED（发光二极管）。而LED背光就是光源是LED的而已。
- - 这两个概念是不同领域的，是可以并存的，现在大部分显示器都是LED背光，TFT的面板。
- 单个引脚电平控制简单，但是通过SPI传输指令不行（但是仍然进行了对应操作，通过调试信息可得。只是液晶屏没反应），推测是SPI根本没连接成功。
- 大多SPI的芯片，MISO是没有驱动力的，所以，MISO是需要配上拉电阻的（引脚设置为上拉）。而由于大多SPI芯片，多不是5V，所以原则上，单片机端MISO需要配制成输入。CS引脚最好也设置为上拉。
- SPI读取，前几个dummy值（0xff或0），有效在最后两字节。间隔读出0和0xFF，有可能是因为没有配置SPI引脚上拉。SPI的MISO（主机接收）一般没有上拉电阻，需要引脚配置上拉，CS也配置上拉会更保险。
- SPI相关引脚均上拉后发现一直读出0xFF，再进行拔掉测试，发现仍一直读出0xFF。但是在上拉前间隔读出0和0xFF，拔掉测试发现一直读出0，太奇怪了。
- SPI和I2C一样，先读ID，读不出ID外设的初始化都别想过。ILI9341芯片手册的读ID指令：

![3f48eca9721b1937e9b1759122024646.png](/_resources/3f48eca9721b1937e9b1759122024646.png)

- 每一种STM32芯片的某些引脚，通过CubeMX配置成GPIO_Output还是可能不能进行电平的编辑，这类似于STM32F103ZET6的某些用于JTAG的引脚要复用为其他功能的引脚时需要进行额外的操作的原理应该是相同的。而且，某些引脚还出于未知原因，在相同的配置下，电平的转换是有延迟的，可能无法满足通信协议的时许要求，这通过逻辑分析仪可以查看到。
- TFTLCD成功调通（通过更换适合的引脚，直到逻辑分析仪显示单片机输出的SPI信号时序完全正确，符合协议的时序图即可），但读ID指令仍无法返回ID，仍返回0和0xFF。而且时序正确且能读出单片机的输出的SPI信号为准确的话，也不需要进行提前的寄存器清空，SPI相关引脚的上拉电阻的配置，SPI频率倒是确实要压到10M以下最好。
- SPI每个时钟变化进行一个bit的操作。因此可以通过SPI波特率计算带宽。
- 图像相关的编程，真的需要每一步都通过各种调试手段确认完全无误。 对图像的通道值编辑和坐标编辑等操作都是极易出错的。因此，在图像处理相关编程中，作为调试工具的LCD屏的存在必不可少。

### 逻辑分析仪的使用

通过各种方法验证每个过程的变量的真实值，是调试的核心。逻辑分析仪如此，串口输出变量如此，显示屏显示图像如此。

逻辑分析仪是完美符合模块化思想的产品。假设各个模块化的外设内部都没有任何问题，那么只要单片机输出的信号符合产品手册的要求，那么就必定能够让外设正常工作。示波器也一样，但是示波器更适合于硬件电路层面上的debug。

测试方法：逻辑分析仪的引脚均未连接时，PulseView上位机软件默认显示每个通道均为一直的高电平。每次单独将逻辑分析仪的一个通道引脚与逻辑分析仪自身的GND引脚连接，如果对应通道的电平变为一直的低电平则此通道测试成功。

使用方法：逻辑分析仪的GND引脚与单片机的GND引脚相连（共地），而逻辑分析仪的通道引脚与需要进行逻辑信号采样的单片机引脚相连。PulseView上位机软件可以设置采样数据量和频率，还有通过设置对应通道对特定的通信协议进行解码。可惜在信号密集数据量太大时上位机软件会很卡。

### SDRAM的使用（ST官方STM32F429I-DISCO开发板自带IS42S16400J芯片）（用于显示图像，也就是将SDRAM作为显存使用）

- 标准的SDRAM一般都是4个BANK，总容量为1Mbit x 16-bit x 4个bank = 67,108,864 bits = 64-Mbit ，每个BANK的组成：4096 rows(12 bit) x 256 columns(8 bit) x 16 bits(总线宽度/数据宽度) =16Mbit, 对应的外部引线是12行8列。
- STM32F429I-DISCO开发板的板载IS42S16400J的起始地址为0xD0000000。

#### FMC/FSMC-SDRAM关键参数

SDRAM的控制参数：

![d9a2edef51b52e064271b3f8cb3fee75.png](/_resources/d9a2edef51b52e064271b3f8cb3fee75.png)

通过原理图，得到STM32F429I-DISCO板载IS42S16400的对应引脚（SDCKE1和SDNE1）：

![2cf6ecdafab762748a099ab02319254f.png](/_resources/2cf6ecdafab762748a099ab02319254f.png)

通过IS42S16400J手册，得到bank数（4）和数据宽度（16bit）：

![dbce79ff7a871218665d7cc9b8e3b36f.png](/_resources/dbce79ff7a871218665d7cc9b8e3b36f.png)

通过IS42S16400J手册，得到地址宽度（12行与8列，但分时复用，因此地址宽度为12bit）：

![76df36a16ebe0b018d1ee58f8cebc9a6.png](/_resources/76df36a16ebe0b018d1ee58f8cebc9a6.png)

CL（CAS Latency）：

- CL：2 memory clock cycles/3 memory clock cycles。
- - 在选定列地址后，就已经确定了具体的存储单元，剩下的事情就是数据通过数据I/O通道（DQ）输出到内存总线上了。但是在CAS发出之后，仍要经过一定的时间才能有数据输出，从CAS与读取命令发出到第一笔数据输出的这段时间，被定义为CL（CAS Latency，CAS潜伏期）。由于CL只在读取时出现，所以CL又被称为读取潜伏期（RL，Read Latency）。CL的单位与tRCD一样，为时钟周期数，具体耗时由时钟频率决定。
- - 数据写入的操作也是在tRCD之后进行，但此时没有了CL（记住，CL只出现在读取操作中）。

写保护（write protection）：

- 一般为Disable。

FMC驱动SDRAM，只支持两种分频。获得SDRAM时钟周期（SDRAM common clock）：

- SDCLK period/clock cycle = 2 x HCLK periods/clock cycle，SDCLK frequency = 1/2 HCLK frequency
- SDCLK period/clock cycle = 3 x HCLK periods/clock cycle，SDCLK frequency = 1/3 HCLK frequency
- 因此SDRAM时钟的单个时钟周期可为2个或3个HCLK时钟周期，也就是说，SDRAM时钟的频率可为HCLK时钟二分频或三分频。

突发读（SDRAM common burst read）：

- 一般为Disable。

读管道延迟（SDRAM common read pipe delay）：

- 一般为1 HCLK clock cycle。

SDRAM的时序参数与计算方法：

- 按照SDCLK为90MHz的话（HCLK二分频），每个时钟周期就是11.1ns。

![a4a888c1f7ea1c2bf51a8850dfba9b0d.png](/_resources/a4a888c1f7ea1c2bf51a8850dfba9b0d.png)

- tMRD（/Load mode register to active delay）：2 Clock cycles。
- tXSR（Exit Self-Refresh to Active Time/Exit self-refresh delay）：min=70ns (7x11.10ns) 。7 Clock cycles。
- tRAS（Command Period(ACT to PRE)/Self-refresh time）：min=42ns (4x11.10ns)，max=120k (ns)。4 Clock cycles。
- tRC（Command Period(REF to REF/ACT to ACT)/SDRAM common row cycle delay）：min=63 ns (7x11.10ns)。7 Clock cycles。
- tWR/tDPL（Input Data To Precharge Command Delay time/Write recovery time）：2 Clock cycles。
- - 数据并不是即时地写入存储电容，因为选通三极管（就如读取时一样）与电容的充电必须要有一段时间，所以数据的真正写入需要一定的周期。为了保证数据的可靠写入，都会留出足够的写入/校正时间（Write Recovery Time，tWR），这个操作也被称作写回（Write Back）。
- tRP（Command Period(PRE to ACT)/SDRAM common row precharge delay）：min=15ns (2x11.10ns)。2 Clock cycles。
- - 在发出预充电命令之后，要经过一段时间才能允许发送RAS行有效命令打开新的工作行，这个间隔被称为tRP（Precharge command Period，预充电有效周期）。和tRCD、CL一样，tRP的单位也是时钟周期数，具体值视时钟频率而定。
- tRCD（Active Command to (Read/Write) Command Delay Time/Row to column delay）：min=15ns (2x11.10ns)。2 Clock cycles。
- - 在发送列读写命令时必须要与行有效命令有一个间隔，这个间隔被定义为tRCD，即RAS to CAS Delay（RAS至CAS延迟），大家也可以理解为行选通周期，这应该是根据芯片存储阵列电子元件响应时间（从一种状态到另一种状态变化的过程）所制定的延迟。tRCD是SDRAM的一个重要时序参数，可以通过主板BIOS经过北桥芯片进行调整，但不能超过厂商的预定范围。广义的tRCD以时钟周期（tCK，Clock Time）数为单位，比如tRCD=2，就代表延迟周期为两个时钟周期，具体到确切的时间，则要根据时钟频率而定，对于PC100SDRAM，tRCD=2，代表20ns的延迟，对于PC133则为15ns。
- - 和SRAM不同，驱动SDRAM的行选和列选的地址线是分时复用的。读写操作发送的是列地址，所以在读写操作之前需要确定行地址和bank。确定行地址后还需要tRCD时间才能把该行打开才可以对该行数据进行读写操作。

#### STM32CubeMX配置方法

STM32CubeMX配置SDRAM1：

![370144add904ceb914e3efb5cd38bd05.png](/_resources/370144add904ceb914e3efb5cd38bd05.png)

![e69ca0c027aa930dcfcf4f6d21b157b5.png](/_resources/e69ca0c027aa930dcfcf4f6d21b157b5.png)

![38706382b7f3fdc373810d8d73f64de3.png](/_resources/38706382b7f3fdc373810d8d73f64de3.png)

![5b8501dbbe865254bdb8c8c3bcd301ce.png](/_resources/5b8501dbbe865254bdb8c8c3bcd301ce.png)

#### STM32CubeMX工程修改

使用SDRAM，在stm32f4xx_hal_sdram库中可以在注释看到FMC初始化后SDRAM还需要初始化，可以用初始化函数HAL_SDRAM_SendCommand()和初始化结构体FMC_SDRAM_CommandTypeDef来初始化。

查看IS42S16400J的手册的初始化部分：

- 上电
- 初始化时钟延迟至少100us
- 在一个无效命令或空操作后执行执行预充电延迟至少100us，所有bank都需要被预充电
- 执行两个自动刷新周期
- 配置模式寄存器

配置模式寄存器：

![543d9bd9d0ee586cdcb21eab7803dc6b.png](/_resources/543d9bd9d0ee586cdcb21eab7803dc6b.png)

- A[2-0]：突发模式读取数，F429的SDRAM不支持突发模式，所以一次读一个，设置为000
- A3 ：1为间隔模式即每次读取4个字节会延迟一会儿，0为连续模式，我们选择连续模式，设置为0
- A[6-4]：相当于CAS，我配置的数和cubeMX一样，设置为011
- A[8-7]：没用，不管
- A9：突发模式写，关了，设置1
- 配置模式寄存器为0b10 0011 0000，即 0x230。

查看STMF4手册，初始化完成FMC和IS42S16400J之后还得配置IS42S16400J的刷新率：

- FMC_SDRTR寄存器：

![e8a199371172d192f1f7e9e962fd63cb.png](/_resources/e8a199371172d192f1f7e9e962fd63cb.png)

- F429频率为180Mhz，经过cubeMX的二分频，得到SDRAM时钟频率就是90Mhz。

![fe233c676af7310aca542953f68cd496.png](/_resources/fe233c676af7310aca542953f68cd496.png)

- IS42S16400J刷新周期（64ms）和行数（4096）可以在IS42S16400J得手册中查找：

![3af1447318f1e5fb1cccc525277c8bbf.png](/_resources/3af1447318f1e5fb1cccc525277c8bbf.png)

- 可以通过计算得到Refresh Rate：

$$\frac{64 \times 10^{-3} \text{ s}}{4096 \text{ rows}} \times 90 \times 10^{6} Hz = 1406.25 \approx 1407$$

fmc.c：

```
/* FMC initialization function */

void MX_FMC_Init(void)
{
  // MX_FMC_Init()函数中自动生成的代码省略。

  /* USER CODE BEGIN FMC_Init 2 */
  //初始化时钟
	FMC_SDRAM_CommandTypeDef Command;
	Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
	Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	Command.AutoRefreshNumber = 1;
	Command.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand( &hsdram1,  &Command,  0xffff);
	HAL_Delay(1);
	
	//在一个无效命令或空操作后执行执行预充电
	Command.CommandMode = FMC_SDRAM_CMD_PALL;
	Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	Command.AutoRefreshNumber = 1;
	Command.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand( &hsdram1,  &Command,  0xffff);
	HAL_Delay(1);
	
	//执行两个自动刷新周期
	Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	Command.AutoRefreshNumber = 2;
	Command.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand( &hsdram1,  &Command,  0xffff);
	HAL_Delay(1);

  //配置模式寄存器
	Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
	Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
	Command.AutoRefreshNumber = 1;
	Command.ModeRegisterDefinition = 0x230;
	HAL_SDRAM_SendCommand( &hsdram1,  &Command,  0xffff);
	HAL_Delay(1);
  
	//配置IS42S16400J刷新率
  HAL_SDRAM_ProgramRefreshRate(&hsdram1 , 1407);
  /* USER CODE END FMC_Init 2 */
}
```

main.c：

```
/* USER CODE BEGIN PV */
uint32_t pbuf[(1024*1024*64)/32] __attribute__((at(0xD0000000+0x0)));
// 语法：变量类型 x __attribute__((at(ADDR))) 可以将变量x存放在指定的地址ADDR中。
// 0x0为地址偏移量，可自定义。
int Flag=0;
/* USER CODE END PV */

/* USER CODE BEGIN 2 */

/*SDRAM测试程序开始*/
	for(uint32_t i=0;i<((1024*1024*64)/32);i++)
	{
		pbuf[i]=1; // 给SDRAM中的数组赋值。
	}
	for(uint32_t i=0;i<((1024*1024*64)/32);i++)
	{
		if(pbuf[i]!=1) // 判断SDRAM中的数组是否与赋值的一致。
		{
			Flag=1;
			break;
		}
	}
	
	if(Flag==1)
	{
		HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_13);
		HAL_Delay(500);
	}
	if(Flag==0)
	{
		HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_14);
		HAL_Delay(500);
	}
/*SDRAM测试程序结束*/
	
/* USER CODE END 2 */
```

### 注意事项

- C:\Users\用户名\STM32Cube\Repository\STM32Cube_FW_F4_V1.27.1\Projects\STM32F429I-Discovery\Examples，有ST官方对应不同芯片型号和外设的例程（虽然没啥用）。
- 注意串口的16位显示会将所有内容都16位化，即使是普通的打印内容。
- 使用普通GPIO口模拟SCCB，不要用硬件IIC，有几率MID只能读出0xFF（也就是SCCB/I2C无法匹配）。全是FF说明从设备没有应答，没有握手成功。因为，I2C总线的地址线和数据线都是上拉了的，没有数据时应该就是高。由此可见，主机接收数据全是FF是因为从设备没有响应的原因。也就是说，有两种可能，一个是从设备故障；二是主机发送的从设备的地址不正确导致从设备不响应。设备故障包括接线问题，时序问题等。
- 参考：结论：网上看有些人也在用cubemx官方的I2C的DMA，我却无法正常使用DMA。目前不知道是什么原因。也可能是画的板子有问题。解决：不使用DMA可以正常使用硬件I2C。总结：遇到问题，多用逻辑分析仪或者示波器分析。
- 在使用杜邦线连接官方板子（没有摄像头底座的板子）时，需要保证OV2460全部线都连紧了，且没有断路，数据传输是非常非常不稳定的。出现SCCB无法匹配（读取厂商ID，MID一直为0xFF或65535即为SCCB没连接上或读取失败。读取MID为32674、HID为9794即为成功连接）时，可以尝试对齐并用力压紧引脚，或寻找合适的角度使得连接正常。
- 引脚连接不稳可能会使得SCCB无法匹配。（SCCB引脚或供电引脚）
- 引脚连接不稳也会使得SCCB匹配后传输图像数据的速率不同（从几百B到十几KB都有）。
- 引脚连接不稳也会使得传输的图像数据的十六位形式变成大部分值为FF而不是具体的值。
- 引脚连接不稳也会使得直接完全接收不到任何图像数据。
- 引脚连接不稳也会使得接受到的图像马赛克花屏、只有部分图像、撕裂现象。
- SCCB匹配后传输的图像数据连JPEG文件头都没有，需要进行硬件排查（例如D#的断线）。利用串口输出图像数据，jpeg的SOI（start of image） 为FF D8，EOD（end of image）为FF D9。在SOI后，老式相机采用JFIF格式，即以FF E0开始，头部含有 .. JFIF...信息。现在Exif更加流行，以FF E1开始。

![934d404e481dbb784d2481efb825f3aa.png](/_resources/934d404e481dbb784d2481efb825f3aa.png)

- SOI（文件头）+APP0（图像识别信息）+ DQT（定义量化表）+ SOF（图像基本信息）+ DHT（定义Huffman表） + DRI（定义重新开始间隔）（非必须）+ SOS（扫描行开始）+ EOI（文件尾）。
- JPEG 文件的格式是分为一个一个的段来存储的，段的多少和长度并不是一定的。JPEG文件的每个段都一定包含两部分，一个是段的标识，它由两个字节构成：第一个字节是十六进制0xFF，第二个字节对于不同的段，这个值是不同的。紧接着的两个字节存放的是这个段的长度（除了前面的两个字节0xFF和0xXX，X表示不确定。他们是不算到段的长度中的）。注意：这个长度的表示方法是按照高位在前，低位在后的。段标识（1Byte）+段类型（1Byte）+段长度（2Byte）+段内容（…）。段标识已经固定为：0xFF。段类型就有很多种。例如SOI 开头两个字节：0xFFD8。例如EOI 结尾两个字节：0xFFD9。段类型：

![5b90ea104a2b44b23ef24760eba0bf19.png](/_resources/5b90ea104a2b44b23ef24760eba0bf19.png)

- 串口输出的JPEG的头为FF D9，跟FF E1（OV2640应当是FF E0 00 10），在数据的最后跟大量的01（一般应当是大量的00），也就是说除了段标识仍为FF，段类型均右移了1位。这种情况就有可能是某条数据线D#断路。1111 1111 /1101 1001 /FF D9错误，1111 1111 /1101 1000 /FF D8正确，因此问题应当是：数据线D0一直为高（1），因此段标识仍为FF，其他数据均右移1位，因此判断数据线D0断线，万用表测出电压几乎为0。换线之后，右移1位被修复，用万用表测，正常的数据线D#的电压应当都是一致且不断变化且大致上与供电电压大概一半的电压的上下浮动。
- - 串口输出的JPEG的头为FB D8，跟FB E0，尾为FB D9，FB（1111 1011）错误，因此问题应当是：数据线D2一直为低（0），但不是完全断路，万用表测出电压不足供电电压大小。换线之后，问题被修复，电压恢复，是与其他数据引脚一样的不断变化的较低电压。
- 根据以上问题也可知，DCMI的硬件接线的问题不会影响到MCU与OV2640的SCCB（I2C）的匹配。
- 摄像头传回图像模糊，注意OV2460只能手动进行调焦，在摄像头镜头处旋转镜头以进行手动调焦。
- 串口显示摄像头传回的图像数据的帧数低，应该不是杜邦线问题（带宽至少有5MB/s）。串口波特率也会影响串口传输的速率，115200波特率大概每秒传输字节数为115200/8=14400字节即14.4KB/s。当然，波特率可以设得越高越好，但是在CubeMX设置串口速率时会提示当前STM32芯片型号支持的适合的特定倍数的波特率的确切值，上位机的串口的波特率一般也不能自定义或自定义得太大。发现即使设置921600的波特率，RX速率仍为70KB左右，大概为4-5帧，存在瓶颈。B站一个STM32F4的OV2640串口展示也不超过3帧率，TFT屏幕可以轻易实现更高帧数，应该就是串口带宽问题。B站正点原子，基于正点自家的F103开发板，自定义1500000波特率，更换不同的摄像头，串口助手的RX speed，一个能跑120KB/s左右，一个只能跑75KB/s左右，效果上都不超过5帧，前面那个是OV5640，后面那个就是OV2640，与我的效果几乎一致。我的无法突破90KB/s应该就是因为采用OV2640，并采用串口传输方法的上限速率。正点原子F103的LCD（注意是显示在LCD上）例程测试与此帧率差不多，但在F407上LCD可以全速运行，因此推测串口视频显示大概瓶颈就在于串口。瓶颈也可能是：程序问题，STM32硬件内部处理速度，STM32串口芯片发送速率限制，或者是OV2640的寄存器设置问题（例如可以对0XD3PCLK分频系数，越小PCLK越大，0X11时钟分频系数寄存器，越小时钟频率越大，0X3D，越小时钟频率越大）。如果直接将图像传输到其他外设而不是通过串口发送给上位机，或者直接在STM32上处理而不是传输到其他设备，帧数较低应该没有问题。帧率的计算需要用到定时器通过频率进行计算。

[OV2640帧率的计算-CSDN博客](:/50496349d2c44233af9d6dbe40439716)

[OV2640时钟频率和帧率的计算-第2页回答 - 平头弟](:/adeadeaf6d4c47db936d7b87048c2cc4)

- 因此串口传输速率有上限不是x-cube-so包的对printf函数的重定向问题，也不是上位机电脑的串口接收问题，取决于摄像头的参数。
- 没有导入X-CUBE-AI包，且使用微库，但printf仍使得程序卡住的问题，待解决。
- keil编译完成时会显示Program Size: Code= RO-data= RW-data= ZI-data=。
- 使用记事本打开工程编译完成得到的.map文件。Code为程序代码部分，RO-data表示程序定义的常量const temp，RW-data 表示已初始化的全局变量，ZI-data 表示 未初始化的全局变量。
- Code，RO-data，RW-data占用flash（ROM）。ZIdata占用SRAM（RAM）。
- 当keil因为ROM或RAM不足而编译不通过时，可以对每个已知会占用很大空间的代码进行分别的编译，查看编译成功后keil显示的各部分的占比。例如X-Cube_AI的MX_X_CUBE_AI_Process()语句占用很大的RO-data和很大的ZI-data，例如使用DCMI驱动摄像头时通过DMA和串口调用一个很大的未初始化的用于缓存接受到的图像数据的变量占用很大的ZI-data，发现确实是ZI-data太大的问题，因此可以适当调整用于缓存接收到的图像数据的变量。注意即使定义了一个变量，或者已经include了相应的库，如果没有实际调用就不会计入占用，推测是编译时自动将根本没有被调用的代码和被定义的变量忽略了，不占用RAM或ROM。
- 使用STM32CubeMX生成的keil工程会根据芯片型号配置好RAM和ROM的大小。
- 经过测试，用于接收图像数据的缓存变量的大小对摄像头串口输出效果的影响在缩小默认的大概一半就开始花屏。
- 不使用JPEG压缩的情况下，每个像素2B也就是一个16位数，可能接收图像数据的缓存变量的要求会更高。
- 将摄像头传输的数据从串口打印并使用XCAM查看图片时，如果同时打印了其他信息（例如测试信息），图片会很容易撕裂模糊。
- 还有能够跑YOLO模型的图片格式也应该不是JPEG，空间和处理方法都需要考虑。
- app_x-cube-ai库里面也包含了ai_platform库和以导入的模型名作为名字的库，和旧版的其实是很相似的。库和函数需要加上模型名，是因为X-CUBE-AI包可以导入多个模型，调用的函数需要加以区别，这个无论新旧版本都符合。
- ST-LINK Utility导入hex文件的路径必须不含中文，全英文路径名也不能太长。
- X-Cube-AI官方资料路径：`C:\Users\xxxxx\STM32Cube\Repository\Packs\STMicroelectronics\X-CUBE-AI\8.1.0\Documentation\index.html` 会有各种html文件的索引。Embedded Inference Client API即为怎么使用库的API的最新方法（一手资料）。目前X-Cube-AI每个版本之间的差别还是很大，必须通过查看一手资料进行新API的学习。
- MX_X_CUBE_AI_Process()编译成功会在串口输出模型验证成功的信息，最重要的就是输入和输出的size（例子：n_inputs/n_outputs : 1/1 I[0] (1,90,3,1)270/float32 @0x20015000/1080 O[0] (1,1,1,6)6/float32 @0x20015018/24）。while(1)代码块中同时跑其他程序和MX_X_CUBE_AI_Process()，之后一直在MX_X_CUBE_AI_Process()卡住，推测是这个函数中含有等待输入的程序一直循环。而且，不编译MX_X_CUBE_AI_Process()也是可以定义模型和输入输出的，因此每次重新生成工程都需要将其注释掉，它就是个占用大又只有验证作用的语句。此外，官方例程的HAR模型的输入维度是1，不需要定义多维的输入。
- OV2640摄像头再次故障，这次数据最后跟的大量00变成了20（00100000），判断为D5一直高电平，可能是不稳或者断线，把D5换线，仍然不行，把其他线逐个拔去来观察串口输出的图像数据的变化，发现大多数线拔掉也能继续工作，除了3.3V（VCC）和SCCB相关的线（已经初始化完了）和时钟相关的线之外，拔掉都基本会对串口输出的图像数据产生不同的影响（D2拔掉几乎没有影响，换线后电压没问题，但是问题没解决），GND则是不接也行。怀疑不止一个引脚发生故障，而且不一定是线的问题，可能是摄像头的特定引脚的故障，或者摄像头本身的故障。万用表测得的电压和正常时基本差别不大。最后发现原来是波特率已经从1382400调为921600以方便调试，但是因为波特率相近因此输出数据仍很类似于JPEG格式（不如说波特率相近数据就会呈现一定的规律性而不是乱码，但仍与原信息有本质差别），串口助手调整接收波特率或cubemx重新配置输出波特率即可。
- 通过SCCB调整OV2640输出为RGB565模式，其他可以继续沿用JPEG模式的设置。使用串口助手保存窗口到txt文件，然后上位机使用Python将RGB565转化为RGB888格式进行PNG显示，有几率能够显示正常图像。while(1)其他语句注释，或者即使没有注释偶尔也会有几率花屏无法显示正常图像（其他语句明明会printf其他调试信息），而即使没有注释，图像会出现一定的乱码（其他printf语句造成）的同时，还会有花纹，或者画面割裂（看起来像是两张图片各一半合并），无法消除。这和在串口助手窗口捕捉到的数据也有关系，首先不能包含太多例如初始化函数输出的调试信息，需要在摄像头稳定运行一段时间后，通过观察串口输出数据的间隔，取两帧（上位机解码只会解码前一帧长度的像素）完整进行保存。还有，试着把摄像头拿起并且上下对正要拍摄的景物，效果会更好。花屏的原因好像是丢失字节。
- 串口和st-link utility的连接是可以同时适用的。
- 微控制器的RAM是SRAM而不是DRAM，速度更快。而ROM是Flash，因为为了能够重复烧录并且断电保存。
- X-CUBE-AI包，![87fc4d7bead66dd16563be73da60d6a6.png](/_resources/87fc4d7bead66dd16563be73da60d6a6.png)处能够设置External RAM和External Flash。注意X-CUBE-AI涉及的变量在定义时处于外部内存的地址需要在代码中手动进行设置，STM32CubeMX和X-CUBE-AI只是能够让你能够成功生成代码（不然X-CUBE-AI认为Flash或RAM不够是无法生成代码的，因为它不知道是否有外部内存），不会自动把涉及的变量定义在外部内存里。
- 把activation buffer变量和存放输入数据的变量都存放在外部内存时，X-CUBE-AI相关的程序运行速度大大降低了。
- OV2640，jpeg模式图片没问题，rgb565模式颜色不对（彩色花屏）、黑屏噪点（黑屏也有颜色点）、干扰条纹（照片中有一些条纹波纹）、且有部分画面不显示（某一边的画面看起来像是另一边的上面部分）。彩色花屏和黑屏噪点应该是字节错位了，因为两个字节一个RGB565像素，如果字节在开头多了一个或少了一个，都会造成全部像素的颜色完全偏离，比较简单的解决方法就是如果发现彩色花屏则把rgb565模式接收到的数据的第一个字节省略掉即可。部分画面不显示和部分画面重复主要画面的一部分的情况，怀疑可能是摄像头窗口设置长宽颠倒或者摄像头rgb模式只能输出正方形造成的，但之后发现是显示相关代码的设置造成的，因为DCMI采集到的数据是按一行一行来还是一列一列来是未知的，所以解码时高和宽的设置就会先入为主。如果将宽度设置成高度的值来解码，那么部分画面就不显示，因为没有读取剩余的宽度的像素值，如果将宽度设置成比高度更大的值来解码，那么部分画面就会重复主要画面的一部分，因为多出来的宽对应的数据是下一行的开头的像素值，怀疑DCMI采集160 * 120但多出来的8个字（32个字节）是为了填补这部分空缺的。如果将高度设置成宽度的值来解码，那么就会部分画面就黑屏，因为多出来的高对应的数据是不存在的。
- X-CUBE-AI包，设置External Flash，External RAM页面可以将权重复制到External RAM中，以减少将权重放置到External Flash的延迟。
- 摄像头模块设置为RGB565模式输出时，在120 * 160像素下，前32个字节似乎是无效数据（DMA应当捕获120 * 160 /4 = 9600个数据，因为DMA操作以字为单位，STM32每个字当然是32bit，也就是4字节），但是DMA捕获到的数据长度为9608，意味着有8个字多出来，经过LCD屏的显示效果，得知是前8个字是无效数据，或者是表示其他信息数据（反正不是像素信息）。
- STM32的SPI接口的波特率，与TFT-LCD的刷屏速度有关，这是因为SPI接口驱动的TFT-LCD只由SPI来传输数据，一定范围内，波特率越高自然延迟越低。
- RGB565：R和B都经过了右移运算，舍弃了低三位的信息（最大为11111000），G也是舍弃了低二位的信息（最大为11111100），所以显示出的颜色能大致符合，稍有色差。pc端必须为 rgb565 或 yuy2等标准格式，pc端再把rgb565 或 yuy2等格式转换为raw（gray）。 将原来的RGB565中的RGB565(R,G,B)统一用Gray替换，形成新的颜色RGB565(Gray,Gray,Gray)，用它替换原来的RGB565(R,G,B)就是灰度图了。
- X-CUBE-AI包在取多输出结果数组的值时，有时需要向前取值（定义一个指针为数组变量，然后将其地址减去偏移量并取值）。这是因为，不同的操作（例如printf语句的数量等）都有可能会影响指针的值。问题最终解决，是因为向X-CUBE-AI传参前的问题，`ai_output[0].data = AI_HANDLE_PTR(out_data_1);ai_output[1].data = AI_HANDLE_PTR(out_data_2);`而不能是`ai_float *output_data[]={&out_data_1[0]};ai_output[i].data = AI_HANDLE_PTR(out_data[i]);`。这可能是程序编译和运行时，未加上换行符的指针数组`out_data`的编译不能按预想地进行编译（可能视为类似结构体地进行编译，自动加上了额外的地址偏移量），或者在指针数组取值时`out_data[i]`的值会发生变化（现象为：`out_data[i]`的值可能发生前后偏移几位，且是随机的），而直接传参指针`&out_data_1[0]`和`out_data_1`的值则不会变化。
- 预训练模型，迁移学习，可以保持参数量不变输入维度变化，但占用内存变化。
- 对rgb565的相关学习，认识到了不能太过相信直觉，必须多方面考证和验证以确认一种方法或协议的具体内容，还要小心错误混淆的二次内容，和因自己相关知识还不够丰富导致对正确内容的错误理解。
- 很多时候，代码错误的未发现，是由于测试不够全面或者只测试了最容易的特例而导致的。
- 外派Arduino项目获得：舵机可以外部供电，但是必须和单片机共地。否则会出现和供电不稳类似的现象，抽搐，乱发信号。

## TinyML-模型压缩方法

模型压缩方法分为三类：

- 剪枝（pruning）
- 量化（quantization）
- 知识蒸馏（knowledge distillation）

### 剪枝

- 去除模型中不重要的冗余参数，以获得稀疏/紧凑的模型结构。
- 一般依赖于二阶泰勒展开来估计修剪的参数的重要性。在这些方法中海森（Hessian）矩阵应该被部分或完全计算。也可以使用其他标准来识别参数的重要性，参数的重要性也被称为显著性。
- 理论上，最佳标准是对每个参数对网络的影响进行精确评估，但是这种评估计算成本过高。因此，可以利用其他评估方法，包括ln -范数、特征图激活（feature map activation）的均值或标准差、批量归一化缩放因子（batch normalization scaling factor，BNSF）、一阶导数、互信息（mutual information），来进行显著性分析。
- 修剪的粒度（granularity of pruning）决定了要修剪模型的哪些参数，广义上讲，修剪可以以结构化或非结构化的方式进行。
- 非结构化剪枝。
- 结构化剪枝。
- - 基于通道的修剪。
- - 基于滤波器的修剪。
- - 基于核的剪枝。

### 量化

- 涉及使用较低精度的数据类型表示模型的激活值和权重。
- 通常情况下，原始精度是32位半精度浮点型。
- 量化区间：均匀、非均匀。均匀量化将值映射为一组均匀间隔的离散值，而非均匀量化的离散值之间的距离不一定相等。
- 量化方案：量化感知训练（QAT）、训练后量化（PTQ）。将训练好的模型量化可能会由于累积的数值误差而对模型的准确性产生负面影响。为了恢复性能下降，通常需要调整网络参数。在QAT中，量化模型的前向和后向传递是以浮点数进行，并且在每次梯度更新后对网络参数进行量化。而在PTQ中，则是在不重新训练网络的情况下进行量化和参数调整。
- 量化部署方案/推理（inference）方案：伪量化/模拟量化、仅整数量化/定点量化。推理就是将训练好的模型进行应用，根据输入得到输出的过程。在伪量化/模拟量化方法中，权重和激活值以低精度存储，但是从加法到矩阵乘法的所有操作都以浮点精度执行，尽管这种方法在浮点操作之前或之后需要不断地解量化和量化，但它有利于模型的准确性。然而，在仅整数量化/定点量化方法中，操作和权重/激活值存储都是用低精度整数算术执行，这样以来，模型可以利用大多数硬件支持的快速整数算术。

![c3a53f2230c40d83af036d051e52b1c2.png](/_resources/c3a53f2230c40d83af036d051e52b1c2.png)

### 知识蒸馏

- 将一个大而精确的模型作为教师，使用教师模型提供的软标签来训练一个小模型。

### 聚类

- 聚类（clustering）的工作原理是将模型中每一层的权重分组为预定义数量的聚类，然后共享属于每个单独聚类的权重的质心值。这减少了模型中唯一权重值的数量，从而降低了其复杂性。