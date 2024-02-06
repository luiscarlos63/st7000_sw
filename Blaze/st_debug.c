/*
 * Luís
 * This file ....
 * 1/2/
 */

/***************************** Include Files **********************************/
#include "st_debug.h"


/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/
// PS GPIO (used for PS MIO LEDs)
XGpioPs Gpio;
XGpioPs_Config *ConfigPtr;


/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/

/* ------------ PSU GPIO in sst_io.c ------------------*/

int32_t stDebug_ioPsGpioInit()
{
	int32_t status;
	ConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr,
						ConfigPtr->BaseAddr);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//Set PSU LEDS
	XGpioPs_SetDirectionPin(&Gpio, PS_LED_4, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, PS_LED_4, 1);

	return status;
}


int32_t stDebug_ioPsGpioSetLED(uint8_t led, uint8_t value)
{
	/* Set the GPIO output to be low. */
	XGpioPs_WritePin(&Gpio, led, value);
	return 0;
}

/* ------------ PSU UART0 in sst_io.c -----------------*/
