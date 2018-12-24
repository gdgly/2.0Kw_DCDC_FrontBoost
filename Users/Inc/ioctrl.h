
#ifndef  __IOCTRL_H
#define  __IOCTRL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define  RELAY_CTRL_PORT    (GPIOC)
#define  RELAY_CTRL_PIN     (GPIO_PIN_4) 
   
#define  ANOLOG_CTRL_PORT   (GPIOE)
#define  ANOLOG_CTRL_PIN    (GPIO_PIN_5)

#define  PFCVS22_CTRL_PORT	(GPIOD)
#define  PFCVS22_CTRL_PIN	(GPIO_PIN_7)
   
void ioCtrlInit_LL(void);
void ioCtrlRelayOpen_LL(void);
void ioCtrlRelayClose_LL(void);
void ioCtrlAnologEnable_LL(void);
void ioCtrlAnologDisable_LL(void);
void ioCtrlPFCVS22Enable_LL(void);
void ioCtrlPFCVS22Disable_LL(void);

#ifdef __cplusplus
}
#endif

#endif

