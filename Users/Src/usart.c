
#include <stdlib.h>
#include "usart.h"

/*
 * @函数功能：串口1配置初始化.
 * @函数参数：无
 * @返回值：无
 */
void usartConfig_LL(void)
{
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART2, ENABLE);
    
    UART2_DeInit();
    
    UART2_Init(9600,
               UART2_WORDLENGTH_8D,
               UART2_STOPBITS_1,
               UART2_PARITY_NO,
               UART2_SYNCMODE_CLOCK_DISABLE,
               UART2_MODE_TXRX_ENABLE);
    
#if (USART_RX_INT_ENABLE == 1)
    UART2_ITConfig(UART2_IT_RXNE_OR, ENABLE);
#endif
    
#if (USART_TX_INT_ENABLE == 1)
    UART2_ITConfig(UART2_IT_TXE, ENABLE);
#endif
    
    UART2_Cmd(ENABLE);
}

/*
 * @函数功能：串口发送单个字节数据.
 * @函数参数：data,待发送的单字节数据.
 * @返回值：无
 */
void usartSendData_LL(uint8_t data)
{
    UART2_SendData8(data);
    
#if (USART_TX_INT_ENABLE == 0)
    while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET){;}
#endif
}

/*
 * @函数功能：串口接收单个字节数据.
 * @函数参数：无
 * @返回值：串口实际接收到的单个字节数据.
 */
uint8_t usartReceiveData_LL(void)
{
    uint8_t rxData;
    
#if (USART_RX_INT_ENABLE == 0)
    while (UART2_GetFlagStatus(UART2_FLAG_RXNE) == RESET){;}
#endif
    
    rxData = UART2_ReceiveData8();
    
    return (rxData);
}

/*
 * @函数功能：串口发送指定长度的数据.
 * @函数参数：pTxData, 待发送的数据存储缓冲区;
 * @函数参数：length, 指定的数据长度;
 * @返回值：无
 */
void usartSendNBytesData(uint8_t* pTxData, uint32_t length)
{
    uint32_t i;
    
    if (pTxData == NULL)return;
    
#if (USART_TX_INT_ENABLE == 0)
    for (i = 0; i < length; i++)
    {
        usartSendData_LL(pTxData[i]);
    }
#elif (USART_TX_INT_ENABLE == 1)
    /* TODO: Add code here to... */
#endif
}

