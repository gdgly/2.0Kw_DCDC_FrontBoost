
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
    FullPower = 0,                              /* 电压满额. */
    ReductionPower = 1,                         /* 电压降额. */
    UnderVoltage = 2,                           /* 电压欠压. */
    OverVoltage = 3,                            /* 电压过压. */
}VoltStatusDef_t;

typedef enum
{
	Normal = 0,
	HighTempAlarm = 1,
	LowTempAlarm = 2,
}TempStatusDef_t;

typedef struct
{
    VoltStatusDef_t inputSta;              		/* 输入电压状态 	. */
    float inputVolt;                            /* 输入电压. 		 */
    
    VoltStatusDef_t outputSta;             		/* 输出电压状态 	  . */
    float outputVolt;                           /* 输出电压. 		 */

	TempStatusDef_t tempSta;					/* 温度状态.     */
	int8_t tempVal;								/* 温度值. */
}SystemInfoParaDef_t;

typedef struct
{
    int8_t tempResult;							/* NTC温度传感器当前的温度值. */
    float  resistance;							/* NTC温度传感器当前的电阻值. */
}ntcSensorParaDef_t;

typedef struct
{
	int8_t val;									/* 系统当前温度值. */
}TemperatureParaDef_t;

   
   
void adcTempChannelInit_LL(void);
void adcBoostInputVoltChannelInit_LL(void);
void adcBoostOutputVoltChannelInit_LL(void);

SystemInfoParaDef_t* getSystemInfoParaPtr(void);

bool getSystemInfoReadyFlag(void);
void configSystemInfoReadyFlag(bool wdata);
void configADConvertCompleteMutexFlag(bool val);

void adcSampleInputVolt_Init(void);
void adcSampleOutputVolt_Init(void);
void adcSampleSystemTemperature_Init(void);

/* 下面三个函数是被中断ISR函数调用. */
void adcSampleTriggerScan(void);
void adcSampleConvertScan(void);
void adcSampleRawdataBuff_Write(uint16_t data);

void adcSampleGetResult(void);


#ifdef __cplusplus
}
#endif

#endif

