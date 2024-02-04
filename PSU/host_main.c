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


#include <stdio.h>
#include <stdint.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_io.h"
#include <xstatus.h>
#include "xil_cache.h"
#include <xil_types.h>
#include <sleep.h>

#include "secure_tiles.h"
#include "xil_exception.h"
#include "xgpiops.h"
#include "ff.h"

/************************** Variable Definitions *****************************/

static FATFS  fatfs;

const char* ip_const33_rp_1 = "rp1_33.bin";
const char* ip_const44_rp_1 = "rp1_44.bin";
const char* ip_const55_rp_1 = "rp1_55.bin";
const char* ip_const33_rp_2 = "rp2_33.bin";
const char* ip_const44_rp_2 = "rp2_44.bin";
const char* ip_const55_rp_2 = "rp2_55.bin";
const char* ip_const33_rp_3 = "rp3_33.bin";
const char* ip_const44_rp_3 = "rp3_44.bin";
const char* ip_const55_rp_3 = "rp3_55.bin";


XGpioPs Gpio;
/************************** Function Prototypes ******************************/
static int SD_Init();
static int SD_Eject();
static int SD_ReadFile(const char *FileName, u32** DestinationAddress, u32* size);



/************************** Function Definitions *****************************/
int main()
{
	int32_t status = 0;

	Bitstream_t bit1;

	uint32_t *pBitBuffAux = NULL;
	uint32_t BitSizeAux = 0;

    init_platform();
    disable_caches();
    status = SD_Init();
    status = st_init();

    status = bit_init(&bit1);
    status = SD_ReadFile(ip_const33_rp_1, &pBitBuffAux, &BitSizeAux);
    status = bit_binFileReg(&bit1, pBitBuffAux, BitSizeAux, 1, 1);

	print("\nBitstreams loaded into memory ");
	print("\nIPs created");

    //Secure initialization


    /*
     * Bitstreams attestation process
     */
    //request attestation parameters
	status = st_command(ST_BIT_GET_ATTEST, &bit1);

    //send


    while(1)
    {
    	usleep(1000);
    }

    SD_Eject();
    cleanup_platform();
    return 0;
}









static int SD_Init()
{
	FRESULT rc;
	TCHAR *Path = "0:/";
	rc = f_mount(&fatfs,Path,0);
	if (rc) {
		//xil_printf(" ERROR : f_mount returned %d\r\n", rc);
		return 1;
	}
	return 0;
}

static int SD_Eject()
{
	FRESULT rc;
	TCHAR *Path = "0:/";
	rc = f_mount(0,Path,1);
	if (rc) {
		//xil_printf(" ERROR : f_mount returned %d\r\n", rc);
		return 1;
	}
	return 0;
}



static int SD_ReadFile(const char *FileName, u32** DestinationAddress, u32* size)
{
	FIL fil;
	FRESULT rc;
	UINT br;
	u32 file_size;
	rc = f_open(&fil, FileName, FA_READ);
	if (rc) {
//		xil_printf(" ERROR : f_open returned %d\r\n", rc);
		return 1;
	}
	file_size = f_size(&fil);	//file_size = fil.fsize;
	*size = file_size;

	//space allocation
	*DestinationAddress = (u32 *)malloc(file_size * sizeof(char)); // Enough memory for the file

	rc = f_lseek(&fil, 0);
	if (rc) {
//		xil_printf(" ERROR : f_lseek returned %d\r\n", rc);
		return 2;
	}
	rc = f_read(&fil, (void**) *DestinationAddress, file_size, &br);
	if (rc) {
//		xil_printf(" ERROR : f_read returned %d\r\n", rc);
		return 3;
	}
	rc = f_close(&fil);
	if (rc) {
//		xil_printf(" ERROR : f_close returned %d\r\n", rc);
		return 4;
	}
	Xil_DCacheFlush();
	return 0;
}
