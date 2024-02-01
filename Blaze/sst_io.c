/**
 * @brief This files defines the IO interfaces for the Super Secure Tiles
 *
 * @author Valetinho
 *
 */


/***************************** Include Files *********************************/

#include "sst_io.h"

/************************** Constant Definitions *****************************/



/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/


/************************** Function Prototypes ******************************/


/*************************** Function Defines ********************************/




/* ------------ PSU GPIO in sst_io.c ------------------*/

XGpioPs Gpio;
XGpioPs_Config *ConfigPtr;



int32_t sst_ioPsGpioInit()
{
	int32_t status;
	ConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr,
						ConfigPtr->BaseAddr);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//Set PSU LEDS
	XGpioPs_SetDirectionPin(&Gpio, SST_LED_1, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, SST_LED_1, 1);

	return status;
}


int32_t sst_ioPsGpioSetLED(uint8_t led, uint8_t value)
{
	/* Set the GPIO output to be low. */
	XGpioPs_WritePin(&Gpio, led, value);
	return 0;
}

/* ------------ PSU UART0 in sst_io.c -----------------*/


/* ------------ SST MAILBOX in sst_io.c ---------------*/







