
#ifndef  __COMM_H
#define  __COMM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
   
#define  REGULATE_VOLT_CMD      (0x01)
   
   
void commReceivedFrameParsing(void);
void commSendCtrlInfo(void);

#ifdef __cplusplus
}
#endif

#endif

