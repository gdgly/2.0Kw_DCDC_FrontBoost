
#ifndef  __COMM_H
#define  __COMM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
   
#define  SEND_INPUTVOLT_CMD		(0x10)
#define  SEND_OUTPUTVOLT_CMD	(0x11)
#define  SEND_TEMPERATURE_CMD	(0x12)
#define  START_STOP_MACHINE_CMD	(0x13)
#define  REGULATE_VOLT_CMD      (0x14)

#define  FRAME_TYPE_SEND		(0x00)
#define  FRAME_TYPE_RESPOND		(0xFF)
   
   
bool getSystemMachineStatus(void);
void configSystemMachineStatus(bool sta);

void commReceivedFrameParsing(void);
void commSendSyetemInfo(void);
void commTimeoutCallback(void);


#ifdef __cplusplus
}
#endif

#endif

