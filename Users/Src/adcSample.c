
#include <math.h>
#include "adcSample.h"
#include "ioctrl.h"
#include "tim4tick.h"

/* NTC温度传感器温度-阻值对应关系表. */
static const ntcSensorParaDef_t ntcTable[166] = 
{
  {-40, 1889.0}, {-39, 1757.0}, {-38, 1635.0},{-37, 1523.0},{-36, 1419.0},{-35, 1323.0},{-34, 1235.0},{-33, 1153.0},{-32, 1077.0},{-31, 1006.0},
  {-30, 941.0},  {-29, 880.4},  {-28, 824.2}, {-27, 771.9}, {-26, 723.4}, {-25, 678.3}, {-24, 636.3}, {-23, 597.2}, {-22, 560.8}, {-21, 526.9},
  {-20, 495.3},  {-19, 465.8},  {-18, 438.3}, {-17, 412.6}, {-16, 388.6}, {-15, 366.1}, {-14, 345.1}, {-13, 325.5}, {-12, 307.1}, {-11, 289.9},
  {-10, 273.8},  {-9, 258.6},   {-8, 244.4},  {-7, 231.1},  {-6, 218.7},  {-5, 206.9},  {-4, 195.9},  {-3, 185.5},  {-2, 175.8},  {-1, 166.6},
  {0, 158.0},    {1, 149.9},    {2, 142.3},   {3, 135.0},   {4, 128.3},   {5, 121.8},   {6, 115.8},   {7, 110.1},   {8, 104.7},   {9, 99.63},
  {10, 94.82},   {11, 90.28},   {12, 85.98},  {13, 81.92},  {14, 78.07},  {15, 74.43},  {16, 70.99},  {17, 67.72},  {18, 64.63},  {19, 61.70},
  {20, 58.92},   {21, 56.28},   {22, 53.77},  {23, 51.40},  {24, 49.14},  {25, 47.00},  {26, 44.96},  {27, 43.03},  {28, 41.19},  {29, 39.44},
  {30, 37.77},   {31, 36.19},   {32, 34.68},  {33, 33.25},  {34, 31.88},  {35, 30.58},  {36, 29.33},  {37, 28.15},  {38, 27.02},  {39, 25.94},
  {40, 24.92},   {41, 23.94},   {42, 23.00},  {43, 22.11},  {44, 21.25},  {45, 20.44},  {46, 19.66},  {47, 18.91},  {48, 18.20},  {49, 17.52},
  {50, 16.86},   {51, 16.24},   {52, 15.64},  {53, 15.07},  {54, 14.52},  {55, 14.00},  {56, 13.50},  {57, 13.01},  {58, 12.55},  {59, 12.11},
  {60, 11.68},   {61, 11.28},   {62, 10.89},  {63, 10.51},  {64, 10.15},  {65, 9.806},  {66, 9.474},  {67, 9.155},  {68, 8.849},  {69, 8.554},
  {70, 8.271},   {71, 7.999},   {72, 7.738},  {73, 7.486},  {74, 7.244},  {75, 7.011},  {76, 6.787},  {77, 6.571},  {78, 6.363},  {79, 6.163},
  {80, 5.971},   {81, 5.785},   {82, 5.606},  {83, 5.434},  {84, 5.268},  {85, 5.107},  {86, 4.953},  {87, 4.804},  {88, 4.660},  {89, 4.522},
  {90, 4.388},   {91, 4.259},   {92, 4.134},  {93, 4.014},  {94, 3.897},  {95, 3.785},  {96, 3.677},  {97, 3.572},  {98, 3.471},  {99, 3.373},
  {100, 3.278},  {101, 3.187},  {102, 3.098}, {103, 3.013}, {104, 2.930}, {105, 2.850}, {106, 2.773}, {107, 2.698}, {108, 2.625}, {109, 2.555},
  {110, 2.487},  {111, 2.421},  {112, 2.357}, {113, 2.296}, {114, 2.236}, {115, 2.178}, {116, 2.121}, {117, 2.067}, {118, 2.014}, {119, 1.963},
  {120, 1.913},  {121, 1.865},  {122, 1.818}, {123, 1.773}, {124, 1.729}, {125, 1.687},
};


