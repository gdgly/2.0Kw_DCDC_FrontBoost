
#include <string.h>
#include "comm.h"
#include "usart.h"
#include "escf.h"
#include "Crc16.h"
#include "tim1PwmCtrl.h"

/*
 * @函数功能：串口通信接收到的数据帧解析。
 * @函数参数：无
 * @返回值：无
 */
void commReceivedFrameParsing(void)
{
    uint8_t Buf[30], Len;
    uint8_t esBuf[15], esLen;
    uint8_t error, sysTatus, retVal = 0xff;
    uint16_t crc;
    uint16_t regVolt;
    
    Len = usartCommReceiveData(Buf);
    
    if (Len != 0) 
	{
        esLen = (uint8_t)escfDecode(Buf, Len, esBuf, sizeof(esBuf));
        for (uint8_t i = 0; i < esLen; i++) 
		{
            esBuf[i] = esBuf[i + 1];
        }
        esLen = esLen - 2;
		
		error = 0;
		
		if ((esBuf[0] != esLen) || (esBuf[2] != 0x00))
		{
			error = 1;
		}
		
		if (dataBufCrcCheck(esBuf, esLen) != 0)
		{
			error = 1;
		}

        if (esBuf[1] == REGULATE_VOLT_CMD) 
		{
            if (error == 0) 
			{
                memcpy(&regVolt, &esBuf[3], sizeof(uint16_t));
				
				/* TODO: Add code here to do... */
				
                retVal = 0;
            } 
			else 
			{
                retVal = 0xff;
            }
            
			esLen = 0;
            esBuf[esLen++] = 0x06;
			esBuf[esLen++] = REGULATE_VOLT_CMD;
			esBuf[esLen++] = 0xFF;
			esBuf[esLen++] = retVal;
            crc = crc16(esBuf, esLen);
            memcpy(&esBuf[esLen], &crc, sizeof(uint16_t));
            esLen = esLen + sizeof(uint16_t);
            
            Len = escfEncode(esBuf, esLen, Buf, sizeof(Buf));
            Buf[Len + 1] = 0xF2;
            for (uint8_t i = Len; i > 0; i--) 
			{
                Buf[i] = Buf[i - 1];
            }
            Buf[0] = 0xF1;
            Len += 2;
            
            usartCommSendData(Buf, Len);                                        /* 调用串口数据发送驱动函数. */
        }
		else if (esBuf[1] == START_STOP_MACHINE_CMD)
		{
            if (error == 0) 
			{
                sysTatus = esBuf[3];
				
				/* TODO: Add code here to do... */
				
                retVal = 0;
            } 
			else 
			{
                retVal = 0xff;
            }	

			esLen = 0;
            esBuf[esLen++] = 0x06;
			esBuf[esLen++] = START_STOP_MACHINE_CMD;
			esBuf[esLen++] = 0xFF;
			esBuf[esLen++] = retVal;
            crc = crc16(esBuf, esLen);
            memcpy(&esBuf[esLen], &crc, sizeof(uint16_t));
            esLen = esLen + sizeof(uint16_t);
            
            Len = escfEncode(esBuf, esLen, Buf, sizeof(Buf));
            Buf[Len + 1] = 0xF2;
            for (uint8_t i = Len; i > 0; i--) 
			{
                Buf[i] = Buf[i - 1];
            }
            Buf[0] = 0xF1;
            Len += 2;
            
            usartCommSendData(Buf, Len);                                        /* 调用串口数据发送驱动函数. */			
		}
    }
}

/*
 * @函数功能：串口通信发送控制信息
 * @函数参数：无
 * @返回值：无
 */
void commSendCtrlInfo(void)
{
    
}

