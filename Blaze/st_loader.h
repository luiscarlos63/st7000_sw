#ifndef ST_LOADER_H
#define ST_LOADER_H


/***************************** Include Files *********************************/

#include <stdint.h>
#include <stdlib.h>
#include "xparameters.h"
#include "xprc.h"


/************************** Constant Definitions *****************************/
#define XDFXC_DEVICE_ID         XPAR_DFX_CONTROLLER_0_DEVICE_ID

#define	TILE_1_ID	0
#define TILE_2_ID 	1
#define TILE_3_ID 	2

#define	TILE_1_ADDR	0xA0000000
#define TILE_2_ADDR 0xA0010000
#define TILE_3_ADDR 0xA0020000

#define RP_NUM_OF_RP		3
#define TILE_NUM_OF_TILES	RP_NUM_OF_RP
#define DFX_MAX_NUM_OF_IP	10
#define	IP_MAX_NUM_OF_BIT	RP_NUM_OF_RP

/**************************** Type Definitions *******************************/








int32_t stLoader_init();


int32_t stLoader_loadBit(uint32_t tile_id, uint32_t *data, uint32_t size);











#endif //ST_LOADER_H




