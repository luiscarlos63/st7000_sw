/**
 *
 */
#ifndef __SST_MAIL_H_
#define __SST_MAIL_H_









/***************************** Include Files *********************************/
#include "xmbox.h"
#include "xstatus.h"
#include "xparameters.h"
 #include "xscugic.h"
#include "xil_exception.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/
/**
 * Para teste de interrupções
 */
#define MBOX_DEVICE_ID		XPAR_MBOX_0_DEVICE_ID
#define MBOX_INTR_ID		XPAR_FABRIC_MBOX_0_VEC_ID

/**************************** Type Definitions *******************************/
typedef struct{
	uint32_t type;
	uint32_t msgId;
	uint32_t *pMsg;
	uint32_t msgLen;
}mailMsg_t;


/************************** Variable Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int mailInit();

int32_t mailRecvMsg(u32 *BufferPtr, u32 RequestedBytes, u32 *BytesRecvdPtr);

int8_t mailSendMsg(mailMsg_t *msg,uint32_t *actualSent);

int8_t mailCreateMsg(mailMsg_t *msg, uint32_t type, uint32_t *pMsg, uint32_t msgLen);

/* xxxxx in sst_mail.c */
















#endif
