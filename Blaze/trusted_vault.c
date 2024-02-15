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
#include "mb_interface.h"

#include "crypto/ed25519.h"


/************************** Constant Definitions *****************************/
// Hardware definitions
/* --- Mailbox --- */
#define MBOX_DEVICE_ID		XPAR_MBOX_0_DEVICE_ID
#define MBOX_INTR_ID		XPAR_FABRIC_MBOX_0_VEC_ID

#define ST_MAIL_ID			MBOX_DEVICE_ID
#define ST_MAIL_INTR_ID		MBOX_INTR_ID

#define MAILBOX_RIT	4	/* mailbox receive interrupt threshold */
#define MAILBOX_SIT	4	/* mailbox send interrupt threshold */

#define FROM_HOST	0
#define FROM_VAULT	1

/* --- Stream -
 * AES key deploy ---*/
#define KEY_NEW_ADDR 	GPO4
#define KEY_STREAM_ID 	0
#define KEY_IV_ID 		1

#define DFXC_VS1_STATUS 0
#define DFXC_VS2_STATUS 1
#define DFXC_VS3_STATUS 2

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
	uint8_t status;
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
VaultBit Bitlist[16];

/************************** Function Prototypes ******************************/

//Commands
static int32_t stComm_getAttest(Bitstream_t *bit);

// Stream
static void inline write_key(volatile unsigned int *a);
static void inline write_IV(volatile unsigned int *a);
static void inline read_axis_dfxc_status(volatile unsigned int *a, uint8_t id);
// Mailbox
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
				retComm = stComm_getAttest((Bitstream_t*)msgComm.content);
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
	static int32_t val = 0;

	if(val == 0)
		val = 1;
	else
		val = 0;

	stDebug_ioPsGpioSetLED(PS_LED_4, val);

	//verifications
	if(bit == NULL)
	{
		return ST_ERROR_TYPE_INV;
	}

	//generate nouce and store it
    unsigned char public_key[32], private_key[64], seed[32], scalar[32];
    unsigned char other_public_key[32], other_private_key[64];
    unsigned char shared_secret[32], other_shared_secret[32];
    unsigned char signature[64];
    const unsigned char message[] = "Hello, world!";
    const int message_len = strlen((char*) message);

    val = ed25519_create_seed(seed);
    ed25519_create_keypair(public_key, private_key, seed);
    ed25519_sign(signature, message, message_len, public_key, private_key);
    val = ed25519_verify(signature, message, message_len, public_key);

    /* create two shared secrets - from both perspectives - and check if they're equal */
    ed25519_key_exchange(shared_secret, other_public_key, private_key);
    ed25519_key_exchange(other_shared_secret, public_key, other_private_key);

	//get TrustedVault Hash
	//val = listGetSlot();
	if(val < 0)
	{
		return -2;
	}

	Bitlist[val].bit = bit;
	//bit->attest.nonce = 0;		//nounce



	return 0;
}




/** ------ Stream -------*/
/*
 * Write 8 32-bit words as efficiently as possible.
 */
static void inline write_key(volatile unsigned int *a)
{
    register int a0,  a1,  a2,  a3;
    register int a4,  a5,  a6,  a7;


    a3  = a[3];  a1  = a[1];  a2  = a[2];  a0  = a[0];
    a7  = a[7];  a5  = a[5];  a6  = a[6];  a4  = a[4];

    putfsl(a0,  KEY_STREAM_ID); putfsl(a1,  KEY_STREAM_ID); putfsl(a2,  KEY_STREAM_ID); putfsl(a3,  KEY_STREAM_ID);
    putfsl(a4,  KEY_STREAM_ID); putfsl(a5,  KEY_STREAM_ID); putfsl(a6,  KEY_STREAM_ID); putfsl(a7,  KEY_STREAM_ID);

}

/*
 * Write 4 32-bit words as efficiently as possible.
 */
static void inline write_IV(volatile unsigned int *a)
{
    register int a0,  a1,  a2,  a3;

    a3  = a[3];  a1  = a[1];  a2  = a[2];  a0  = a[0];

    putfsl(a0,  KEY_IV_ID); putfsl(a1,  KEY_IV_ID); putfsl(a2,  KEY_IV_ID); putfsl(a3,  KEY_IV_ID);
}

/*
 * Read 16 32-bit words as efficiently as possible.
 * Deprecated - The Microblaze cpu will use the AXI_MM interface instead.
 */
static void inline read_axis_dfxc_status(volatile unsigned int *a, uint8_t id)
{
    register int a0;

    getfsl(a0,  0);

    a[0]  = a0;
}

/* ------ Mail ----------*/
/**
 * Send a message to PSU
 * Blocking
 */
static int32_t mailSendMsg(Mail_t *msg)
{
	XMbox_WriteBlocking(&Mbox, (uint32_t*)msg, sizeof(Mail_t));
	return msg->id;
}

/**
 * Receive a message from PSU
 * Blocking
 */
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
