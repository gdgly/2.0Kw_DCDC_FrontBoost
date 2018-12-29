
#include <string.h>
#include "comm.h"
#include "usart.h"
#include "escf.h"
#include "Crc16.h"
#include "voltRegulate.h"
#include "adcSample.h"
#include "tim3timeout.h"
#include "systemStatus.h"



static bool systemCurSwitchingState    = FALSE;		/* 系统当前开机状态. FALSE,关机态; TRUE,开机态; */
static bool systemPrevSwitchingState   = FALSE;		/* 系统前一次的开机状态. */
static bool systemSwitchingChangeState = FALSE;		/* 系统开关机改变的状态. FALSE,状态无变化; TRUE,状态有变化; */

static int8_t transmitOrderCntSemaphore = 0;		/* 串口发送命令帧顺序计数信号量. */

static int8_t maxRetryTimes     = 0;				/* 最大重传次数. -1,一直重传. */
static int8_t retryDelayTime    = 0;				/* 每次传输最大等待超时时间,毫秒级. */
static int8_t keepingRetryTimes = 0;				/* 当前已经重传次数计数器. */

static bool alwaysRetryFlag        = FALSE;			/* 一直重传标志位. */
static bool retryAgainFlag         = FALSE;			/* 再次重传标志位. */
static bool waitRespondTimeoutFlag = FALSE;			/* 等待应答数据帧超时标志位. */

static uint8_t commRxDataErrorCnt  = 0;				/* 串口通信接收应答数据帧数据错误计数器. */
static uint8_t commTimeoutErrorCnt = 0;				/* 串口通信等待应答数据帧超时错误计数器. */


/*
 * @函数功能：获取系统当前运行状态值.
 * @函数参数：无
 * @返回值：系统当前运行状态值.
 */
bool getSystemCurrentSwitchingStatus(void)
{
	return (systemCurSwitchingState);
}

/*
 * @函数功能：配置系统当前运行状态值.
 * @函数参数：sta, 待配置的状态值.
 * @返回值：无
 */
void configSystemCurrentSwitchingStatus(bool sta)
{
	systemCurSwitchingState = sta;
}

/*
 * @函数功能：配置系统当前运行状态值.
 * @函数参数：sta, 待配置的状态值.
 * @返回值：无
 */
bool getSystemSwitchingChangeStatus(void)
{
	return (systemSwitchingChangeState);
}

/*
 * @函数功能：配置系统当前运行状态值.
 * @函数参数：sta, 待配置的状态值.
 * @返回值：无
 */
void configSystemSwitchingChangeStatus(bool wdata)
{
	systemSwitchingChangeState = wdata;
}

/*
 * @函数功能：串口通信接收到的数据帧解析.
 * @函数参数：无
 * @返回值：无
 */
