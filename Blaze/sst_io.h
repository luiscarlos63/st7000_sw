/**
 * @brief This files defines the IO interfaces for the Super Secure Tiles
 *
 * @author Valetinho
 *
 */

#ifndef __SST_IO_H_
#define __SST_IO_H_




/***************************** Include Files *********************************/
#include "xparameters.h"

#include "xgpiops.h"
#include "xmbox.h"


/************************** Constant Definitions *****************************/

/* ------------ PSU GPIO in sst_io.c ------------------*/
#define SST_LED_1 7
#define SST_LED_2 18
/* ------------ PSU UART0 in sst_io.c -----------------*/


/* ------------ SST MAILBOX in sst_io.c ---------------*/


/**************************** Type Definitions *******************************/



/************************** Variable Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/* PSU GPIO in sst_io.c */

int32_t sst_ioPsGpioInit();
int32_t sst_ioPsGpioSetLED(uint8_t led, uint8_t value);


/* PSU UART0 in sst_io.c */

/* SST MAILBOX in sst_io.c */
















#endif
