
//#ifdef ST_DEBUG_IO

/**
 *
 */
#ifndef __ST_DEBUG_H_
#define __ST_DEBUG_H_








/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xparameters.h"
#include "xgpiops.h"
#include <stdint.h>
/************************** Constant Definitions *****************************/
#define PS_LED_4 7


/**************************** Type Definitions *******************************/



/************************** Variable Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/




/* PSU GPIO in sst_io.c */

int32_t stDebug_ioPsGpioInit();
int32_t stDebug_ioPsGpioSetLED(uint8_t led, uint8_t value);


/* PSU UART0 in sst_io.c */

/* SST MAILBOX in sst_io.c */













//#endif	//ST_DEBUG_IO
#endif	//__ST_DEBUG_H_
