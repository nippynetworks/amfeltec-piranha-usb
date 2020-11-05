 /*
  *  mem.h - 'firmwqre memory access API definitions'
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

#ifndef _mem_h
#define _mem_h


#define COMMAND_NACK     0x00
#define COMMAND_ACK      0x01
#define COMMAND_READ_PM  0x02
#define COMMAND_WRITE_PM 0x03
#define COMMAND_READ_EE  0x04
#define COMMAND_WRITE_EE 0x05
#define COMMAND_READ_CM  0x06
#define COMMAND_WRITE_CM 0x07
#define COMMAND_RESET    0x08
#define COMMAND_READ_ID  0x09
#define COMMAND_READ_STATUS_REG  0x0A

#define PM30F_ROW_SIZE 32
#define PM33F_ROW_SIZE 64*8
#define EE30F_ROW_SIZE 16

enum eFamily
{
	dsPIC30F,
	dsPIC33F,
	PIC24H,
	PIC24F
};


class mem_cMemRow
{
public:

	enum eType
	{
		Program,
		EEProm,
		Configuration
	};
	mem_cMemRow(eType Type, unsigned int StartAddr, int RowNumber, eFamily Family);

	int InsertData(unsigned int Address, char * pData);
	int FormatData(void);
	int SendData  (int fd);

private:
	unsigned char	*m_pBuffer;
	unsigned int	m_Address;
	int		m_bEmpty;
	eType		m_eType;
	unsigned short	m_Data[PM33F_ROW_SIZE*2];
	int		m_RowNumber;
	eFamily		m_eFamily;
	int		m_RowSize;
};


#endif
