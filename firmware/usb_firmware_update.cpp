 /*
  *  usb_firmware_update.cpp - 'firmware update application. Low level'
  *
  *  Maintained by:  <support@amfeltec.com>
  *
  *  Copyright (C) 2012-2015 Amfeltec Corp.
  *
  *
  *  This program is free software; you can redistribute it and/or modify
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation; either version 2, or (at your option)
  *  any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program; see the file COPYING.  If not, write to
  *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
  */

# include <ctype.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
#include "../include/amf_usb_ioctl.h"
#include "mem.h"

#define BUFFER_SIZE         4096
#define READ_BUFFER_TIMEOUT 1000

#define PM_SIZE 1536 /* Max: 144KB/3/32=1536 PM rows for 30F. */
#define EE_SIZE 128  /* 4KB/2/16=128 EE rows */
#define CM_SIZE 8

static mem_cMemRow **AllocateFwRecords(eFamily Family);
static void FreeFwRecords(mem_cMemRow **ppMemory);
static mem_cMemRow **LoadHexFile(int fd, eFamily Family, char *filename);
static int SendHexFile(int fd, mem_cMemRow **ppMemory);

int ReceiveData (int fd, unsigned char *pBuffer, int BytesToRead);
int WriteCommBlock (int fd, unsigned char *pBuffer, int BytesToWrite);


int amf_usb_firmware_update(int fd,char *filename)
{
	int	err = 0;
	eFamily	family = dsPIC33F;

	/*parse in the data in the firmware file to memory*/
	mem_cMemRow ** ppMemory = LoadHexFile(fd, family, filename);
	if (ppMemory == NULL){
		printf("\nERROR: Failed to allocate memory\n\n");
		err=-EINVAL;
		goto upd_error_memory;
	}	

	err = SendHexFile(fd, ppMemory);

	FreeFwRecords(ppMemory);
upd_error_memory:

	return err;
}


static mem_cMemRow **AllocateFwRecords(eFamily Family)
{
	int	Row, total_len = 0, offset = 0;

	total_len = 	sizeof(mem_cMemRow *) * PM_SIZE + 
			sizeof(mem_cMemRow *) * EE_SIZE + 
			sizeof(mem_cMemRow *) * CM_SIZE;
	mem_cMemRow ** ppMemory = (mem_cMemRow **)malloc(total_len);
	if (ppMemory == NULL) return NULL;

	offset = 0;
	for(Row = 0; Row < PM_SIZE; Row++, offset++){
		ppMemory[offset] = new mem_cMemRow(mem_cMemRow::Program, 
						0x000000, 
						Row, 
						Family);
		if (ppMemory[offset] == NULL) return NULL;
	}

	for(Row = 0; Row < EE_SIZE; Row++, offset++){
		ppMemory[offset] = new mem_cMemRow(
						mem_cMemRow::EEProm, 
						0x7FF000, 
						Row, 
						Family);
		if (ppMemory[offset] == NULL) return NULL;
	}

	for(Row = 0; Row < CM_SIZE; Row++, offset++){
		ppMemory[offset] = new mem_cMemRow(
						mem_cMemRow::Configuration,
						0xF80000, 
						Row, 
						Family);
		if (ppMemory[offset] == NULL) return NULL;
	}
	return ppMemory;
}

static void FreeFwRecords(mem_cMemRow **ppMemory)
{
	int	Row;

	for(Row = 0; Row < PM_SIZE+EE_SIZE+CM_SIZE; Row++){
		if (ppMemory[Row]) delete ppMemory[Row];
	}
	if (ppMemory) free(ppMemory);
	return;
}

