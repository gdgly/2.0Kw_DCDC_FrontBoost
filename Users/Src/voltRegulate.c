
#include "voltRegulate.h"
#include "tim1PwmCtrl.h"


/*
 * @函数功能：
 * @函数参数：
 * @返回值：无
 */
void boostOutputVoltageRegulate(uint16_t inputVolt)
{
    float volt;
    float dutyCycle, temp;
    
    if (inputVolt > 51300) {
        volt = 51300.0 / 100.0;
    } else if (inputVolt < 44000) {
        volt = 44000.0 / 100.0;
    } else {
        volt = (float)inputVolt / 100.0;
    }
    
    temp = 528.984 - volt;
    dutyCycle = temp / 140.67;
    dutyCycle = dutyCycle * 100.0;
    
    configVoltageChannel_DutyCycle_LL(dutyCycle);
}


