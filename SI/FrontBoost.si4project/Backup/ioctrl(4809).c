
#include "ioctrl.h"
#include "comm.h"
#include "adcSample.h"
#include "tim4tick.h"

static bool relayStatus      = FALSE;
static bool anologChipStatus = FALSE;
static bool ncpChipStatus    = FALSE;


bool getNcpChipStatus(void)
{
    return (ncpChipStatus);
}

/*
 * @函数功能：IO输入输出端口初始化
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlInit_LL(void)
{
    GPIO_Init(RELAY_CTRL_PORT, RELAY_CTRL_PIN, GPIO_MODE_OUT_PP_LOW_FAST);        /* 上电默认关闭继电器输出. */
    GPIO_Init(ANOLOG_CTRL_PORT, ANOLOG_CTRL_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW);     /* 上电默认禁止模拟主控芯片UC3843使能. */
	GPIO_Init(PFCVS22_CTRL_PORT, PFCVS22_CTRL_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);	  /* 上电默认关断后级PFC输出,需要在接收到开机指令后才打开. */
}

/*
 * @函数功能：继电器输出打开
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlRelayOpen_LL(void)
{
	if (relayStatus == FALSE)
	{
    	GPIO_WriteHigh(RELAY_CTRL_PORT, RELAY_CTRL_PIN);
		relayStatus = TRUE;
	}
}

/*
 * @函数功能：继电器输出关闭
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlRelayClose_LL(void)
{
	if (relayStatus == TRUE)
	{
    	GPIO_WriteLow(RELAY_CTRL_PORT, RELAY_CTRL_PIN);
		relayStatus = FALSE;
	}
}

/*
 * @函数功能：模拟芯片驱动使能打开
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlAnologEnable_LL(void)
{
	if (anologChipStatus == FALSE)
	{
    	GPIO_WriteLow(ANOLOG_CTRL_PORT, ANOLOG_CTRL_PIN);
		anologChipStatus = TRUE;
	}
}

/*
 * @函数功能：模拟芯片驱动使能关闭
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlAnologDisable_LL(void)
{
	if (anologChipStatus == TRUE)
	{
    	GPIO_WriteHigh(ANOLOG_CTRL_PORT, ANOLOG_CTRL_PIN);
		anologChipStatus = FALSE;
	}
}

/*
 * @函数功能：PFV芯片驱动使能打开
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlPFCVS22Enable_LL(void)
{
	if (ncpChipStatus == FALSE)
	{
		GPIO_WriteLow(PFCVS22_CTRL_PORT, PFCVS22_CTRL_PIN);
		ncpChipStatus = TRUE;
	}
}

/*
 * @函数功能：PFV芯片驱动使能关闭
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlPFCVS22Disable_LL(void)
{
	if (ncpChipStatus == TRUE)
	{
		GPIO_WriteHigh(PFCVS22_CTRL_PORT, PFCVS22_CTRL_PIN);
		ncpChipStatus = FALSE;
	}
}

/*
 * @函数功能：PFV芯片驱动使能关闭
 * @函数参数：无
 * @返回值：无
 */
void ioCtrlProcessing(void)
{
	SystemInfoParaDef_t *pInfoPar = getSystemInfoParaPtr();
	
//	if (getSystemMachineStateChangeFlag() == TRUE)					/* 系统开关机状态有变化吗? */
//	{
//		if (getSystemMachineStatus() == TRUE)						/* 是开机吗? */
//		{
//			ioCtrlAnologEnable_LL();

			if (pInfoPar->outputSta == FullPower)
			{
				ioCtrlPFCVS22Enable_LL();
			}

			if (ncpChipStatus == TRUE)
			{
				configSystemMachineStateChangeFlag(FALSE);
			}
//		}
#if 0
		else														/* 是关机吗? */
		{
			ioCtrlPFCVS22Disable_LL();

			systemDelayms(100);

			ioCtrlAnologDisable_LL();

			configSystemMachineStateChangeFlag(FALSE);
		}
#endif
//	}
}



