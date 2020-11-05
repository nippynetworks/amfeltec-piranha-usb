 /*
  *  mem.cpp - 'firmware memory API'
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "mem.h"

extern int   WriteCommBlock (int fd, unsigned char *pBuffer ,  int BytesToWrite);
extern int   ReceiveData (int fd, unsigned char *pBuffer ,  int BytesToReceive);

/******************************************************************************/
mem_cMemRow::mem_cMemRow(eType Type, unsigned int StartAddr, int RowNumber, eFamily Family)
{
	int Size = 0;

	m_RowNumber = RowNumber;
	m_eFamily    = Family;
	m_eType     = Type;
	m_bEmpty    = 1;

	if(m_eType == Program)
	{
		if(m_eFamily == dsPIC30F)
		{
			m_RowSize = PM30F_ROW_SIZE;
		}
		else
		{
			m_RowSize = PM33F_ROW_SIZE;
		}
	}
	else
	{
		m_RowSize = EE30F_ROW_SIZE;
	}

	if(m_eType == Program)
	{
		Size = m_RowSize * 4;
		m_Address = StartAddr + RowNumber * m_RowSize * 2;
	}
	if(m_eType == EEProm)
	{
		Size = m_RowSize * 2;
		m_Address = StartAddr + RowNumber * m_RowSize * 2;
	}
	if(m_eType == Configuration)
	{
		Size = 4;
		m_Address = StartAddr + RowNumber * 2;
	}

	m_pBuffer   = (unsigned char *)malloc(Size);

	memset(m_Data, 0xFFFF, sizeof(unsigned short)*PM33F_ROW_SIZE*2);	
}
/******************************************************************************/
int mem_cMemRow::InsertData(unsigned int Address, char * pData)
{
	if(Address < m_Address)
	{
		return 0;
	}

	if((m_eType == Program) && (Address >= (m_Address + m_RowSize * 2)))
	{
		return 0;
	}

	if((m_eType == EEProm) && (Address >= (m_Address + m_RowSize * 2)))
	{
		return 0;
	}

	if((m_eType == Configuration) && (Address >= (m_Address + 2)))
	{
		return 0;
	}

	m_bEmpty    = 0;

	sscanf(pData, "%4hx", &(m_Data[Address - m_Address]));
	
	return 1;
}
/******************************************************************************/
int mem_cMemRow::FormatData(void)
{
	if(m_bEmpty == 1)
	{
		return 0;
	}

	if(m_eType == Program)
	{
		for(int Count = 0; Count < m_RowSize; Count += 1)
		{
			m_pBuffer[0 + Count * 4] = (m_Data[Count * 2]     >> 8) & 0xFF;
			m_pBuffer[1 + Count * 4] = (m_Data[Count * 2])          & 0xFF;
			m_pBuffer[2 + Count * 4] = (m_Data[Count * 2 + 1] >> 8) & 0xFF;

			m_pBuffer[3 + Count * 4] = (m_Data[Count * 2 + 1]) & 0xFF;
		}
	}
	else if(m_eType == Configuration)
	{
		m_pBuffer[0] = (m_Data[0]  >> 8) & 0xFF;
		m_pBuffer[1] = (m_Data[0])       & 0xFF;
		m_pBuffer[2] = (m_Data[1]  >> 8) & 0xFF;

		m_pBuffer[3] = (m_Data[1]) & 0xFF;
	}
	else
	{
		for(int Count = 0; Count < m_RowSize; Count++)
		{
			m_pBuffer[0 + Count * 2] = (m_Data[Count * 2] >> 8) & 0xFF;
			m_pBuffer[1 + Count * 2] = (m_Data[Count * 2])      & 0xFF;
		}
	}
	return 0;
}
/******************************************************************************/
int mem_cMemRow::SendData(int fd)
{
	unsigned char	Buffer[4] = {0,0,0,0};
	static int counter = 0;

	if((m_bEmpty == 1) && (m_eType != Configuration))
	{
		return 0;
	}

	if(m_eType == Program)
	{

		Buffer[0] = COMMAND_WRITE_PM;
		Buffer[1] = (m_Address)       & 0xFF;
		Buffer[2] = (m_Address >> 8)  & 0xFF;
		Buffer[3] = (m_Address >> 16) & 0xFF;

		//printf("...WRITING Program 1: %02X:%02X:%02X:%02X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);					

		if (WriteCommBlock(fd, Buffer, 4)) {
			return -EIO;
		} else {
		}

		sleep(1);		

		counter++;

		//printf("...WRITING Program 2: %02X:%02X:%02X:%02X...\n", m_pBuffer[0], m_pBuffer[1], m_pBuffer[2], m_pBuffer[3]);					

		if (WriteCommBlock(fd, m_pBuffer, m_RowSize * 4)) {
			printf("\n...INSIDE %s... Program, after WriteCommBlock2: EIO!...\n", __FUNCTION__);					
			return -EIO;
		} else {
		}

	}
	else if(m_eType == EEProm)
	{

		Buffer[0] = COMMAND_WRITE_EE;
		Buffer[1] = (m_Address)       & 0xFF;
		Buffer[2] = (m_Address >> 8)  & 0xFF;
		Buffer[3] = (m_Address >> 16) & 0xFF;

		//printf("...WRITING EEPROM 1: %02X:%02X:%02X:%02X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);					

		if (WriteCommBlock(fd, Buffer, 4)) return -EIO;

		//printf("...WRITING EEPROM 2: %02X:%02X:%02X:%02X...\n", m_pBuffer[0], m_pBuffer[1], m_pBuffer[2], m_pBuffer[3]);					


		if (WriteCommBlock(fd, m_pBuffer, m_RowSize * 2)) return -EIO;
	}
	else if((m_eType == Configuration) && (m_RowNumber == 0))
	{

		Buffer[0] = COMMAND_WRITE_CM;
		Buffer[1] = (char)(m_bEmpty)& 0xFF;
		Buffer[2] = m_pBuffer[0];
		Buffer[3] = m_pBuffer[1];

		//printf("\n...WRITING Config, RowNum=%d: %02X:%02X:%02X:%02X\n", m_RowNumber, Buffer[0], Buffer[1], Buffer[2], Buffer[3]);					

		if (WriteCommBlock(fd, Buffer, 4)) {
			return -EIO;
		} else {
		}			
	}
	else if((m_eType == Configuration) && (m_RowNumber != 0))
	{

		if((m_eFamily == dsPIC30F) && (m_RowNumber == 7))
		{
			return 0;
		}
		Buffer[0] = (char)(m_bEmpty)& 0xFF;
		Buffer[1] = m_pBuffer[0];
		Buffer[2] = m_pBuffer[1];

		//printf("...WRITING Config, RowNum=%d: %02X:%02X:%02X\n", m_RowNumber, Buffer[0], Buffer[1], Buffer[2]);					

		if (WriteCommBlock(fd, Buffer, 3)) {
			return -EIO;
		} else {
		}
	}
	else
	{
		return -EINVAL;
	}

	sleep(1);
	
	if (ReceiveData(fd, Buffer, 1)) {

		printf("SendData: No Answer (1Byte): m_eType=%d, m_RowNumber=%d...\n", m_eType, m_RowNumber);
		return -EIO;
	} else {

		//printf("ReceiveData: m_eType=%d, m_RowNumber=%d, Buffer[0]=%02X...\n", m_eType, m_RowNumber, Buffer[0]);

	}

	if (Buffer[0] != COMMAND_ACK) {

		printf("SendData: Not ACK (1Byte): m_eType=%d, m_RowNumber=%d, Buff0=%X...\n", m_eType, m_RowNumber, Buffer[0]);
		return -EBUSY;
	}

	return 0;
}
