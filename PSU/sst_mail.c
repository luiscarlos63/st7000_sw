/*****************************************************************************/


/***************************** Include Files **********************************/
#include "sst_mail.h"



/************************** Constant Definitions *****************************/
#define MAILBOX_RIT	4	/* mailbox receive interrupt threshold */
#define MAILBOX_SIT	4	/* mailbox send interrupt threshold */


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/
static XMbox Mbox;

static uint32_t msgIdCount = 0;


/************************** Function Prototypes ******************************/




/************************** Function Definitions *****************************/


int8_t mailCreateMsg(mailMsg_t *msg, uint32_t type, uint32_t *pMsg, uint32_t msgLen)
{
	msg->pMsg 	= pMsg;
	msg->type 	= type;
	msg->msgLen	= msgLen;
	msg->msgId 	= msgIdCount++;

	return 0;
}


int8_t mailSendMsg(mailMsg_t *msg, uint32_t *actualSent)
{
	int ret;

	ret = XMbox_Write(&Mbox, msg->pMsg, msg->msgLen, actualSent);

	return ret;
}

int32_t mailRecvMsg(u32 *BufferPtr, u32 RequestedBytes, u32 *BytesRecvdPtr)
{
	return XMbox_Read(&Mbox, BufferPtr, RequestedBytes, BytesRecvdPtr);
}


/*****************************************************************************/
/**
* This function initializes the SST "mail" module. It sets up Mailbox instance
* and the GIC to receive interrupts from the Mailbox
* @return
*		- XST_SUCCESS if all good
*****************************************************************************/
int mailInit()
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










