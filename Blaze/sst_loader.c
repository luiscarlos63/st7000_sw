#include "sst_loader.h"

//---------------------- For DFX Controller -----------------------------------

u32 rm_loading   	= 0;
u32 const_loading  	= 0;
u32 mult_loading	= 0;
u32 add_loading		= 0;


/////////////////////////////////////////////	Bitstream	///////////////////////////////


/////////////////////////////////////////////	IP		///////////////////////////////

uint8_t ip_init_ip(ip_t* ip, uint32_t ip_id)
{
	if(!ip_id)	//0 is reserved
		return XST_FAILURE;

	ip->id = ip_id;
	return XST_SUCCESS;
}

/*
 * add a bitstream to an IP
 * @ ip			- IP instance
 * @ bit 		- Bitstream to associate
 * @ tile_id	- Tile to which the bit is associated
 */
uint8_t ip_add_bitstream(ip_t* ip, bitstream_t* bit, uint32_t tile_id)
{
	if(tile_id >= TILE_NUM_OF_TILES)
		return XST_FAILURE;

	ip->bits[tile_id] = bit;
	return XST_SUCCESS;
}


////////////////////////////////////////////	TILE	//////////////////////////////////

/*
 * initilaizes a tile
 */
uint8_t tile_init(tile_t* til)
{
	til->ip_running = 0;
	til->status = AVAIL;

	return XST_SUCCESS;
}
//////////////////////////////////////////////	DFX //////////////////////////////////
/*
 * Sets the ADDRESS and SIZE of the
 */

uint8_t dfx_set_bitstream_source(dfx_t* dfx, uint32_t tile_id, bitstream_t* bit)
{
	//DFX controller shutdown
    XPrc_SendShutdownCommand(&dfx->dfx_ctrl, tile_id);
	while(XPrc_IsVsmInShutdown(&dfx->dfx_ctrl, tile_id)==XPRC_SR_SHUTDOWN_OFF);

    XPrc_SetBsSize   (&dfx->dfx_ctrl, tile_id, 0,  bit->size);
    XPrc_SetBsAddress(&dfx->dfx_ctrl, tile_id, 0,  (uint32_t)bit->data);

    XPrc_SendRestartWithNoStatusCommand(&dfx->dfx_ctrl, tile_id);
	while(XPrc_IsVsmInShutdown(&dfx->dfx_ctrl, tile_id)==XPRC_SR_SHUTDOWN_ON);

	return XST_SUCCESS;
}

/*
*    initialization of dfx standalone
*    Everything necessary to get the DFX controller up and running.
*/
uint8_t dfx_init(dfx_t* dfx)
{
	uint32_t prc_init;

	//DFX controller IP init
    dfx->dfx_conf = XPrc_LookupConfig(XDFXC_DEVICE_ID);
	if (NULL == dfx->dfx_conf) {
	return XST_FAILURE;
	}

	prc_init = XPrc_CfgInitialize(&dfx->dfx_ctrl, dfx->dfx_conf,dfx->dfx_conf->BaseAddress);
	if (prc_init != XST_SUCCESS) {
	return XST_FAILURE;
	}


	//Tile init
	tile_init(&dfx->tiles[0]);
	tile_init(&dfx->tiles[1]);
	tile_init(&dfx->tiles[2]);

	dfx->tiles[0].base_ptr = (uint32_t*)TILE_1_ADDR;
	dfx->tiles[1].base_ptr = (uint32_t*)TILE_2_ADDR;
	dfx->tiles[2].base_ptr = (uint32_t*)TILE_3_ADDR;

	return XST_SUCCESS;
}


/*
*    Every operation necessary to load the bitstream to the RP
*                        DRAM -> RP
*                           DFX_trigger
*/
uint8_t dfx_load_ip(dfx_t* dfx, ip_t* ip, uint32_t tile_num)
{
	uint32_t tile_id = 0;

	//API asks gets the TILE in a range from 1 - N, but
	tile_id = tile_num - 1;
	if(tile_id >= TILE_NUM_OF_TILES)
		return XST_FAILURE;

	//set the "to be run" bitstream info
	dfx_set_bitstream_source( dfx, 					//dfx instance
							  tile_id,				//choosen tile instance
							  ip->bits[tile_id]);	//ip to be run	&   in the corresponding tile

	dfx->tiles[tile_id].ip_running = ip->id;

	//sent the ATTEST module the "to be run" bitstream info (for future)
	/*
	 *	send_ip_id
	 *	send_tile_ip
	 */

	//Send trigger to dfx controller
	if (XPrc_IsSwTriggerPending(&dfx->dfx_ctrl, tile_id, NULL)==XPRC_NO_SW_TRIGGER_PENDING)
	{
	    XPrc_SendSwTrigger(&dfx->dfx_ctrl, tile_id, 0);
	}

	dfx->tiles[tile_id].status = ACTIVE;
	return XST_SUCCESS;
}

uint8_t dfx_get_tile_status(dfx_t* dfx, tile_status* status, uint32_t tile_id)
{

	if(tile_id >= TILE_NUM_OF_TILES)
		return XST_FAILURE;

	*status = dfx->tiles[tile_id].status;
	return XST_SUCCESS;
}