void commReceivedFrameParsing(void)
{
    uint8_t  Buf[20], Len;
    uint8_t  esBuf[15], esLen;
    uint8_t  error, retVal, cmd;
    uint16_t crc, regVolt;
    
    Len = usartCommReceiveData(Buf);
    
    if (Len != 0) 
	{
        esLen = (uint8_t)escfDecode(Buf, Len, esBuf, sizeof(esBuf));			/* "转义序列"解码. */
        for (uint8_t i = 0; i < esLen; i++) 
		{
            esBuf[i] = esBuf[i + 1];
        }
        esLen = esLen - 2;
		
		error = 0;
		
		if (esBuf[2] == FRAME_TYPE_SEND)										/* 接收到的数据帧是后级LLC控制部分主动发送的. */
		{
			if (esBuf[0] != esLen)error = 1;									/* 接收到的数据长度不正确. */
			if (dataBufCrcCheck(esBuf, esLen) == 0)error = 1;					/* 接收到的数据CRC校验不正确. */
			
			if (esBuf[1] == REGULATE_VOLT_CMD)									/* 收到的是调节输出电压命令. */
			{
				cmd = REGULATE_VOLT_CMD;
				
				if (error == 0)
				{
					memcpy(&regVolt, &esBuf[3], sizeof(uint16_t));
					boostOutputVoltageRegulate(regVolt);						/* 调用输出电压调节函数. */
					
					retVal = 0;													/* 数据妥收, 应答结果清零. */
				}
				else 
				{
					retVal = 0xff;												/* 数据未妥收, 应答结果置位. */
				}
			}
			else if (esBuf[1] == START_STOP_MACHINE_CMD)						/* 收到的是系统开关机命令. */
			{
				cmd = START_STOP_MACHINE_CMD;
				
				if (error == 0)
				{
					systemCurSwitchingState = (bool)esBuf[3];					/* 系统开关机状态更新. */

					if (systemCurSwitchingState != systemPrevSwitchingState)
					{
						systemSwitchingChangeState = TRUE;						/* 系统开关机状态有变化标志置位. */

						systemPrevSwitchingState = systemCurSwitchingState;		/* 系统上次开关机状态被复写. */
					}
					
					retVal = 0;													/* 数据妥收, 应答结果清零. */
				}
				else 
				{
					retVal = 0xff;												/* 数据未妥收, 应答结果置位. */
				}
			}
			else
			{
				return;
			}
			
			esLen = 0;
			esBuf[esLen++] = 0x06;												/* 帧长度6字节. */
			esBuf[esLen++] = cmd;												/* 命令字节. */
			esBuf[esLen++] = FRAME_TYPE_RESPOND;								/* 帧类型是应答帧. */
			esBuf[esLen++] = retVal;											/* 接收结果. */
			crc = calculateCRC16(esBuf, esLen);
			memcpy(&esBuf[esLen], &crc, sizeof(uint16_t));						/* CRC16校验码. */
			esLen += sizeof(uint16_t);
			
			Len = escfEncode(esBuf, esLen, Buf, sizeof(Buf));					/* 串口发送数据帧之前做"转义序列"编码. */
			Buf[Len + 1] = 0xF2;
			for (uint8_t i = Len; i > 0; i--)
			{
				Buf[i] = Buf[i - 1];
			}
			Buf[0] = 0xF1;
			Len += 2;															/* "转义序列"编码完成. */
			
			usartCommSendData(Buf, Len);                                        /* 调用串口数据发送驱动函数. */
		}
    }
}

/*
 * @函数功能：串口通信发送系统参数信息.
 * @函数参数：无
 * @返回值：无
 */