static volatile uint16_t adcSampleRawDataBuf[ADC_SAMPLE_RAWDATABUF_SIZE] = {0,0,0,0,0,0,0,0,0,0};
static volatile uint8_t adcDataBufIndex = 0;
static volatile bool sampleFinishMutex = FALSE;		/* 当前通道采集完成互斥量. */
static volatile bool convertCompleteMutex = TRUE;	/* 当前通道转换完成互斥量. */

static VoltParaDef_t boostPara = 
{
    .inputSta   = UnderVoltage,
    .inputVolt  = 0.0,
    .outputSta  = UnderVoltage,
    .outputVolt = 0.0,
};

/* 系统当前温度值. */
static int8_t systemTemperature = 0;

/* 采样对象标识器. 0,采集输入电压信号; 1,采集输出电压信号; 2,采集温度信号; -1,无效; */
static int8_t sampleObjectMarker = 0;     
                                 
/* 定时器扫描采集对象索引标识计数器. */
static int8_t timerScanIndex = -1;




/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void adcTempChannelInit_LL(void)
{
    GPIO_Init(GPIOF, GPIO_PIN_4, GPIO_MODE_IN_FL_NO_IT);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, ENABLE);
    
    ADC1_DeInit();
    ADC1_Init(ADC1_CONVERSIONMODE_SINGLE,
              ADC1_CHANNEL_12,
              ADC1_PRESSEL_FCPU_D8,
              ADC1_EXTTRIG_TIM,
              DISABLE, 
              ADC1_ALIGN_RIGHT,
              ADC1_SCHMITTTRIG_CHANNEL12,
              DISABLE
              );
    
    ADC1_ITConfig(ADC1_IT_EOCIE, ENABLE);                           /* 使能ADC转换完成中断. */
    ADC1_Cmd(ENABLE);
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void adcBoostInputVoltChannelInit_LL(void)
{
    GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_IN_FL_NO_IT);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, ENABLE);
    
    ADC1_DeInit();
    ADC1_Init(ADC1_CONVERSIONMODE_SINGLE,
              ADC1_CHANNEL_4,
              ADC1_PRESSEL_FCPU_D8,
              ADC1_EXTTRIG_TIM,
              DISABLE, 
              ADC1_ALIGN_RIGHT,
              ADC1_SCHMITTTRIG_CHANNEL12,
              DISABLE
              );   
    
    ADC1_ITConfig(ADC1_IT_EOCIE, ENABLE);                           /* 使能ADC转换完成中断. */
    ADC1_Cmd(ENABLE);
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void adcBoostOutputVoltChannelInit_LL(void)
{
    GPIO_Init(GPIOB, GPIO_PIN_1, GPIO_MODE_IN_FL_NO_IT);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, ENABLE);
    
    ADC1_DeInit();
    ADC1_Init(ADC1_CONVERSIONMODE_SINGLE,
              ADC1_CHANNEL_1,
              ADC1_PRESSEL_FCPU_D8,
              ADC1_EXTTRIG_TIM,
              DISABLE, 
              ADC1_ALIGN_RIGHT,
              ADC1_SCHMITTTRIG_CHANNEL12,
              DISABLE
              );   
    
    ADC1_ITConfig(ADC1_IT_EOCIE, ENABLE);                           /* 使能ADC转换完成中断. */
    ADC1_Cmd(ENABLE);   
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void adcSampleRawdataBuff_Write(uint16_t data)
{
    if (sampleFinishMutex == FALSE) 
    {
        adcSampleRawDataBuf[adcDataBufIndex++] = data;
        
        if (adcDataBufIndex >= ADC_SAMPLE_RAWDATABUF_SIZE) 
        {
            adcDataBufIndex = 0;
            sampleFinishMutex = TRUE;                           	/* POST采集结束互斥信号量. */
        }
    }
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void configADConvertCompleteMutexFlag(bool val)
{
    convertCompleteMutex = val;
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void adcSampleConvertScan(void)
{
    if ((sampleObjectMarker >= 0) && (sampleObjectMarker <= 2))
    {
        if ((convertCompleteMutex == TRUE) && (sampleFinishMutex == FALSE))
        {
            ADC1_StartConversion();
            convertCompleteMutex = FALSE;
        }
    }
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
uint16_t adcMovingFilter(uint16_t* pRawData, uint8_t len)
{
    uint16_t maxVal, minVal, retVal;
    uint8_t i;
    uint32_t sumVal;
    
    maxVal = *pRawData;
    minVal = *pRawData;
    
    for (i = 1; i < len; i++) 
	{
        if (pRawData[i] > maxVal)maxVal = pRawData[i];
        if (pRawData[i] < minVal)minVal = pRawData[i];
    }
    
    sumVal = 0;
    for (i = 0; i < len; i++) 
	{
        sumVal += *pRawData;
        pRawData++;
    }
    
    sumVal -= maxVal;
    sumVal -= minVal;
    
    retVal = (uint16_t)(sumVal / (len - 2));
    
    return (retVal);
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
static float calculateBoostInputVoltage(uint16_t adcData)
{
    float Vsp, Vout;
    float adcRawdata = (float)adcData;
    
    Vsp = (adcRawdata / 1024.0) * 3.0;                      /* Vsp = (ADC Rawdata / 2^10) * Vref */
    
    Vout = Vsp * 194.548;                                   /* Vout = Vsp * k */
    
    return (Vout);
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
static float calculateBoostOutputVoltage(uint16_t adcData)
{
    float Vsp, Vout;
    float adcRawdata = (float)adcData;
    
    Vsp = (adcRawdata / 1024.0) * 3.0;                      /* Vsp = (ADC Rawdata / 2^10) * Vref */
    
    Vout = Vsp * 312.11;                                   /* Vout = Vsp * k */
    
    return (Vout);
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
static int8_t calculateSystemTemperature(uint16_t adcData)
{
    float Vsp, Rx;
    int8_t temp = 0;
    float adcRawdata = (float)adcData;
    
    Vsp = (adcRawdata / 1024.0) * 3.0;
    Rx = (Vsp * 20.0) / (3.0 - Vsp);
    
    if ((Rx > 1889.0) && (fabs(Rx - 1889.0) > 1e-6))
    {
        temp = -40;
        return (temp);
    }
    else if ((Rx < 1.687) && (fabs(Rx - 1.687) > 1e-6))
    {
        temp = 125;
        return (temp);
    } 
    else 
    {
        for (uint8_t i = 0; i < (sizeof(ntcTable) / sizeof(ntcTable[0])); i++)// Rx < ntcTable[i].resistance
        {
			/* Rx <= ntcTable[i].resistance && Rx > ntcTable[i+1].resistance  */
            if ((((Rx < ntcTable[i].resistance) && (fabs(Rx - ntcTable[i].resistance) > 1e-6)) || (fabs(Rx - ntcTable[i].resistance) < 1e-6)) && \
                ((Rx > ntcTable[i+1].resistance) && (fabs(Rx - ntcTable[i+1].resistance) > 1e-6)))         
            {
                temp = ntcTable[i].tempResult;
                break;
            }
        }
        
        return (temp);
    }
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
static void boostInputVoltageCompare(float volt)
{
    const float EPSINON = 1e-6;
    
    if ((volt < 200.0) && (fabs(volt - 200.0) > EPSINON))               /* 输入电压小于200V. */
    {
        boostPara.inputSta = UnderVoltage;                              /* Boost输入电压欠压. */
    }
    else if (((volt > 200.0) && (fabs(volt - 200.0) > EPSINON)) && \
             ((volt < 250.0) && (fabs(volt - 250.0) > EPSINON)))        /* 输入电压大于200V小于250V. */
    {
        boostPara.inputSta = ReductionPower;                            /* Boost输入电压降额,降功率输出. */
        ioCtrlRelayOpen_LL();
    }
    else if (((volt > 250.0) && (fabs(volt - 250.0) > EPSINON)) && \
             ((volt < 500.0) && (fabs(volt - 500.0) > EPSINON)))        /* 输入电压大于250V小于500V. */
    {
        boostPara.inputSta = FullPower;                                 /* Boost输入电压正常,满功率输出. */
        ioCtrlRelayOpen_LL();
    }
    else if ((volt > 500.0) && (fabs(volt - 500.0) > EPSINON))          /* 输入电压大于500V. */
    {
        boostPara.inputSta = OverVoltage;                               /* Boost输入电压过压. */
    }
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
static void boostOutputVoltageCompare(float volt)
{
	const float EPSINON = 1e-6;
	
	if ((volt < 470.0) && (fabs(volt - 470.0) > EPSINON))
	{
		boostPara.outputSta = UnderVoltage;
	}
	else if (((volt > 470.0) && (fabs(volt - 470.0) > EPSINON)) && \
			((volt < 530.0) && (fabs(volt - 530.0) > EPSINON)))
	{
		boostPara.outputSta = FullPower;
	}
	else if ((volt > 530.0) && (fabs(volt - 530.0) > EPSINON))
	{
		boostPara.outputSta = OverVoltage;
	}
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void adcSampleTriggerScan(void)
{
    const uint16_t PERIOD = 100;
    static uint16_t scanCnt = 0;
    
    if (timerScanIndex != -1)
    {
        scanCnt++;
        if (scanCnt >= PERIOD)                  /* 5ms * 100 = 500ms. */
        {
            scanCnt = 0;
            
            sampleObjectMarker = timerScanIndex;
            timerScanIndex = -1;
        }
    }
}

/*
 * @函数功能：获取ADC采集结果数据
 * @函数参数：无
 * @返回值：无
 */
void adcSampleGetResult(void)
{
    uint16_t result;
    
    if (sampleFinishMutex == TRUE)                                              /* Polling采集结束互斥信号量.若Poll到了,则处理ADC采集的原始数据. */                      
    {
        result = adcMovingFilter((uint16_t*)adcSampleRawDataBuf,
                                  ADC_SAMPLE_RAWDATABUF_SIZE);                  /* 滑动滤波处理. */
        
        if (sampleObjectMarker == 0)                                            /* 当前ADC是采集Boost输入电压信号. */          
        {
            boostPara.inputVolt = calculateBoostInputVoltage(result);
            boostInputVoltageCompare(boostPara.inputVolt);

            adcBoostOutputVoltChannelInit_LL();
            timerScanIndex = 1;
            
            sampleObjectMarker = -1;                                            /* 采集对象标识器置为无效值. */    
        }
        else if (sampleObjectMarker == 1)                                       /* 当前ADC是采集Boost输出电压信号. */
        {
            boostPara.outputVolt = calculateBoostOutputVoltage(result);
            boostOutputVoltageCompare(boostPara.outputVolt);
            
            adcTempChannelInit_LL();
            timerScanIndex = 2;
            
            sampleObjectMarker = -1;                                            /* 采集对象标识器置为无效值. */ 
        }
        else if (sampleObjectMarker == 2)                                       /* 当前ADC是采集温度信号. */
        {
            systemTemperature = calculateSystemTemperature(result);
            
            adcBoostInputVoltChannelInit_LL();
            timerScanIndex = 0;
            
            sampleObjectMarker = -1;                                            /* 采集对象标识器置为无效值. */ 
        }
        
        sampleFinishMutex = FALSE;                                              /* 释放采集结束互斥信号量. */
    }
}

