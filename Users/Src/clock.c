
#include "clock.h"

/*
 * @函数功能：硬件系统时钟初始化.注意：该函数用于配置系统各时钟源和频率,需要在进入main函数后第一步就要调用.
 * @函数参数：无
 * @返回值：无
 */
void systemClockInit_LL(void)
{
    CLK_DeInit();
    CLK_LSICmd(ENABLE);                                             /* 打开片内低速RC(128KHz)时钟 */
	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
    CLK_HSICmd(ENABLE);                                             /* 打开片内高速RC(16MHz)时钟  */
    while (CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == RESET){;}          /* 等待片内高速RC时钟稳定输出 */
    
//    CLK_HSECmd(ENABLE);
//    while (CLK_GetFlagStatus(CLK_FLAG_HSERDY) == RESET){;}
//    while (CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSE, DISABLE, CLK_CURRENTCLOCKSTATE_DISABLE) == ERROR){;}
    
    CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);                        /* 配置Fcpu主频为1分频,即16MHz */
}
