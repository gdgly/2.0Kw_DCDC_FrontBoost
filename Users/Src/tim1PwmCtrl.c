
#include "tim1PwmCtrl.h"


static float voltageDutyCycle = 15.6;            /* 默认产生占空比为63.5%的PWM波形. */

/**
  * @函数功能：Timer1定时器的通道1的输出比较模式初始化.
  * @函数参数：无
  * @返回值：无
  */
void tim1PwmControlInit_LL(void)
{
    uint16_t voltChannelCCR;
    
    voltChannelCCR = (uint16_t)(voltageDutyCycle * 10.0);
      
#ifdef PWM_VOLTAGE_CHANNEL
#if (VOLTAGE_CH_IDLE_STATE == 0)
    GPIO_Init(VOLTAGE_CH_GPIO_PORT, VOLTAGE_CH_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
#else 
    GPIO_Init(VOLTAGE_CH_GPIO_PORT, VOLTAGE_CH_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);
#endif
#endif
    
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, ENABLE);
    
    /*TIM1 Frequency = TIM1 counter clock/(ARR + 1) */
    TIM1_TimeBaseInit(15, 
                      TIM1_COUNTERMODE_UP,
                      999,
                      0);                                           /* TIM1 Frequency=16M/(15+1)/(999+1)=1KHz */
    
    TIM1_ARRPreloadConfig(DISABLE);                                 /* 不使能自动重装载,影子寄存器的值在写入ARR寄存器时立马更新 */
    
#ifdef PWM_VOLTAGE_CHANNEL
    TIM1_OC1Init(TIM1_OCMODE_PWM1, 
                 TIM1_OUTPUTSTATE_ENABLE,                           /* PWM输出打开 */
                 TIM1_OUTPUTNSTATE_DISABLE,                         /* PWM反向输出关闭 */
                 voltChannelCCR,                                    /* CCR寄存器赋值,该值决定占空比 */
                 TIM1_OCPOLARITY_HIGH,                              /* COUNT计数器值小于CCR时PWM输出高 */
                 TIM1_OCNPOLARITY_LOW,                              /* PWM反向输出低.此处反向输出禁止,该值无影响 */
                 TIM1_OCIDLESTATE_RESET,                            /* PWM空闲状态输出低 */
                 TIM1_OCNIDLESTATE_SET);                            /* PWM反向空闲状态输出高 */
    
    TIM1_CCxCmd(VOLTAGE_CHANNEL, ENABLE);
    TIM1_OC1PreloadConfig(ENABLE);
#endif
    
    TIM1_CtrlPWMOutputs(ENABLE);
    
    TIM1_Cmd(ENABLE);
}

/**
  * @函数功能：修改电压通道输出的PWM占空比.
  * @函数参数：dutyCycle,占空比.该参数是一个浮点类型的数据.
  * @返回值：无
  */
void configVoltageChannel_DutyCycle_LL(float dutyCycle)
{
    uint16_t ccr_val;
    
    ccr_val = (uint16_t )(dutyCycle * 10.0);
    
    TIM1_SetCompare1(ccr_val);
}

