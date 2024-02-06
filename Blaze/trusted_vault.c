/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#define ST_DEBUG_IO

/***************************** Include Files *********************************/
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include "sleep.h"  // Include the sleep header
#include "xparameters.h"
#include "xmbox.h"
#include "st_debug.h"



/************************** Constant Definitions *****************************/
// Hardware definitions
#define MBOX_DEVICE_ID		XPAR_MBOX_0_DEVICE_ID
#define MBOX_INTR_ID		XPAR_FABRIC_MBOX_0_VEC_ID

#define ST_MAIL_ID			MBOX_DEVICE_ID
#define ST_MAIL_INTR_ID		MBOX_INTR_ID

#define MAILBOX_RIT	4	/* mailbox receive interrupt threshold */
#define MAILBOX_SIT	4	/* mailbox send interrupt threshold */

#define FROM_HOST	0
#define FROM_VAULT	1
/**************************** Type Definitions *******************************/
typedef struct
{
	uint8_t nonce[32];
	uint8_t attest_pk[32];
	uint8_t kernel_cert_hash[48];
	uint8_t kernel_cert_sig[512];
	uint8_t attest_sig[64];
}BitAttestation_t;

typedef struct
{
	uint32_t tileId;		//reconfigurable partition id
	uint32_t ipId;			//reconfigurable module id
	uint32_t *data;
	uint32_t size;
	BitAttestation_t attest;
}Bitstream_t;

typedef struct
{
	Bitstream_t *bit;		//pointer to a bitstream
	uint32_t tileIdSave;		//reconfigurable partition id
	uint32_t ipIdSave;			//reconfigurable module id
	uint8_t bitSessionKey[256];
}VaultBit;


typedef enum SecureTilesCommands
{
	ST_BIT_GET_ATTEST 		= 0x10,
	ST_BIT_SET_ENC_KEY,

	ST_LOADER_BIT			= 0x20,

	ST_TILE_STATUS 			= 0x30,


	//Errors
	ST_ERROR_TYPE_INV		= 0x80000000,
	ST_ERROR_CONTENT_INV,
	ST_ERROR_SOURCE_INV
}SecTilesComm_t;


typedef struct
{
	SecTilesComm_t type;
	uint32_t *content;
	uint32_t src;
	uint32_t id;
}Mail_t;
/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/
// Mailbox
static XMbox Mbox;


//TrustedVault

/************************** Function Prototypes ******************************/

//Commands
int32_t stComm_getAttest(Bitstream_t *bit);


static int32_t mailInit();
static int32_t mailSendMsg(Mail_t *msg);
static int32_t mailRecvMsg(Mail_t *msg);



/*************************** Function Defines ********************************/

int main()
{

	Mail_t msgComm;
	int32_t retRecv = 0,
			retSend = 0;
	int32_t retComm = 0;


	// ----- Initializations -----
    init_platform();
#ifdef ST_DEBUG_IO
    stDebug_ioPsGpioInit();
#endif // ST_DEBUG_IO
    mailInit();

    while(1)
    {
    	retRecv = mailRecvMsg(&msgComm);

		if(msgComm.content != NULL)

    	switch (msgComm.type) {
			case ST_BIT_GET_ATTEST:
				stComm_getAttest((Bitstream_t*)msgComm.content);
			break;
			case ST_BIT_SET_ENC_KEY:

			break;
			case ST_LOADER_BIT:
				//verify if the saved TILE and IP ids still match

				//Decrypt the bitstream key with session key

				//Load the key to the AES module

				//update DFX and trigge the loading process
			break;
			case ST_TILE_STATUS:

			break;

			default:

			break;
		}
		else
		{
			msgComm.type = ST_ERROR_CONTENT_INV;
		}

    	//Change mail source and send it back (meaning it is ready)
    	msgComm.src = FROM_VAULT;
    	retSend = mailSendMsg(&msgComm);
    }

    cleanup_platform();
    return 0;
}


/*
 * COMMANDS
 */

int32_t stComm_getAttest(Bitstream_t *bit)
{
	static val = 0;

	if(val == 0)
		val = 1;
	else
		val = 0;

	stDebug_ioPsGpioSetLED(PS_LED_4, val);

	return 0;
}



static int32_t mailSendMsg(Mail_t *msg)
{
	XMbox_WriteBlocking(&Mbox, (uint32_t*)msg, sizeof(Mail_t));
	return msg->id;
}


static int32_t mailRecvMsg(Mail_t *msg)
{
	XMbox_ReadBlocking(&Mbox, (uint32_t*)msg, sizeof(Mail_t));
	return msg->id;
}






/*
 * This function initializes the SST "mail" module. It sets up Mailbox instance
 * and the GIC to receive interrupts from the Mailbox
 * @return
 *		- XST_SUCCESS if all good
 */
static int32_t mailInit()
{
	XMbox_Config *ConfigPtr;
	XMbox *MboxInstPtr = &Mbox;
	int Status;

	//get default mailbox config
	ConfigPtr = XMbox_LookupConfig(MBOX_DEVICE_ID );
	if (ConfigPtr == (XMbox_Config *)NULL)
	{
		return XST_FAILURE;
	}

	//init mailbox instance
	Status = XMbox_CfgInitialize(&Mbox, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	//Sets the Threshold
	if (MboxInstPtr->Config.UseFSL == 0)
	{
		XMbox_SetSendThreshold(MboxInstPtr, MAILBOX_SIT);
		XMbox_SetReceiveThreshold(MboxInstPtr, MAILBOX_RIT);
	}

	return Status;
}
