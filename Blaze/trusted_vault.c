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
#define AES256

/***************************** Include Files *********************************/
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include "sleep.h"  // Include the sleep header
#include "xparameters.h"
#include "xmbox.h"
#include "mb_interface.h"
#include "st_debug.h"

#include "cryptolibs/tiny-AES-c/aes.h"
#include "cryptolibs/tiny_sha3/sha3.h"
#include "cryptolibs/tiny_hmac/hmac.h"
#include "cryptolibs/tiny_hmac/sha1.h"


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

typedef struct
{
	uint8_t vaultHash[32];
	uint8_t vaultPubKey[32];
	uint8_t vaultPrvKey[32];		//

	uint8_t verifierPubKey[32];		//Given at build time

	uint8_t sessionNounce[32];
	uint8_t sessionKey[32];
}Vault_t;

typedef enum SecureTilesCommands
{
	ST_VAULT_INIT			= 0x00,
	ST_VAULT_SESSION			  ,

	ST_BIT_GET_ATTEST 		= 0x10,
	ST_BIT_SET_ENC_KEY            ,

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


//Trusted Vault
static Vault_t vaultInst;
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
    		case ST_VAULT_INIT:
    			//retComm = stComm_init();
    		break;
    		case ST_VAULT_SESSION:

    		break;
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


static int test_decrypt_cbc(void)
{

#if defined(AES256)
    uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                      0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
    uint8_t in[]  = { 0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e, 0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6,
                      0x9c, 0xfc, 0x4e, 0x96, 0x7e, 0xdb, 0x80, 0x8d, 0x67, 0x9f, 0x77, 0x7b, 0xc6, 0x70, 0x2c, 0x7d,
                      0x39, 0xf2, 0x33, 0x69, 0xa9, 0xd9, 0xba, 0xcf, 0xa5, 0x30, 0xe2, 0x63, 0x04, 0x23, 0x14, 0x61,
                      0xb2, 0xeb, 0x05, 0xe2, 0xc3, 0x9b, 0xe9, 0xfc, 0xda, 0x6c, 0x19, 0x07, 0x8c, 0x6a, 0x9d, 0x1b };
#elif defined(AES192)
    uint8_t key[] = { 0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5, 0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b };
    uint8_t in[]  = { 0x4f, 0x02, 0x1d, 0xb2, 0x43, 0xbc, 0x63, 0x3d, 0x71, 0x78, 0x18, 0x3a, 0x9f, 0xa0, 0x71, 0xe8,
                      0xb4, 0xd9, 0xad, 0xa9, 0xad, 0x7d, 0xed, 0xf4, 0xe5, 0xe7, 0x38, 0x76, 0x3f, 0x69, 0x14, 0x5a,
                      0x57, 0x1b, 0x24, 0x20, 0x12, 0xfb, 0x7a, 0xe0, 0x7f, 0xa9, 0xba, 0xac, 0x3d, 0xf1, 0x02, 0xe0,
                      0x08, 0xb0, 0xe2, 0x79, 0x88, 0x59, 0x88, 0x81, 0xd9, 0x20, 0xa9, 0xe6, 0x4f, 0x56, 0x15, 0xcd };
#elif defined(AES128)
    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t in[]  = { 0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46, 0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
                      0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee, 0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2,
                      0x73, 0xbe, 0xd6, 0xb8, 0xe3, 0xc1, 0x74, 0x3b, 0x71, 0x16, 0xe6, 0x9e, 0x22, 0x22, 0x95, 0x16,
                      0x3f, 0xf1, 0xca, 0xa1, 0x68, 0x1f, 0xac, 0x09, 0x12, 0x0e, 0xca, 0x30, 0x75, 0x86, 0xe1, 0xa7 };
#endif
    uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t out[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                      0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
                      0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
                      0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
//  uint8_t buffer[64];
    struct AES_ctx ctx;

    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_decrypt_buffer(&ctx, in, 64);


    if (0 == memcmp((char*) out, (char*) in, 64)) {
	return(0);
    } else {
	return(1);
    }
}

static int test_encrypt_cbc(void)
{
#if defined(AES256)
    uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                      0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
    uint8_t out[] = { 0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e, 0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6,
                      0x9c, 0xfc, 0x4e, 0x96, 0x7e, 0xdb, 0x80, 0x8d, 0x67, 0x9f, 0x77, 0x7b, 0xc6, 0x70, 0x2c, 0x7d,
                      0x39, 0xf2, 0x33, 0x69, 0xa9, 0xd9, 0xba, 0xcf, 0xa5, 0x30, 0xe2, 0x63, 0x04, 0x23, 0x14, 0x61,
                      0xb2, 0xeb, 0x05, 0xe2, 0xc3, 0x9b, 0xe9, 0xfc, 0xda, 0x6c, 0x19, 0x07, 0x8c, 0x6a, 0x9d, 0x1b };
#elif defined(AES192)
    uint8_t key[] = { 0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5, 0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b };
    uint8_t out[] = { 0x4f, 0x02, 0x1d, 0xb2, 0x43, 0xbc, 0x63, 0x3d, 0x71, 0x78, 0x18, 0x3a, 0x9f, 0xa0, 0x71, 0xe8,
                      0xb4, 0xd9, 0xad, 0xa9, 0xad, 0x7d, 0xed, 0xf4, 0xe5, 0xe7, 0x38, 0x76, 0x3f, 0x69, 0x14, 0x5a,
                      0x57, 0x1b, 0x24, 0x20, 0x12, 0xfb, 0x7a, 0xe0, 0x7f, 0xa9, 0xba, 0xac, 0x3d, 0xf1, 0x02, 0xe0,
                      0x08, 0xb0, 0xe2, 0x79, 0x88, 0x59, 0x88, 0x81, 0xd9, 0x20, 0xa9, 0xe6, 0x4f, 0x56, 0x15, 0xcd };
#elif defined(AES128)
    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t out[] = { 0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46, 0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
                      0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee, 0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2,
                      0x73, 0xbe, 0xd6, 0xb8, 0xe3, 0xc1, 0x74, 0x3b, 0x71, 0x16, 0xe6, 0x9e, 0x22, 0x22, 0x95, 0x16,
                      0x3f, 0xf1, 0xca, 0xa1, 0x68, 0x1f, 0xac, 0x09, 0x12, 0x0e, 0xca, 0x30, 0x75, 0x86, 0xe1, 0xa7 };
#endif
    uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    uint8_t in[]  = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                      0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
                      0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
                      0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
    struct AES_ctx ctx;

    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, in, 64);


    if (0 == memcmp((char*) out, (char*) in, 64)) {
	return(0);
    } else {
	return(1);
    }
}


int my_hmac_test()
{
	uint8_t key_bin[1024];
	  uint8_t msg_bin[1024];
	  uint8_t expected_bin[20];
	  uint8_t hmac_bin[20];
	  uint32_t key_len;
	  uint32_t msg_len;
	  uint32_t output_len;



	  hmac_sha1(key_bin, (key_len / 2), msg_bin, (msg_len / 2), hmac_bin);
}


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

	test_encrypt_cbc();
	test_decrypt_cbc();
	my_hmac_test();
	//get TrustedVault Hash
	//val = listGetSlot();
	if(val < 0)
	{
		return -2;
	}

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
