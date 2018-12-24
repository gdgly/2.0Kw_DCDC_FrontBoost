
#include "ioctrl.h"

/*
 * @函数功能：IO输入输出端口初始化
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlInit_LL(void)
{
    GPIO_Init(RELAY_CTRL_PORT, RELAY_CTRL_PIN, GPIO_MODE_OUT_PP_LOW_FAST);        /* 上电默认关闭继电器输出. */
    GPIO_Init(ANOLOG_CTRL_PORT, ANOLOG_CTRL_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW);     /* 上电默认禁止模拟主控芯片UC3843使能. */
	GPIO_Init(PFCVS22_CTRL_PORT, PFCVS22_CTRL_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW);	  
}

/*
 * @函数功能：继电器输出打开
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlRelayOpen_LL(void)
{
    GPIO_WriteHigh(RELAY_CTRL_PORT, RELAY_CTRL_PIN);
}

/*
 * @函数功能：继电器输出关闭
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlRelayClose_LL(void)
{
    GPIO_WriteLow(RELAY_CTRL_PORT, RELAY_CTRL_PIN);
}

/*
 * @函数功能：模拟芯片驱动使能打开
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlAnologEnable_LL(void)
{
    GPIO_WriteLow(ANOLOG_CTRL_PORT, ANOLOG_CTRL_PIN);
}

/*
 * @函数功能：模拟芯片驱动使能关闭
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlAnologDisable_LL(void)
{
    GPIO_WriteHigh(ANOLOG_CTRL_PORT, ANOLOG_CTRL_PIN);
}

/*
 * @函数功能：PFV芯片驱动使能打开
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlPFCVS22Enable_LL(void)
{
	GPIO_WriteLow(PFCVS22_CTRL_PORT, PFCVS22_CTRL_PIN);
}

/*
 * @函数功能：PFV芯片驱动使能关闭
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlPFCVS22Disable_LL(void)
{
	GPIO_WriteHigh(PFCVS22_CTRL_PORT, PFCVS22_CTRL_PIN);
}




