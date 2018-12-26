
#ifndef  __TIM1PWMCTRL_H
#define  __TIM1PWMCTRL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
   
#define  PWM_VOLTAGE_CHANNEL
   
#define  VOLTAGE_CHANNEL        (TIM1_CHANNEL_1)
#define  VOLTAGE_CH_IDLE_STATE  (0)
   
#define  VOLTAGE_CH_GPIO_PORT   (GPIOC)
#define  VOLTAGE_CH_PIN         (GPIO_PIN_1)


void tim1PwmControlInit_LL(void);
void configVoltageChannel_DutyCycle_LL(float dutyCycle);
   
   
#ifdef __cplusplus
}
#endif

#endif
