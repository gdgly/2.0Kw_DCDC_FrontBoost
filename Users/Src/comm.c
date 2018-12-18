
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
    uint8_t Buf[20], Len;
    uint8_t esBuf[10], esLen;
    uint8_t retVal = 0xff;
    uint16_t crc;
    float duty;
    
    Len = usartCommReceiveData(Buf);
    
    if (Len != 0) {
        esLen = (uint8_t)escfDecode(Buf, Len, esBuf, sizeof(esBuf));
        for (uint8_t i = 0; i < esLen; i++) {
            esBuf[i] = esBuf[i + 1];
        }
        esLen = esLen - 2;
        
        if (esBuf[0] == REGULATE_VOLT_CMD) {
            if (dataBufCrcCheck(esBuf, esLen) != 0) {
                memcpy(&duty, &esBuf[1], sizeof(float));
                configVoltageChannel_DutyCycle_LL(duty);                        /* 调用修改电压通道占空比输出底层驱动函数. */
                
                retVal = 0;
            } else {
                retVal = 0xff;
            }
            
            esBuf[1] = retVal;
            
            crc = crc16(esBuf, 2);
            memcpy(&esBuf[2], &crc, sizeof(uint16_t));
            esLen =  2 + sizeof(uint16_t);
            
            Len = escfEncode(esBuf, esLen, Buf, sizeof(Buf));
            usartCommSendData(Buf, Len);                                        /* 调用串口数据发送驱动函数. */
        }
    }
}

