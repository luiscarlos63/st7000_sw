#ifndef SECURE_TILES_H
#define SECURE_TILES_H

/***************************** Include Files **********************************/

#include <stdint.h>
#include <stdlib.h>
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
	uint64_t tileId;		//reconfigurable partition id
	uint64_t ipId;			//reconfigurable module id
	uint32_t *data;
	uint32_t size;
	BitAttestation_t attest;
}Bitstream_t;



typedef enum SecureTilesCommands
{
	ST_BIT_GET_ATTEST= 0x10,
	ST_BIT_SET_ENC_KEY,

	ST_LOADER_BIT		= 0x20,

	ST_TILE_STATUS 		= 0x30
}SecTilesComm_t;


/*
 *
 */
int32_t bit_init(Bitstream_t* bit);

/*
 *  registers the bitstream's binary data
 */
int32_t
bit_binFileReg(	Bitstream_t* bit, uint32_t *buf, uint32_t lenght,
				uint32_t ipId, uint32_t tileId);


/*
*    Initialization of Secure Tiles in standalone
*
*/
int32_t st_init();

/*
*
*/
int32_t st_command(SecTilesComm_t type, void *msgContent);









#endif // SECURE_TILES_H
