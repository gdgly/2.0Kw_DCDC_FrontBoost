
#ifndef  __ADCTEMP_H
#define  __ADCTEMP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
   
#define  ADC_SAMPLE_RAWDATABUF_SIZE     (10)
   
   
void adcTempChannelInit_LL(void);
void adcBoostInputVoltChannelInit_LL(void);
void adcBoostOutputVoltChannelInit_LL(void);


void adcSampleRawdataBuf_Write(uint16_t data);
void configAdcConvertCompleteEventFlag(bool val);
bool getAdcConvertCompleteEventFlag(void);
void adcSampleConvertScan(void);

uint16_t adcMovingFilter(uint16_t* pRawData, uint8_t len);
void adcSampleGetResult(void);

#ifdef __cplusplus
}
#endif

#endif

