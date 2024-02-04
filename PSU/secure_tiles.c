/**
 * Valetinho
 */

/***************************** Include Files **********************************/

#include "secure_tiles.h"
#include "ff.h"



/************************** Constant Definitions *****************************/
#define MAILBOX_RIT	4	/* mailbox receive interrupt threshold */
#define MAILBOX_SIT	4	/* mailbox send interrupt threshold */

#define FROM_HOST	0
#define FROM_VAULT	1
/**************************** Type Definitions *******************************/

typedef struct
{
	SecTilesComm_t type;
	uint32_t *content;
	uint32_t src;
	uint32_t id;
}Mail_t;
/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/
// Bitstream
Bitstream_t defaultBitVal =
{
	.tileId = 0xFFFFFFFF,
	.ipId	= 0xFFFFFFFF,
	.data	= NULL,
	.size	= 0,
};

// Mailbox
static XMbox Mbox;
static int32_t msgIdCount = 0;

//TrustedVault


/************************** Function Prototypes ******************************/
static int32_t mailInit();
static int32_t mailBuildMsg(Mail_t *msg, SecTilesComm_t type, void *msgContent, uint32_t src);
static int32_t mailSendMsg(Mail_t *msg);
static int32_t mailRecvMsg(Mail_t *msg);




/************************** Function Definitions *****************************/





/*
 *
 */
int32_t bit_init(Bitstream_t* bit)
{
	return memcpy(bit, &defaultBitVal, sizeof(Bitstream_t));
}

/*
 *  registers the bitstream's binary data
 */
int32_t
bit_binFileReg(	Bitstream_t* bit, uint32_t *buf, uint32_t lenght,
				uint32_t ipId, uint32_t tileId)
{
	if(bit == NULL)
	{
		return 1;
	}
	if(buf == NULL)
	{
		return 2;
	}
	if(lenght == 0)
	{
		return 3;
	}

	bit->data 	= buf;
	bit->size 	= lenght;
	bit->tileId = tileId;
	bit->ipId 	= ipId;

	return 0;
}




/*
*    Initialization of Secure Tiles in standalone
*
*/
int32_t st_init()
{
	//Mailbox init
	mailInit();

	/*
	 * TODO: fazer um echo com o TrustedVault
	 */
	//send dummy

	//recv dummy

	return 0;
}




/*
*	Prepares the command based on the type and the pointer to the
*	container
*/
int32_t st_command(SecTilesComm_t type, void *msgContent)
{
	Mail_t msg;
	int32_t retSend = 0;
	int32_t retRecv = 0;

	mailBuildMsg(&msg, type, msgContent, FROM_HOST);

	/*
	 * send the command
	 */
	retSend = mailSendMsg(&msg);
	if(retSend < 0)
	{
		return 1;
	}

	/*
	 * Wait for the responce
	 */
	retRecv = mailRecvMsg(&msg);
	if(retRecv < 0)
	{
		return 2;
	}

	if(retSend != retRecv)
	{
		return 3;
	}

	return 0;
}




static int32_t
mailBuildMsg(Mail_t *msg, SecTilesComm_t type, void *msgContent, uint32_t src)
{
	msg->type = type;
	msg->content = (uint32_t*)msgContent;
	msg->src = src;
	msg->id = msgIdCount++;

	return msg->id;
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

	//Interrupts enable
	if (MboxInstPtr->Config.UseFSL == 0)
	{
		XMbox_SetInterruptEnable(MboxInstPtr, 	XMB_IX_STA |
												XMB_IX_RTA |
												XMB_IX_ERR);
	}

	return Status;
}

