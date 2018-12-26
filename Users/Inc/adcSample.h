
#ifndef  __ADCSAMPLE_H
#define  __ADCSAMPLE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
   
#define  ADC_SAMPLE_RAWDATABUF_SIZE     (10)
   
   
typedef enum
{
    FullPower = 0,                              /* Boost电压满额. */
    ReductionPower = 1,                         /* Boost电压降额. */
    UnderVoltage = 2,                           /* Boost电压欠压. */
    OverVoltage = 3,                            /* Boost电压过压. */
}VoltStatusDef_t;

typedef struct
{
	bool inputUpdateFlag;							/* Boost输入电压更新    */
    VoltStatusDef_t inputSta;              		/* Boost输入电压状态 	*/
    float inputVolt;                            /* Boost输入电压 		*/
    
	bool outputUpdateFlag;							/* Boost输出电压更新    */
    VoltStatusDef_t outputSta;             		/* Boost输出电压状态 	*/
    float outputVolt;                           /* Boost输出电压 		*/
}VoltParaDef_t;

typedef struct
{
    int8_t tempResult;							/* NTC温度传感器当前的温度值. */
    float  resistance;							/* NTC温度传感器当前的电阻值. */
}ntcSensorParaDef_t;

typedef struct
{
	int8_t val;
	bool valueUpdateFlag;
}TemperatureParaDef_t;

   
   
void adcTempChannelInit_LL(void);
void adcBoostInputVoltChannelInit_LL(void);
void adcBoostOutputVoltChannelInit_LL(void);

VoltParaDef_t* getSystemVoltageParaPtr(void);
void configSystemInputVoltParaUpdateFlag(bool wdata);
void configSystemOutputVoltParaUpdateFlag(bool wdata);
TemperatureParaDef_t* getSystemTemperatureParaPtr(void);
void configSystemTemperatureParaUpdateFlag(bool wdata);
bool getSystemInfoReadyFlag(void);
void configSystemInfoReadyFlag(bool wdata);


void adcSampleRawdataBuff_Write(uint16_t data);
void configADConvertCompleteMutexFlag(bool val);
void adcSampleConvertScan(void);
uint16_t adcMovingFilter(uint16_t* pRawData, uint8_t len);

void adcSampleTriggerScan(void);
void adcSampleGetResult(void);

#ifdef __cplusplus
}
#endif

#endif

