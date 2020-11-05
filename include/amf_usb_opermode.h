 /*
  *  amf_usb_opermode.h - 'FXO operating modes definitions'
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


#ifndef __AMF_USB_OPERMODE_H__
#define __AMF_USB_OPERMODE_H__

static struct fxo_mode {
	char *name;
	/* FXO */
	int ohs;
	int ohs2;
	int rz;
	int rt;
	int ilim;
	int dcv;
	int mini;
	int acim;
	int ring_osc;
	int ring_x;
} fxo_modes[] =
{
	{ "FCC", 0, 0, 0, 1, 0, 0x3, 0, 0, }, 	/* US, Canada */
	{ "TBR21", 0, 0, 0, 0, 1, 0x3, 0, 0x2, 0x7e6c, 0x023a, },
		/* Austria, Belgium, Denmark, Finland, France, Germany,
		   Greece, Iceland, Ireland, Italy, Luxembourg, Netherlands,
		   Norway, Portugal, Spain, Sweden, Switzerland, and UK */
	{ "ARGENTINA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "AUSTRALIA", 1, 0, 0, 0, 0, 0, 0x3, 0x3, },
	{ "AUSTRIA", 0, 1, 0, 0, 1, 0x3, 0, 0x3, },
	{ "BAHRAIN", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "BELGIUM", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "BRAZIL", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "BULGARIA", 0, 0, 0, 0, 1, 0x3, 0x0, 0x3, },
	{ "CANADA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "CHILE", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "CHINA", 0, 0, 0, 0, 0, 0, 0x3, 0xf, },
	{ "COLUMBIA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "CROATIA", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "CYPRUS", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "CZECH", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "DENMARK", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ECUADOR", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "EGYPT", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "ELSALVADOR", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "FINLAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "FRANCE", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "GERMANY", 0, 1, 0, 0, 1, 0x3, 0, 0x3, },
	{ "GREECE", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "GUAM", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "HONGKONG", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "HUNGARY", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "ICELAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "INDIA", 0, 0, 0, 0, 0, 0x3, 0, 0x4, },
	{ "INDONESIA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "IRELAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ISRAEL", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ITALY", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "JAPAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "JORDAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "KAZAKHSTAN", 0, 0, 0, 0, 0, 0x3, 0, },
	{ "KUWAIT", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "LATVIA", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "LEBANON", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "LUXEMBOURG", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "MACAO", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "MALAYSIA", 0, 0, 0, 0, 0, 0, 0x3, 0, },	/* Current loop >= 20ma */
	{ "MALTA", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "MEXICO", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "MOROCCO", 0, 0, 0, 0, 1, 0x3, 0, 0x2, },
	{ "NETHERLANDS", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "NEWZEALAND", 0, 0, 0, 0, 0, 0x3, 0, 0x4, },
	{ "NIGERIA", 0, 0, 0, 0, 0x1, 0x3, 0, 0x2, },
	{ "NORWAY", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "OMAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "PAKISTAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "PERU", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "PHILIPPINES", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "POLAND", 0, 0, 1, 1, 0, 0x3, 0, 0, },
	{ "PORTUGAL", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "ROMANIA", 0, 0, 0, 0, 0, 3, 0, 0, },
	{ "RUSSIA", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "SAUDIARABIA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "SINGAPORE", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "SLOVAKIA", 0, 0, 0, 0, 0, 0x3, 0, 0x3, },
	{ "SLOVENIA", 0, 0, 0, 0, 0, 0x3, 0, 0x2, },
	{ "SOUTHAFRICA", 1, 0, 1, 0, 0, 0x3, 0, 0x3, },
	{ "SOUTHKOREA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "SPAIN", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "SWEDEN", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "SWITZERLAND", 0, 1, 0, 0, 1, 0x3, 0, 0x2, },
	{ "SYRIA", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "TAIWAN", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "THAILAND", 0, 0, 0, 0, 0, 0, 0x3, 0, },
	{ "UAE", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "UK", 0, 1, 0, 0, 1, 0x3, 0, 0x5, },
	{ "USA", 0, 0, 0, 0, 0, 0x3, 0, 0, },
	{ "YEMEN", 0, 0, 0, 0, 0, 0x3, 0, 0, },
};


#endif  //__AMF_USB_OPERMODE_H__
