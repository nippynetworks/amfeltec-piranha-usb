 /*
  *  amf_usb_daa.c - 'daa related functions'
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

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

#include "amf_usb.h"
#include "amf_usb_opermode.h"

/* Un-comment the following for POTS line support for Japan */
/* #define	JAPAN */

/* Un-comment for lines (eg from and ISDN TA) that remove */
/* phone power during ringing                             */
/* #define ZERO_BATT_RING */

//===========================================================================================   FXO MODES

int	amf_usb_set_opermode(amf_usb_t *ausb, char *opermode)
{
	int x;

	ausb->opermode = 0;
	for (x = 0; x < (sizeof(fxo_modes) / sizeof(fxo_modes[0])); x++) {
		if (!strcmp(fxo_modes[x].name, opermode))
			break;
	}
	if (x < sizeof(fxo_modes) / sizeof(fxo_modes[0])) {
		ausb->opermode = x;
		return 0;
	} else {
		return 1;
	}
}

//=============================================================== MODULE DETECT =============

int amf_usb_module_detect(amf_usb_t *ausb)
{
	int				mod_no;
	unsigned char	byte;
	int 			err;

	ausb->mod_cnt = 0;
	for(mod_no = 0; mod_no < MAX_USB_MODULES; mod_no++){

		err = __amf_usb_fxo_write(ausb, mod_no, 1, 0x80);
		byte = __amf_usb_fxo_read(ausb, mod_no, 2);
		if (byte == 0x03){
			ausb->mod_cnt++;
			A_DEBUG("%s: FXO Line %d detected!\n", AMF_USB_NAME, mod_no + 1);
		}else{
			A_DEBUG("%s: Module %d - Not detected!\n", AMF_USB_NAME, mod_no + 1);
		}
	}
	return 0;
}

//===========================================================================================   INIT_DAA

static int amf_usb_daa_insane(amf_usb_t *ausb, int mod_no)
{
	unsigned char	byte;

	byte = __amf_usb_fxo_read(ausb, mod_no, 2);
	if (byte != 0x3)
		return -2;
	byte = __amf_usb_fxo_read(ausb, mod_no, 11);
	A_DEBUG("%s%d: VoiceDAA System: %02x\n", AMF_USB_NAME, ausb->order, byte & 0xf);
	return 0;
}

