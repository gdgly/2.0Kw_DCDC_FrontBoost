
#include "adcTemp.h"

static volatile uint16_t adcSampleRawDataBuf[ADC_SAMPLE_RAWDATABUF_SIZE] = {0,0,0,0,0,0,0,0,0,0};
static volatile uint8_t adcDataBufIndex = 0;
static volatile bool adcSampleFinishEventFlag = FALSE;
static volatile bool adcConvertCompleteEventFlag = TRUE;


static float boostInVolt;

/* 采样对象计数器信号量. 0,采集Boost输入电压信号; 1,采集Boost输出电压信号; 2,采集温度信号; */
static int8_t sampleObjectCntSemphore = 0;                                     


/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void adcSampleRawdataBuf_Write(uint16_t data)
{
    if (adcSampleFinishEventFlag == FALSE) {
        adcSampleRawDataBuf[adcDataBufIndex++] = data;
        if (adcDataBufIndex >= ADC_SAMPLE_RAWDATABUF_SIZE) {
            adcDataBufIndex = 0;
            adcSampleFinishEventFlag = TRUE;
            adcConvertCompleteEventFlag = FALSE;
        }
    }
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void configAdcConvertCompleteEventFlag(bool val)
{
    adcConvertCompleteEventFlag = val;
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
bool getAdcConvertCompleteEventFlag(void)
{
    return (adcConvertCompleteEventFlag);
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void adcSampleConvertScan(void)
{
    if ((sampleObjectCntSemphore >= 0) && (sampleObjectCntSemphore <= 2))
    {
        if (adcConvertCompleteEventFlag == TRUE)
        {
            ADC1_StartConversion();
            adcConvertCompleteEventFlag = FALSE;
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
    
    for (i = 1; i < len; i++) {
        if (pRawData[i] > maxVal)maxVal = pRawData[i];
        if (pRawData[i] < minVal)minVal = pRawData[i];
    }
    
    sumVal = 0;
    for (i = 0; i < len; i++) {
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
void calculateBoostInputVoltage(uint16_t adcData)
{
    float Vsp;
    float adcRawdta = (float)adcData;
    
    Vsp = (adcRawdta / 1024.0) * 3.0;
    
    boostInVolt = Vsp * 194.548;
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void adcSampleGetResult(void)
{
    uint16_t result;
    
    if (adcSampleFinishEventFlag == TRUE) 
    {
        adcSampleFinishEventFlag = FALSE;
        result = adcMovingFilter((uint16_t*)adcSampleRawDataBuf, ADC_SAMPLE_RAWDATABUF_SIZE);
        
        /* 当前ADC是采集Boost输入电压信号. */
        if (sampleObjectCntSemphore == 0)               
        {
            calculateBoostInputVoltage(result);
            
            adcConvertCompleteEventFlag = TRUE;
            
//            sampleObjectCntSemphore = -1;
//            adcBoostOutputVoltChannelInit_LL();
        }
        /* 当前ADC是采集Boost输出电压信号. */
        else if (sampleObjectCntSemphore == 1)
        {
            nop();
            /* Add code here to do... */
            
//            sampleObjectCntSemphore = -1;
//            adcTempChannelInit_LL();
        }
        /* 当前ADC是采集温度信号. */
        else if (sampleObjectCntSemphore == 2)
        {
            nop();
            /* Add code here to do... */
            
//            sampleObjectCntSemphore = -1;
//            adcBoostInputVoltChannelInit_LL();
        }
    }
}

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




