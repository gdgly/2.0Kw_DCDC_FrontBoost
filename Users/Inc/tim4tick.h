
#ifndef __TIM4TICK_H
#define __TIM4TICK_H

#include "main.h"


void systemTimTickInit_LL(void);
void timTick_Decrement(void);
void systemDelayms(uint32_t nCount);

#endif