int amf_usb_init_daa(amf_usb_t *ausb, int mod_no, int fast, int sane)
{
	unsigned char	reg16=0, reg26=0, reg30=0, reg31=0;
	unsigned long	start_ticks;
	int err = 0;

	u8 byte = 0xFF;
	u8 byte_test = 0xFF;
	// Read / Write test
	byte_test = __amf_usb_fxo_read(ausb, 0, 20);		// mod_no = 0
	err += __amf_usb_fxo_write(ausb, 0, 20, 0x5A);
	wait_just_a_bit(100, 1);
	byte = __amf_usb_fxo_read(ausb, 0, 20);
	// restore
	err += __amf_usb_fxo_write(ausb, 0, 20, byte_test);
	if (byte == 0x5A) {
		A_DEBUG_USB("%s: DAA Read/Write test: OK! (%02X:%02X)\n", AMF_USB_NAME, 0x5A, byte);
	} else {
		DEBUG_EVENT("%s: DAA Read/Write test: ERR! (%02X:%02X)\n", AMF_USB_NAME, 0x5A, byte);
		return -1;
	}
	// Read / Write test (end)

	/* Software reset */
	err += __amf_usb_fxo_write(ausb, mod_no, 1, 0x80);

	wait_just_a_bit(100, 1);

	if (!sane && amf_usb_daa_insane(ausb, mod_no)){
		return -2;
	}

	/* Enable PCM, ulaw */
	if (ausb->alawoverride) {
		err += __amf_usb_fxo_write(ausb, mod_no, 33, 0x20);			// Europe
	}
	else {
		err += __amf_usb_fxo_write(ausb, mod_no, 33, 0x28);			// America
	}

	/* Set On-hook speed, Ringer impedance, and ringer threshold */
	reg16 |= (fxo_modes[ausb->opermode].ohs << 6);
	reg16 |= (fxo_modes[ausb->opermode].rz << 1);
	reg16 |= (fxo_modes[ausb->opermode].rt);
	err += __amf_usb_fxo_write(ausb, mod_no, 16, reg16);

	/* Set DC Termination:
	**	Tip/Ring voltage adjust,
	**	minimum operational current,
	**	current limitation */
	reg26 |= (fxo_modes[ausb->opermode].dcv << 6);
	reg26 |= (fxo_modes[ausb->opermode].mini << 4);
	reg26 |= (fxo_modes[ausb->opermode].ilim << 1);
	err += __amf_usb_fxo_write(ausb, mod_no, 26, reg26);

	/* Set AC Impedance */
	reg30 = (unsigned char)(fxo_modes[ausb->opermode].acim);
	err += __amf_usb_fxo_write(ausb, mod_no, 30, reg30);

	/* Misc. DAA parameters */
	reg31 = 0xa3;
	reg31 |= (fxo_modes[ausb->opermode].ohs2 << 3);
	err += __amf_usb_fxo_write(ausb, mod_no, 31, reg31);

	/* Set Transmit/Receive timeslot */
	err += __amf_usb_fxo_write(ausb, mod_no, 34, mod_no * 8 + 1);
	err += __amf_usb_fxo_write(ausb, mod_no, 35, 0x00);
	err += __amf_usb_fxo_write(ausb, mod_no, 36, mod_no * 8 + 1);
	err += __amf_usb_fxo_write(ausb, mod_no, 37, 0x00);

	/* Enable ISO-Cap */
	err += __amf_usb_fxo_write(ausb, mod_no, 6, 0x00);

	if (!fast) {
		/* Wait 1000ms for ISO-cap to come up */
		start_ticks = jiffies + 2*HZ;
		while(!(__amf_usb_fxo_read(ausb, mod_no, 11) & 0xf0)){
			if (jiffies > start_ticks){
				break;
			}
			wait_just_a_bit(HZ/10, fast);
		}

		if (!(__amf_usb_fxo_read(ausb, mod_no, 11) & 0xf0)) {
			A_DEBUG("%s%d: VoiceDAA did not bring up ISO link properly!\n", AMF_USB_NAME, ausb->order);
			return -1;
		}

		A_DEBUG("%s%d: ISO-Cap is now up, line side: %02x rev %02x\n",
				AMF_USB_NAME, ausb->order,
				(__amf_usb_fxo_read(ausb, mod_no, 11)) >> 4,
				((__amf_usb_fxo_read(ausb, mod_no, 13)) >> 2) & 0xf);

	} else {
		amf_delay(10000);
	}

	/* Enable on-hook line monitor */
	err += __amf_usb_fxo_write(ausb, mod_no, 5, 0x08);
	err += __amf_usb_fxo_write(ausb, mod_no, 3, 0x00);
	err += __amf_usb_fxo_write(ausb, mod_no, 2, 0x04 | 0x03);	/* Ring detect mode (begin/end) */
	ausb->fx[mod_no].fxo.imask = 0x00;

	/* Take values for fxo_txgain and fxo_rxgain and apply them to module */
	if (ausb->fxo_txgain) {
		if (ausb->fxo_txgain >= -150 && ausb->fxo_txgain < 0) {
			A_DEBUG("%s%d: Adjust TX Gain to %2d.%d dB\n",	AMF_USB_NAME, ausb->order,
					ausb->fxo_txgain / 10,
					ausb->fxo_txgain % -10);
			err += __amf_usb_fxo_write(ausb, mod_no, 38, 16 + (ausb->fxo_txgain/-10));
			if(ausb->fxo_txgain % 10) {
				err += __amf_usb_fxo_write(ausb, mod_no, 40, 16 + (-ausb->fxo_txgain%10));
			}
		}
		else if (ausb->fxo_txgain <= 120 && ausb->fxo_txgain > 0) {
			A_DEBUG("%s%d: Adjust TX Gain to %2d.%d dB\n", AMF_USB_NAME, ausb->order,
					ausb->fxo_txgain / 10,
					ausb->fxo_txgain % 10);
			err += __amf_usb_fxo_write(ausb, mod_no, 38, ausb->fxo_txgain/10);
			if (ausb->fxo_txgain % 10){
				err += __amf_usb_fxo_write(ausb, mod_no, 40, (ausb->fxo_txgain % 10));
			}
		}
	}
	if (ausb->fxo_rxgain) {
		if (ausb->fxo_rxgain >= -150 && ausb->fxo_rxgain < 0) {
			A_DEBUG("%s%d: Adjust RX Gain to %2d.%d dB\n",	AMF_USB_NAME, ausb->order,
					ausb->fxo_rxgain / 10,
					(-1) * (ausb->fxo_rxgain % 10));
			err += __amf_usb_fxo_write(ausb, mod_no, 39, 16 + (ausb->fxo_rxgain/-10));
			if(ausb->fxo_rxgain%10) {
				err += __amf_usb_fxo_write(ausb, mod_no, 41, 16 + (-ausb->fxo_rxgain%10));
			}
		}else if (ausb->fxo_rxgain <= 120 && ausb->fxo_rxgain > 0) {
			A_DEBUG("%s%d: Adjust RX Gain to %2d.%d dB\n",	AMF_USB_NAME, ausb->order,
					ausb->fxo_rxgain / 10,
					ausb->fxo_rxgain % 10);
			err += __amf_usb_fxo_write(ausb, mod_no, 39, ausb->fxo_rxgain/10);
			if(ausb->fxo_rxgain % 10) {
				err += __amf_usb_fxo_write(ausb, mod_no, 41, (ausb->fxo_rxgain%10));
			}
		}
	}

	/* NZ -- crank the tx gain up by 7 dB */
	if (!strcmp(fxo_modes[ausb->opermode].name, "NEWZEALAND")) {
		A_DEBUG("%s%d: Adjusting gain\n", AMF_USB_NAME, ausb->order);
		err += __amf_usb_fxo_write(ausb, mod_no, 38, 0x7);
	}

	//set battery to 1 for correct check_hook on init
	ausb->fx[mod_no].fxo.battery = 1;
	{
		u8 reg8;
		u8 reg6;
//		__amf_usb_cpu_read(ausb, 8, &reg8);
//printk(KERN_ERR "+chunk len: %X\n", reg8);
		reg6 = ausb->chunk_size <<4;
#if LOOPBACK_LONG
		reg8 = 1;
#elif LOOPBACK_SHORT
		reg8 = 4;
#endif
printk(KERN_ERR "++chunk len set: %X\n", reg6);
//		__amf_usb_cpu_write(ausb, 8, reg8);
//__amf_usb_cpu_read(ausb, 8, &reg8);
		__amf_usb_cpu_write(ausb, 6, reg6);
__amf_usb_cpu_read(ausb, 6, &reg6);

printk(KERN_ERR "+++++ read back len: %X\n", reg6);

	}

//__amf_usb_cpu_write(ausb, 8, 1);		/* Loopback inside DAA */

/*
//RS>>> Print register:
{
	int i, j = 0;
	u8 value;
	
	for(i = 0; i < 60; i++){
		if(0 == i%10){
			printk("\n\%d --\t", j);
			j++;
		}
		value = __amf_usb_fxo_read(ausb, mod_no, i);
		printk("%02X ", value);
	}
	printk("\n");
}
*/

	return 0;
}