void commSendSyetemInfo(void)
{
	const uint8_t MAX_ERROR_TIME = 3;
    SystemInfoParaDef_t *pInfoPar;
    uint8_t  Buf[25], Len;
    uint8_t  esBuf[15], esLen;
	uint8_t  txDataBuf[25], txDataLen;
	uint16_t calcCrc;
	uint8_t  error;
	uint16_t commStatus;
	
	pInfoPar = getSystemInfoParaPtr();
	
	if (getSystemInfoReadyFlag() == TRUE)
	{
		if (transmitOrderCntSemaphore == 0)		
		{
			esLen = 0;
			esBuf[esLen++] = 10;
			esBuf[esLen++] = SEND_INPUTVOLT_CMD;
			esBuf[esLen++] = FRAME_TYPE_SEND;
			memcpy(&esBuf[esLen], &pInfoPar->inputVolt, sizeof(float));
			esLen += sizeof(float);
			esBuf[esLen++] = (uint8_t)pInfoPar->inputSta;
			calcCrc = calculateCRC16(esBuf, esLen);
			memcpy(&esBuf[esLen], &calcCrc, sizeof(uint16_t));
			esLen += sizeof(uint16_t);
			
			Len = escfEncode(esBuf, esLen, Buf, sizeof(Buf));						/* 串口发送数据帧之前做"转义序列"编码. */
			Buf[Len + 1] = 0xF2;
			for (uint8_t i = Len; i > 0; i--)
			{
				Buf[i] = Buf[i - 1];
			}
			Buf[0] = 0xF1;
			Len += 2;																/* "转义序列"编码完成. */
			
			memcpy(&txDataBuf[0], &Buf[0], Len);									/* 数据做备份处理用于发送. */
			txDataLen = Len;														
			
			Len               	   = 0;												/* 长度计数器清零,用于准备接收应答数据帧长度. */
			maxRetryTimes          = 3;												/* 最大重传次数3次.     */
			keepingRetryTimes      = 0;												/* 当前重传次数计数器清零. */
			retryDelayTime         = 20;											/* 单次等待超时时间20ms. */											
			alwaysRetryFlag        = FALSE;
			retryAgainFlag         = FALSE;
			waitRespondTimeoutFlag = FALSE;

			usartCommSendData(txDataBuf, txDataLen);                     			/* 调用串口发送驱动函数用于数据发送. */                   	
			tim3TimeoutFunc_Start_LL(retryDelayTime);								/* 启动超时定时器,等待应答超时. */
			transmitOrderCntSemaphore = -1;											/* 传输顺序计数信号量置为无效. */
										
			while ((Len == 0) && (waitRespondTimeoutFlag == FALSE))					/* 等待直到收到应答信号或者等待超时则退出. */
			{
				Len = usartCommReceiveData(Buf);
				
				if (alwaysRetryFlag == TRUE)
				{
					alwaysRetryFlag = FALSE;
					usartCommSendData(txDataBuf, txDataLen);                                        	
					tim3TimeoutFunc_Start_LL(retryDelayTime);
					transmitOrderCntSemaphore = -1;	
				}
				
				if (retryAgainFlag == TRUE)
				{
					retryAgainFlag = FALSE;
					usartCommSendData(txDataBuf, txDataLen);                                        	
					tim3TimeoutFunc_Start_LL(retryDelayTime);
					transmitOrderCntSemaphore = -1;	
				}
			}

			transmitOrderCntSemaphore = 0;
			
			if (Len != 0)														
			{
				tim3TimeoutFunc_Stop_LL();											/* 接收到了应答帧,关闭超时计数器. */
				
				esLen = (uint8_t)escfDecode(Buf, Len, esBuf, sizeof(esBuf));		/* "转义序列"解码. */
				for (uint8_t i = 0; i < esLen; i++) 
				{
					esBuf[i] = esBuf[i + 1];
				}
				esLen = esLen - 2;													/* "转义序列"解码完成. */
				
				error = 0;
				
				if (esBuf[2] != FRAME_TYPE_RESPOND)error = 1;						/* 接收到的帧类型不是应答帧. */
				if (esBuf[0] != esLen)error = 1;									/* 接收到的数据长度不正确. */
				if (dataBufCrcCheck(esBuf, esLen) == 0)error = 1;					/* 接收到的数据CRC校验不正确. */
				if (esBuf[3] != 0)error = 1;										/* 应答帧有效位不等于0表示未妥收. */
				if (esBuf[1] != SEND_INPUTVOLT_CMD)error = 1;
				
				if (error == 0)														/* 收到正确的应答数据帧,通信正确. */
				{
					transmitOrderCntSemaphore = 1;									/* 准备发送下一个参数信息. */

					commStatus = getSystemWorkingStatus();
					commStatus &= ~(1 << UsartComm_TimeOut_Error);
					commStatus &= ~(1 << UsartComm_Data_Error);
					configSystemWorkingStatus(commStatus);

					commRxDataErrorCnt  = 0;
					commTimeoutErrorCnt = 0;
				}
				else																/* 收到错误的应答数据帧,通信错误. */
				{
					commRxDataErrorCnt++;
					if (commRxDataErrorCnt >= MAX_ERROR_TIME)
					{
						commRxDataErrorCnt = 0;
						transmitOrderCntSemaphore = 0;
						
						commStatus = getSystemWorkingStatus();
						commStatus |= (1 << UsartComm_Data_Error);
						configSystemWorkingStatus(commStatus);

						configSystemInfoReadyFlag(FALSE);
						adcSampleInputVolt_Init();
					}						
				}
			}
			else if (waitRespondTimeoutFlag == TRUE)								/* 等待应答帧超时,通信超时. */
			{
				commTimeoutErrorCnt++;
				if (commTimeoutErrorCnt >= MAX_ERROR_TIME)
				{
					commTimeoutErrorCnt = 0;
					transmitOrderCntSemaphore = 0;
					
					commStatus = getSystemWorkingStatus();
					commStatus |= (1 << UsartComm_TimeOut_Error);
					configSystemWorkingStatus(commStatus);

					configSystemInfoReadyFlag(FALSE);
					adcSampleInputVolt_Init();
				}				
			}
		}
		else if (transmitOrderCntSemaphore == 1)									/* 本轮通信需要发送输出电压信息数据. */											
		{
			esLen = 0;
			esBuf[esLen++] = 10;
			esBuf[esLen++] = SEND_OUTPUTVOLT_CMD;
			esBuf[esLen++] = FRAME_TYPE_SEND;
			memcpy(&esBuf[esLen], &pInfoPar->outputVolt, sizeof(float));
			esLen += sizeof(float);
			esBuf[esLen++] = (uint8_t)pInfoPar->outputSta;
			calcCrc = calculateCRC16(esBuf, esLen);
			memcpy(&esBuf[esLen], &calcCrc, sizeof(uint16_t));
			esLen += sizeof(uint16_t);
			
			Len = escfEncode(esBuf, esLen, Buf, sizeof(Buf));						/* 串口发送数据帧之前做"转义序列"编码. */
			Buf[Len + 1] = 0xF2;
			for (uint8_t i = Len; i > 0; i--)
			{
				Buf[i] = Buf[i - 1];
			}
			Buf[0] = 0xF1;
			Len += 2;																/* "转义序列"编码完成. */
			
			memcpy(&txDataBuf[0], &Buf[0], Len);
			txDataLen = Len;
			
			Len               	   = 0;
			maxRetryTimes          = 3;												/* 最大重传次数3次.     */
			keepingRetryTimes      = 0;												/* 当前重传次数计数器.  */
			retryDelayTime         = 20;											/* 单次等待超时时间20ms */											
			alwaysRetryFlag        = FALSE;
			retryAgainFlag         = FALSE;
			waitRespondTimeoutFlag = FALSE;

			usartCommSendData(txDataBuf, txDataLen);                                        	
			tim3TimeoutFunc_Start_LL(retryDelayTime);
			transmitOrderCntSemaphore = -1;											
										
			while ((Len == 0) && (waitRespondTimeoutFlag == FALSE))					/* 等待直到收到应答信号或者等待超时则退出. */
			{
				Len = usartCommReceiveData(Buf);
				
				if (alwaysRetryFlag == TRUE)
				{
					alwaysRetryFlag = FALSE;
					usartCommSendData(txDataBuf, txDataLen);                                        	
					tim3TimeoutFunc_Start_LL(retryDelayTime);
					transmitOrderCntSemaphore = -1;
				}
				
				if (retryAgainFlag == TRUE)
				{
					retryAgainFlag = FALSE;
					usartCommSendData(txDataBuf, txDataLen);                                        	
					tim3TimeoutFunc_Start_LL(retryDelayTime);
					transmitOrderCntSemaphore = -1;
				}
			}

			transmitOrderCntSemaphore = 1;
			
			if (Len != 0)														
			{
				tim3TimeoutFunc_Stop_LL();											/* 接收到了应答帧,关闭超时计数器. */
				
				esLen = (uint8_t)escfDecode(Buf, Len, esBuf, sizeof(esBuf));		/* "转义序列"解码. */
				for (uint8_t i = 0; i < esLen; i++) 
				{
					esBuf[i] = esBuf[i + 1];
				}
				esLen = esLen - 2;
				
				error = 0;
				
				if (esBuf[2] != FRAME_TYPE_RESPOND)error = 1;						/* 接收到的帧类型不是应答帧. */
				if (esBuf[0] != esLen)error = 1;									/* 接收到的数据长度不正确. */
				if (dataBufCrcCheck(esBuf, esLen) == 0)error = 1;					/* 接收到的数据CRC校验不正确. */
				if (esBuf[3] != 0)error = 1;										/* 应答帧有效位不等于0表示未妥收. */
				if (esBuf[1] != SEND_OUTPUTVOLT_CMD)error = 1;
				
				if (error == 0)														/* 收到正确的应答数据帧,通信正确. */
				{
					transmitOrderCntSemaphore = 2;									/* 准备发送下一个参数信息. */

					commStatus = getSystemWorkingStatus();
					commStatus &= ~(1 << UsartComm_TimeOut_Error);
					commStatus &= ~(1 << UsartComm_Data_Error);
					configSystemWorkingStatus(commStatus);

					commRxDataErrorCnt  = 0;
					commTimeoutErrorCnt = 0;								
				}
				else																/* 收到错误的应答数据帧,通信错误. */
				{
					commRxDataErrorCnt++;
					if (commRxDataErrorCnt >= MAX_ERROR_TIME)
					{
						commRxDataErrorCnt = 0;
						transmitOrderCntSemaphore = 0;
						
						commStatus = getSystemWorkingStatus();
						commStatus |= (1 << UsartComm_Data_Error);
						configSystemWorkingStatus(commStatus);

						configSystemInfoReadyFlag(FALSE);
						adcSampleInputVolt_Init();
					}							
				}
			}
			else if (waitRespondTimeoutFlag == TRUE)								/* 等待应答帧超时,通信超时. */
			{
				commTimeoutErrorCnt++;
				if (commTimeoutErrorCnt >= MAX_ERROR_TIME)
				{
					commTimeoutErrorCnt = 0;
					transmitOrderCntSemaphore = 0;
					
					commStatus = getSystemWorkingStatus();
					commStatus |= (1 << UsartComm_TimeOut_Error);
					configSystemWorkingStatus(commStatus);

					configSystemInfoReadyFlag(FALSE);
					adcSampleInputVolt_Init();
				}									
			}
		}
		else if (transmitOrderCntSemaphore == 2)									/* 本轮通信需要发送温度信息数据. */				
		{
			esLen = 0;
			esBuf[esLen++] = 6;
			esBuf[esLen++] = SEND_TEMPERATURE_CMD;
			esBuf[esLen++] = FRAME_TYPE_SEND;
			memcpy(&esBuf[esLen], &pInfoPar->tempVal, sizeof(int8_t));
			esLen += sizeof(int8_t);
			calcCrc = calculateCRC16(esBuf, esLen);
			memcpy(&esBuf[esLen], &calcCrc, sizeof(uint16_t));
			esLen += sizeof(uint16_t);
			
			Len = escfEncode(esBuf, esLen, Buf, sizeof(Buf));						/* 串口发送数据帧之前做"转义序列"编码. */
			Buf[Len + 1] = 0xF2;
			for (uint8_t i = Len; i > 0; i--)
			{
				Buf[i] = Buf[i - 1];
			}
			Buf[0] = 0xF1;
			Len += 2;																/* "转义序列"编码完成. */
			
			memcpy(&txDataBuf[0], &Buf[0], Len);
			txDataLen = Len;
			
			Len               	   = 0;
			maxRetryTimes          = 3;												/* 最大重传次数3次.     */
			keepingRetryTimes      = 0;												/* 当前重传次数计数器.  */
			retryDelayTime         = 20;											/* 单次等待超时时间20ms */											
			alwaysRetryFlag        = FALSE;
			retryAgainFlag         = FALSE;
			waitRespondTimeoutFlag = FALSE;

			usartCommSendData(txDataBuf, txDataLen);                                        	
			tim3TimeoutFunc_Start_LL(retryDelayTime);
			transmitOrderCntSemaphore = -1;											
										
			while ((Len == 0) && (waitRespondTimeoutFlag == FALSE))					/* 等待直到收到应答信号或者等待超时则退出. */
			{
				Len = usartCommReceiveData(Buf);
				
				if (alwaysRetryFlag == TRUE)
				{
					alwaysRetryFlag = FALSE;
					usartCommSendData(txDataBuf, txDataLen);                                        	
					tim3TimeoutFunc_Start_LL(retryDelayTime);	
					transmitOrderCntSemaphore = -1;
				}
				
				if (retryAgainFlag == TRUE)
				{
					retryAgainFlag = FALSE;
					usartCommSendData(txDataBuf, txDataLen);                                        	
					tim3TimeoutFunc_Start_LL(retryDelayTime);	
					transmitOrderCntSemaphore = -1;
				}
			}

			transmitOrderCntSemaphore = 2;
			
			if (Len != 0)														
			{
				tim3TimeoutFunc_Stop_LL();											/* 接收到了应答帧,关闭超时计数器. */
				
				esLen = (uint8_t)escfDecode(Buf, Len, esBuf, sizeof(esBuf));		/* "转义序列"解码. */
				for (uint8_t i = 0; i < esLen; i++) 
				{
					esBuf[i] = esBuf[i + 1];
				}
				esLen = esLen - 2;
				
				error = 0;
				
				if (esBuf[2] != FRAME_TYPE_RESPOND)error = 1;						/* 接收到的帧类型不是应答帧. */
				if (esBuf[0] != esLen)error = 1;									/* 接收到的数据长度不正确. */
				if (dataBufCrcCheck(esBuf, esLen) == 0)error = 1;					/* 接收到的数据CRC校验不正确. */
				if (esBuf[3] != 0)error = 1;										/* 应答帧有效位不等于0表示未妥收. */
				if (esBuf[1] != SEND_TEMPERATURE_CMD)error = 1;
				
				if (error == 0)														/* 收到正确的应答数据帧,通信正确. */
				{
					transmitOrderCntSemaphore = 0;

					commStatus = getSystemWorkingStatus();
					commStatus &= ~(1 << UsartComm_TimeOut_Error);
					commStatus &= ~(1 << UsartComm_Data_Error);
					configSystemWorkingStatus(commStatus);

					commRxDataErrorCnt  = 0;
					commTimeoutErrorCnt = 0;

					configSystemInfoReadyFlag(FALSE);
					adcSampleInputVolt_Init();
				}
				else																/* 收到错误的应答数据帧,通信错误. */
				{
					commRxDataErrorCnt++;
					if (commRxDataErrorCnt >= MAX_ERROR_TIME)
					{
						commRxDataErrorCnt = 0;
						transmitOrderCntSemaphore = 0;
						
						commStatus = getSystemWorkingStatus();
						commStatus |= (1 << UsartComm_Data_Error);
						configSystemWorkingStatus(commStatus);	

						configSystemInfoReadyFlag(FALSE);
						adcSampleInputVolt_Init();
					}		
				}
			}
			else if (waitRespondTimeoutFlag == TRUE)								/* 等待应答帧超时,通信超时. */
			{
				commTimeoutErrorCnt++;
				if (commTimeoutErrorCnt >= MAX_ERROR_TIME)
				{
					commTimeoutErrorCnt = 0;
					transmitOrderCntSemaphore = 0;
					
					commStatus = getSystemWorkingStatus();
					commStatus |= (1 << UsartComm_TimeOut_Error);
					configSystemWorkingStatus(commStatus);

					configSystemInfoReadyFlag(FALSE);
					adcSampleInputVolt_Init();
				}	
			}
		}
	}
}

/*
 * @函数功能：串口通信发送系统参数信息.
 * @函数参数：无
 * @返回值：无
 */
void commTimeoutCallback(void)
{
	if (transmitOrderCntSemaphore == -1)
	{
		if (maxRetryTimes == -1)
		{
			alwaysRetryFlag = TRUE;
		}
		else if (keepingRetryTimes < (maxRetryTimes - 1))
		{
			keepingRetryTimes += 1;
			retryAgainFlag = TRUE;
		}
		else
		{
			waitRespondTimeoutFlag = TRUE;
		}
	}
}
