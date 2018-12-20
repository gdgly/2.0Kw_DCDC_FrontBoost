
#include "ioctrl.h"

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void ioCtrlInit_LL(void)
{
    GPIO_Init(RELAY_CTRL_PORT, RELAY_CTRL_PIN, GPIO_MODE_OUT_PP_LOW_FAST);        /* 上电默认关闭继电器输出. */
    GPIO_Init(ANOLOG_CTRL_PORT, ANOLOG_CTRL_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);     /* 上电默认禁止模拟主控芯片UC3843使能. */
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void ioCtrlRelayOpen_LL(void)
{
    GPIO_WriteHigh(RELAY_CTRL_PORT, RELAY_CTRL_PIN);
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void ioCtrlRelayClose_LL(void)
{
    GPIO_WriteLow(RELAY_CTRL_PORT, RELAY_CTRL_PIN);
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void ioCtrlAnologEnable_LL(void)
{
    GPIO_WriteLow(ANOLOG_CTRL_PORT, ANOLOG_CTRL_PIN);
}

/*
 * @函数功能：
 * @函数参数：
 * @返回值：
 */
void ioCtrlAnologDisable_LL(void)
{
    GPIO_WriteHigh(ANOLOG_CTRL_PORT, ANOLOG_CTRL_PIN);
}