void amf_voicedaa_recheck_sanity(amf_usb_t *ausb, int mod_no) {
	int res;
	/* Check loopback */
	res = ausb->regs[mod_no][34];
	if (!res) {
		A_DEBUG("%s%d: Resetting DAA\n",	AMF_USB_NAME, ausb->order);
		amf_usb_init_daa(ausb, mod_no, 1, 1);
	}
	return;
}

//====================================================================================== Check Hook

void amf_voicedaa_check_hook(amf_usb_t *ausb, int mod_no) {
	signed char b;
	int daa_err = 0;
	unsigned char res;
	int err = 0;

//===================================================== Check DAA Control 1	(reg 5)
	b = ausb->regs[mod_no][5];

	if ((b & 0x2) || !(b & 0x8)) {
		A_DEBUG_USB("%s%d: DAA correction (mod:%d  Reg:5 = %02X)!\n", AMF_USB_NAME, ausb->order, mod_no, (unsigned char)b);
		daa_err++;
	}
	b &= 0x9b;
	if (ausb->fx[mod_no].fxo.offhook) {
		if (b != 0x9){
			A_DEBUG_TX("%s%d %d: Correcting Reg5 <- 0x09 (%02X:offhook=%d:%ld)!\n",
					AMF_USB_NAME, ausb->order, mod_no,
					(unsigned char)b, ausb->fx[mod_no].fxo.offhook, jiffies);
			err = __amf_usb_fxo_write(ausb, mod_no, 5, 0x9);
		}
	} else {
		if (b != 0x8){
			A_DEBUG_TX("%s%d% d: Correcting Reg5 <- 0x08 (%02X:offhook=%d:%ld)!\n",
					AMF_USB_NAME, ausb->order, mod_no,
					(unsigned char)b, ausb->fx[mod_no].fxo.offhook, jiffies);
			err = __amf_usb_fxo_write(ausb, mod_no, 5, 0x8);
		}
	}

	if (daa_err) {
		return;
	}

//===================================================== Check Ring (reg 5)


	if (!ausb->fx[mod_no].fxo.offhook) {
		res = ausb->regs[mod_no][5];
		if ((res & 0x60) && ausb->fx[mod_no].fxo.battery) {
			ausb->fx[mod_no].fxo.ringdebounce += (DAHDI_CHUNKSIZE * 4);           /*16*/
			if (ausb->fx[mod_no].fxo.ringdebounce >= DAHDI_CHUNKSIZE * 256) {      /*64*/
				if (!ausb->fx[mod_no].fxo.wasringing) {
					ausb->fx[mod_no].fxo.wasringing = 1;
					dahdi_hooksig(&ausb->chans[mod_no], DAHDI_RXSIG_RING);
//print_dahdi_call(ausb,  "dahdi_hooksig");
					A_DEBUG("%s%d: RING on span %d (%X)!\n", AMF_USB_NAME, ausb->order,
							ausb->span.spanno, res);
				}
				ausb->fx[mod_no].fxo.ringdebounce = DAHDI_CHUNKSIZE * 64;
			}
		} else {
			ausb->fx[mod_no].fxo.ringdebounce -= DAHDI_CHUNKSIZE * 4;
			if (ausb->fx[mod_no].fxo.ringdebounce <= 0) {
				if (ausb->fx[mod_no].fxo.wasringing) {
					ausb->fx[mod_no].fxo.wasringing = 0;
					dahdi_hooksig(&ausb->chans[mod_no], DAHDI_RXSIG_OFFHOOK);
//print_dahdi_call(ausb, "dahdi_hooksig");
					A_DEBUG("%s%d: NO RING on span %d (off-hook) (%X)!\n", AMF_USB_NAME, ausb->order,
							ausb->span.spanno, res);
				}
				ausb->fx[mod_no].fxo.ringdebounce = 0;
			}
		}
	}

//===================================================== Check Line (reg 29)
	b = ausb->regs[mod_no][29];

	if (abs(b) <= 1){
		ausb->fx[mod_no].fxo.statusdebounce ++;
		if (ausb->fx[mod_no].fxo.statusdebounce >= FXO_LINK_DEBOUNCE){

			if (ausb->fx[mod_no].fxo.status != FXO_DISCONNECTED){
				A_DEBUG("%s%d: FXO Line is disconnected!\n", AMF_USB_NAME, ausb->order);
				ausb->fx[mod_no].fxo.status = FXO_DISCONNECTED;

			}
			ausb->fx[mod_no].fxo.statusdebounce = FXO_LINK_DEBOUNCE;
		}
	}else{
		ausb->fx[mod_no].fxo.statusdebounce--;
		if (ausb->fx[mod_no].fxo.statusdebounce <= 0) {

			if (ausb->fx[mod_no].fxo.status != FXO_CONNECTED){
				A_DEBUG("%s%d: FXO Line is connected!\n", AMF_USB_NAME, ausb->order);
				ausb->fx[mod_no].fxo.status = FXO_CONNECTED;
			}
			ausb->fx[mod_no].fxo.statusdebounce = 0;
		}
	}

//===================================================== Check Battery


	if (abs(b) < ausb->battthresh) {
		if (ausb->fx[mod_no].fxo.battery && !ausb->fx[mod_no].fxo.battdebounce) {
			A_DEBUG("%s%d: NO BATTERY on span %d (%02X)!\n", AMF_USB_NAME,	ausb->order,
						ausb->span.spanno, (unsigned char)b);
#ifdef JAPAN
			if ((!ausb->fx[mod_no].fxo.ohdebounce) && ausb->fx[mod_no].fxo.offhook) {
				dahdi_hooksig(&ausb->chans[mod_no], DAHDI_RXSIG_ONHOOK);
//print_dahdi_call(ausb, "dahdi_hooksig");
				A_DEBUG("%s%d: Signalled On Hook span %d\n", AMF_USB_NAME, ausb->order, ausb->span.spanno);
#ifdef ZERO_BATT_RING
				ausb->fx[mod_no].fxo.onhook++;
#endif
			}
#else
			dahdi_hooksig(&ausb->chans[mod_no], DAHDI_RXSIG_ONHOOK);
//print_dahdi_call(ausb, "dahdi_hooksig");
			A_DEBUG("%s%d: Signalled On Hook span %d (%02X)\n", AMF_USB_NAME, ausb->order,
						ausb->span.spanno, (unsigned char)b);
#endif
			ausb->fx[mod_no].fxo.battery =  0;
			ausb->fx[mod_no].fxo.battdebounce = ausb->battdebounce;
			// Digium style alarm:
			ausb->span.alarms = DAHDI_ALARM_RED;
			dahdi_alarm_notify(&ausb->span);
//print_dahdi_call(ausb, "dahdi_alarm_notify");

		} else if (!ausb->fx[mod_no].fxo.battery) {
			ausb->fx[mod_no].fxo.battdebounce = ausb->battdebounce;
		}


	} else if (abs(b) >= ausb->battthresh) {
		if (!ausb->fx[mod_no].fxo.battery && !ausb->fx[mod_no].fxo.battdebounce) {
			A_DEBUG("%s%d: BATTERY on span %d (%s) (%02X)!\n", AMF_USB_NAME, ausb->order,
						ausb->span.spanno, (b < 0) ? "-" : "+", (unsigned char)b);

#ifdef ZERO_BATT_RING
			if (ausb->fx[mod_no].fxo.onhook) {
				ausb->fx[mod_no].fxo.onhook = 0;
				dahdi_hooksig(&ausb->chans[mod_no], DAHDI_RXSIG_OFFHOOK);
//print_dahdi_call(ausb, "dahdi_hooksig");
				A_DEBUG("%s%d: Signalled Off Hook span %d\n", AMF_USB_NAME, ausb->order,
						ausb->span.spanno);
			}
#else
			dahdi_hooksig(&ausb->chans[mod_no], DAHDI_RXSIG_OFFHOOK);
//print_dahdi_call(ausb, "dahdi_hooksig");
			A_DEBUG("%s%d: Signalled Off Hook span %d (%02X)\n", AMF_USB_NAME, ausb->order,
						ausb->span.spanno, (unsigned char)b);
#endif
			ausb->fx[mod_no].fxo.battery = 1;
			ausb->fx[mod_no].fxo.battdebounce = ausb->battdebounce;

			// Digium style alarm:
			if (ausb->span.alarms) {
				ausb->span.alarms = 0;
				dahdi_alarm_notify(&ausb->span);
//print_dahdi_call(ausb, "dahdi_alarm_notify");
			}

		} else if (ausb->fx[mod_no].fxo.battery) {
			ausb->fx[mod_no].fxo.battdebounce = ausb->battdebounce;
		}

	}

	if (ausb->fx[mod_no].fxo.battdebounce) {
		ausb->fx[mod_no].fxo.battdebounce--;
	}

	if ((b >= 0) && (ausb->fx[mod_no].fxo.lastpol >= 0)) {

		ausb->fx[mod_no].fxo.lastpol = 1;
		ausb->fx[mod_no].fxo.polaritydebounce = POLARITY_DEBOUNCE;

	} else if ((b < 0) && (ausb->fx[mod_no].fxo.lastpol <= 0)) {

		ausb->fx[mod_no].fxo.lastpol = -1;
		ausb->fx[mod_no].fxo.polaritydebounce = POLARITY_DEBOUNCE;
	}

	if (ausb->fx[mod_no].fxo.polaritydebounce) {
		ausb->fx[mod_no].fxo.polaritydebounce--;
	}

	if (ausb->fx[mod_no].fxo.polaritydebounce < 1) {
		if (ausb->fx[mod_no].fxo.lastpol != ausb->fx[mod_no].fxo.polarity) {
			A_DEBUG("%s%d: Polarity reversed %d -> %d (%ld)\n", AMF_USB_NAME, ausb->order,
						ausb->fx[mod_no].fxo.polarity,
						ausb->fx[mod_no].fxo.lastpol,
						jiffies);
			if (ausb->fx[mod_no].fxo.polarity){
				dahdi_qevent_lock(&ausb->chans[mod_no],	DAHDI_EVENT_POLARITY);
//print_dahdi_call(ausb, "dahdi_qevent_lock");
			}
			ausb->fx[mod_no].fxo.polarity = ausb->fx[mod_no].fxo.lastpol;
		}
	}

	return;
}


//============================================================== FXO READ / WRITE

u_int8_t __amf_usb_fxo_read(amf_usb_t *ausb, int mod_no, unsigned char off)
{
	int retval;

	retval = amf_usb_read_value(RD_FXO_COMMAND, ausb, off, 1000);

	if(0 > retval){
		printk(KERN_ERR "Read error DAA: %d\n", off);
	}

	return retval & 0xFF; 
}


int __amf_usb_fxo_write(amf_usb_t *ausb, int mod_no, unsigned char off, unsigned char data)
{
	/* update registers shadow */
	ausb->regs[mod_no][off]  = data;

	return amf_usb_write_value(WR_FXO_COMMAND, ausb, off, data, 0, 100);
}