static mem_cMemRow **LoadHexFile(int fd, eFamily Family, char *filename)
{
	FILE		*fin = NULL;
	mem_cMemRow	**ppMemory;
	char		Buffer[BUFFER_SIZE];
	int		ExtAddr = 0, Row;
	int		cnt = 0; 

	/* Initialize Memory */
	ppMemory = AllocateFwRecords(Family);
	if (ppMemory == NULL){
		printf("ERROR: Failed to allocate memory for firmware\n");
		return NULL;		
	}

	fin = fopen(filename, "r");
	if (fin == NULL){
		printf("ERROR: Failed to open firmware file %s\n",
				filename);
		return NULL;		
	}

	printf("\nReading Amfeltec USB firmware file");
	while(fgets(Buffer, sizeof(Buffer), fin) != NULL)
	{
		int ByteCount;
		int Address;
		int RecordType;

		cnt++;
		if ((cnt % 60) == 0) {
			printf(".");
		}

		sscanf(Buffer+1, "%2x%4x%2x", &ByteCount, &Address, &RecordType);

		if(RecordType == 0)
		{
			Address = (Address + ExtAddr) / 2;
			
			for(int CharCount = 0; CharCount < ByteCount*2; CharCount += 4, Address++)
			{
				bool bInserted = 0;

				for(Row = 0; Row < (PM_SIZE + EE_SIZE + CM_SIZE); Row++)
				{
					if((bInserted = ppMemory[Row]->InsertData(Address, Buffer + 9 + CharCount)) == 1)
					{
						break;
					}
				}

				if(bInserted != 1)
				{
					printf("/nFailed (Bad Hex file)\n");
					printf("ERROR: Address 0x%x out of range\n", Address);
					goto error;
				}
			}
		}
		else if(RecordType == 1)
		{
		}
		else if(RecordType == 4)
		{
			sscanf(Buffer+9, "%4x", &ExtAddr);

			ExtAddr = ExtAddr << 16;
		}
		else
		{
			printf("/nFailed (Unknown hex record type)\n");
			goto error;
		}
	}

	for(Row = 0; Row < (PM_SIZE + EE_SIZE + CM_SIZE); Row++)
	{
		ppMemory[Row]->FormatData();
	}


	printf(" Done!\n");


	if (fin) fclose(fin);
	return ppMemory;
	
error:
	if (fin) fclose(fin);
	if (ppMemory) FreeFwRecords(ppMemory);
	return NULL;
}

static int SendHexFile(int fd, mem_cMemRow **ppMemory)
{
	int		Row = 0, err = 0;
	int cnt = 0;
	

	printf("Programming Amfeltec USB device");
	//setbuf(stdout, NULL);   // bad practise...
	fflush(stdout);			  // bad practise...


	for(Row = 0; Row < (PM_SIZE + EE_SIZE + CM_SIZE); Row++)

	{

		cnt++;
		if ((cnt % 45) == 0) {
			printf(".");
			fflush(stdout);			// bad practise...won't help anyway...
		}


		err = ppMemory[Row]->SendData(fd);
		if (err){
			switch(err){
			case -EINVAL:
				printf("Failed (Unknown memory type)\n");
				break;
			case -EBUSY:
				printf("\nFailed (Timeout:%d)!\n", Row);
				printf("ERROR: Failed to receive ACK command\n");
				break;
			case -EIO:
				break;
			default:
				printf("Failed (Unknown error code %d)\n", err);
				break;
			}
			printf("\n\n");
			err = -EINVAL;
			goto done;
		}
	}

	printf(" Done!\n");

done:
	return err;
}

int WriteCommBlock(int fd, unsigned char *pBuffer , int BytesToWrite)
{
	int	len = 0, cnt = 0;
 
	while(cnt < BytesToWrite){

		if ((cnt + 32) < BytesToWrite){
			len = 32;
		}else{
			len = BytesToWrite - cnt;
		}
		if (exec_usb_fw_data_write(fd, &pBuffer[cnt], len)){
			return -EINVAL;
		}
		cnt += len;
	}
	return 0;
}

int ReceiveData (int fd, unsigned char *pBuffer, int BytesToRead)
{
	if (exec_usb_fw_data_read(fd, pBuffer, BytesToRead)){
		return -EINVAL;
	}
	return 0;
}


int amf_usb_status_reg(int fd)
{
return 0;
}


