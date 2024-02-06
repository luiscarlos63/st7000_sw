/**Luís ..
 * This file ....
 * 01/...
 *
 */


/***************************** Include Files **********************************/

#include "st_loader.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/

// DFX Controller
XPrc Dfxc;
XPrc_Config *pDfxcConf;


/************************** Function Prototypes ******************************/

static int32_t stLoader_setDfxcBitSource(uint32_t tile_id, uint32_t *data, uint32_t size);





/************************** Function Definitions *****************************/


int32_t stLoader_init()
{
	uint32_t ret;

	//DFX controller IP init
	pDfxcConf = XPrc_LookupConfig(XDFXC_DEVICE_ID);
	if (NULL == pDfxcConf) {
	return XST_FAILURE;
	}

	ret = XPrc_CfgInitialize(&Dfxc, pDfxcConf, pDfxcConf->BaseAddress);
	if (ret != XST_SUCCESS) {
	return XST_FAILURE;
	}

	return 0;
}


int32_t stLoader_loadBit(uint32_t tile_id, uint32_t *data, uint32_t size)
{

	if(tile_id >= TILE_NUM_OF_TILES)
		return XST_FAILURE;


	stLoader_setDfxcBitSource(tile_id, data, size);

	//Send trigger to dfx controller
	if (XPrc_IsSwTriggerPending(&Dfxc, tile_id, NULL) == XPRC_NO_SW_TRIGGER_PENDING)
	{
		XPrc_SendSwTrigger(&Dfxc, tile_id, 0);
	}

	return XST_SUCCESS;
}



static int32_t stLoader_setDfxcBitSource(uint32_t tile_id, uint32_t *data, uint32_t size)
{
	//DFX controller shutdown
    XPrc_SendShutdownCommand(&Dfxc, tile_id);
	while(XPrc_IsVsmInShutdown(&Dfxc, tile_id)==XPRC_SR_SHUTDOWN_OFF);

    XPrc_SetBsSize   (&Dfxc, tile_id, 0,  size);
    XPrc_SetBsAddress(&Dfxc, tile_id, 0,  (uint32_t)data);

    XPrc_SendRestartWithNoStatusCommand(&Dfxc, tile_id);
	while(XPrc_IsVsmInShutdown(&Dfxc, tile_id)==XPRC_SR_SHUTDOWN_ON);

	return 0;
}








