#include "secure_tiles.h"
#include "ff.h"

//---------------------- For DFX Controller -----------------------------------

bit_attestation_t attestDefault = {
		.nounce = {0}
};


u32 rm_loading   	= 0;
u32 const_loading  	= 0;
u32 mult_loading	= 0;
u32 add_loading		= 0;



/////////////////////////////////////////////	Bitstream	///////////////////////////////

/*
*    Everything necessary to laod the bitstreams from SD card to DRAM.
*                        SD_card -> DRAM
*/
uint8_t bitstream_init(bitstream_t* bit, const char* file_path)
{
	int status;
	uint32_t *file_ptr;
	uint32_t file_size;

	//mount SD card
	status = SD_Init();
	if (status != SST_SUCCESS)
		return SST_FAILURE;

	//read bitstream from file in the SD card
	status = ReadFile(file_path, &file_ptr, &file_size);
	if (status != SST_SUCCESS)
		return SST_FAILURE;

	//set the bitstream struct
	bit->data = file_ptr;
	bit->size = file_size;

	//unmount SD card
	status = SD_Eject();
	if (status != SST_SUCCESS)
		return SST_FAILURE;

	return status;
}


/////////////////////////////////////////////	IP		///////////////////////////////

uint8_t ip_init_ip(ip_t* ip, uint32_t ip_id)
{
	if(!ip_id)	//0 is reserved
		return SST_FAILURE;

	ip->id = ip_id;
	return SST_SUCCESS;
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
		return SST_FAILURE;

	ip->bits[tile_id] = bit;
	return SST_SUCCESS;
}


////////////////////////////////////////////	TILE	//////////////////////////////////

/*
 * initilaizes a tile
 */
uint8_t tile_init(tile_t* til)
{
	til->ip_running = 0;
	til->status = AVAIL;
}





/*
*    initialization of dfx standalone
*    Everything necessary to get the DFX controller up and running.
*/
uint8_t sst_init(sst_t* sst)
{
	//Tile init
	tile_init(&sst->tiles[0]);
	tile_init(&sst->tiles[1]);
	tile_init(&sst->tiles[2]);

	sst->tiles[0].base_ptr = (uint32_t*)TILE_1_ADDR;
	sst->tiles[1].base_ptr = (uint32_t*)TILE_2_ADDR;
	sst->tiles[2].base_ptr = (uint32_t*)TILE_3_ADDR;

	//Mailbox init
	mailInit();

	return SST_SUCCESS;
}


/*
*    Checks operation condtion and notifies the Secure Attestor
*
*
*/
uint8_t sst_load_ip(sst_t* sst, ip_t* ip, uint32_t tile_num)
{
	int8_t ret = 0;
	int i = 0;
	uint32_t tile_id = 0;
	uint32_t actualSent = 0;
	mailMsg_t msg;


	//check if tile exists
	tile_id = tile_num - 1;
	if(tile_id >= TILE_NUM_OF_TILES)
		return SST_FAILURE;


	/*
	 * queue load request in mailbox
	 *
	 */
	mailCreateMsg(&msg, 1, (uint32_t*)ip, sizeof(ip_t));
	ret = mailSendMsg(&msg ,&actualSent);


	/*
	 * TODO: print de TESTE
	 * */
	printf("foi carregado o IP: %d, no tile: %d -- ret %d", ip->id, tile_num, ret);

	return ret;


}

uint8_t sst_get_tile_status(sst_t* sst, tile_status* status, uint32_t tile_id)
{

	if(tile_id >= TILE_NUM_OF_TILES)
		return SST_FAILURE;

	*status = sst->tiles[tile_id].status;

	return 0;
}


int8_t stt_att_nounce(bitstream_t *bit)
{

}

/*
 * FAZER COMO O TI-BLE5 FAZ PARA O GAP E O GATT. APESAR DE FAZREM PARTE DA MESMA FERRAMENTA,
 * A API FAZ SEPARA AS AS SUAS FUNCIONALIDADES.
 */

