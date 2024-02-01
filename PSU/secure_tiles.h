#ifndef SECURE_TILES_H
#define SECURE_TILES_H

/***************************** Include Files **********************************/

#include <stdint.h>
#include <stdlib.h>
#include "sst_mail.h"

#include "xmbox.h"
#include "xstatus.h"
#include "xparameters.h"
 #include "xscugic.h"
#include "xil_exception.h"
#include "xil_cache.h"
/************************** Constant Definitions *****************************/


#define	TILE_1_ADDR	0xA0010000
#define TILE_2_ADDR 0xA0020000
#define TILE_3_ADDR 0xA0030000


// Hardware definitions
#define MBOX_DEVICE_ID		XPAR_MBOX_0_DEVICE_ID
#define MBOX_INTR_ID		XPAR_FABRIC_MBOX_0_VEC_ID

#define ST_MAIL_ID			MBOX_DEVICE_ID
#define ST_MAIL_INTR_ID		MBOX_INTR_ID


/**************************** Type Definitions *******************************/

typedef struct SecureTilesStruct *SecTilesHandle;


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
	uint64_t tile;		//reconfigurable partition id
	uint64_t rm;		//reconfigurable module id
	uint32_t *data;
	uint32_t size;
    bit_attestation_t attest;
}Bitstream_t;

typedef struct SecureTilesStruct
{
	uint32_t status;
};

typedef enum SecureTilesCommands
{
	ST_BIT_GET_ATTEST= 0x10,
	ST_BIT_SET_ENC_KEY,

	ST_LOADER_BIT		= 0x20,

	ST_TILE_STATUS 		= 0x30
}SecTilesComm_t;

typedef struct{
	uint32_t type;
	uint32_t msgId;
	uint32_t *pMsg;
	uint32_t msgLen;
}MailMsg_t;


/*
 *
 */
int32_t bit_init(Bitstream_t* bit);

/*
 *  registers the bitstream binary data
 */
int32_t bit_binFileReg(Bitstream_t* bit, uint32_t *buf, uint32_t lenght);




/*
*    Initialization of Secure Tiles in standalone
*
*/
int32_t st_init();

/*
*
*/
int32_t st_command(SecTilesComm_t type, void *comm, );




int32_t mailInit();

int32_t mailRecvMsg(u32 *BufferPtr, u32 RequestedBytes, u32 *BytesRecvdPtr);

int32_t mailSendMsg(mailMsg_t *msg,uint32_t *actualSent);

int32_t mailCreateMsg(mailMsg_t *msg, uint32_t type, uint32_t *pMsg, uint32_t msgLen);







#endif // SECURE_TILES_H
