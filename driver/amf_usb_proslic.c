 /*
  *  amf_usb_proslic.c - 'proslic related functions'
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

//#define AMF_INIT_PROSLIC_IN_CPU
//#define AMF_USB_AUTO_CALIBRATE

#define OHT_TIMER               6000

// Indirect access registers: Traditional way
#define IDA_LO		28
#define IDA_HI		29
#define IAA 		30

static alpha  indirect_regs[] =
{
{0,255,"DTMF_ROW_0_PEAK",0x55C2},
{1,255,"DTMF_ROW_1_PEAK",0x51E6},
{2,255,"DTMF_ROW2_PEAK",0x4B85},
{3,255,"DTMF_ROW3_PEAK",0x4937},
{4,255,"DTMF_COL1_PEAK",0x3333},
{5,255,"DTMF_FWD_TWIST",0x0202},
{6,255,"DTMF_RVS_TWIST",0x0202},
{7,255,"DTMF_ROW_RATIO_TRES",0x0198},
{8,255,"DTMF_COL_RATIO_TRES",0x0198},
{9,255,"DTMF_ROW_2ND_ARM",0x0611},
{10,255,"DTMF_COL_2ND_ARM",0x0202},
{11,255,"DTMF_PWR_MIN_TRES",0x00E5},
{12,255,"DTMF_OT_LIM_TRES",0x0A1C},
{13,0,"OSC1_COEF",0x7B30},
{14,1,"OSC1X",0x0063},
{15,2,"OSC1Y",0x0000},
{16,3,"OSC2_COEF",0x7870},
{17,4,"OSC2X",0x007D},
{18,5,"OSC2Y",0x0000},
{19,6,"RING_V_OFF",0x0000},
{20,7,"RING_OSC",0x7EF0},
{21,8,"RING_X",0x0160},
{22,9,"RING_Y",0x0000},
{23,255,"PULSE_ENVEL",0x2000},
{24,255,"PULSE_X",0x2000},
{25,255,"PULSE_Y",0x0000},
/*{26,13,"RECV_DIGITAL_GAIN",0x4000},*/	/* playback volume set lower */
{26,13,"RECV_DIGITAL_GAIN",0x2000},	/* playback volume set lower */
{27,14,"XMIT_DIGITAL_GAIN",0x4000},
/*{27,14,"XMIT_DIGITAL_GAIN",0x2000}, */
{28,15,"LOOP_CLOSE_TRES",0x1000},
{29,16,"RING_TRIP_TRES",0x3600},
{30,17,"COMMON_MIN_TRES",0x1000},
{31,18,"COMMON_MAX_TRES",0x0200},
{32,19,"PWR_ALARM_Q1Q2",0x07C0},
{33,20,"PWR_ALARM_Q3Q4",0x2600},
{34,21,"PWR_ALARM_Q5Q6",0x1B80},
{35,22,"LOOP_CLOSURE_FILTER",0x8000},
{36,23,"RING_TRIP_FILTER",0x0320},
{37,24,"TERM_LP_POLE_Q1Q2",0x008C}, /*original*/
{38,25,"TERM_LP_POLE_Q3Q4",0x0100},
{39,26,"TERM_LP_POLE_Q5Q6",0x0010},
{40,27,"CM_BIAS_RINGING",0x0C00},
{41,64,"DCDC_MIN_V",0x0C00},
/*{42,255,"DCDC_XTRA",0x1000},*/
{43,66,"LOOP_CLOSE_TRES_LOW",0x1000},
};

static int acim2tiss[16] = { 0x0, 0x1, 0x4, 0x5, 0x7, 0x0, 0x0, 0x6, 0x0, 0x0, 0x0, 0x2, 0x0, 0x3 };

static int amf_usb_proslic_init_indirect_regs(amf_usb_t *ausb, int mod_no, int fast);
static int amf_usb_proslic_setreg_indirect(amf_usb_t *ausb, int mod_no, unsigned char address, unsigned short data, int fast);
static int amf_usb_proslic_getreg_indirect(amf_usb_t *ausb, int mod_no, unsigned char address, int fast);
static int __amf_usb_fxs_write_indirect(amf_usb_t *ausb, int mod_no, unsigned char off, unsigned short data, int fast);
static u_int16_t __amf_usb_fxs_read_indirect(amf_usb_t *ausb, int mod_no, unsigned char off, int fast);


//===============================================================================================================================  PROSLIC INSANE

static int amf_usb_init_proslic_insane(amf_usb_t *ausb, int mod_no)
{
	unsigned char	value;

	value = __amf_usb_fxo_read(ausb, mod_no, 0);

	if ((value & 0x30) >> 4){
		DEBUG_EVENT("%s%d: Proslic is not a Si3210 (%02X)\n", AMF_USB_NAME, ausb->order, value);
		return -1;
	} else {
		A_DEBUG_USB("%s%d: Proslic Si3210 detected (%02X)\n", AMF_USB_NAME, ausb->order, value);
	}

	if (((value & 0x0F) == 0) || ((value & 0x0F) == 0x0F)){
		DEBUG_EVENT("%s%d: Proslic is not loaded (%02X)\n", AMF_USB_NAME, ausb->order, value & 0x0F);
		return -1;
	} else {
		A_DEBUG_USB("%s%d: Proslic is loaded (%02X)\n", AMF_USB_NAME, ausb->order, value & 0x0F);
	}

	if ((value & 0x0F) < 2){
		DEBUG_EVENT("%s%d: Proslic 3210 version %d is too old\n", AMF_USB_NAME, ausb->order, value & 0x0F);
		return -1;
	} else {
		A_DEBUG_USB("%s%d: Proslic 3210 version: %d OK\n", AMF_USB_NAME, ausb->order, value & 0x0F);
	}

	/* Step 8 */
	value = __amf_usb_fxo_read(ausb, mod_no, 8);
	if (value != 0x2) {
		DEBUG_EVENT("%s%d: Proslic insane (1) %d should be 2\n", AMF_USB_NAME, ausb->order, value);
		return -1;
	} else {
		A_DEBUG_USB("%s%d: Proslic insane (1): %d OK\n", AMF_USB_NAME, ausb->order, value);
	}

	value = __amf_usb_fxo_read(ausb, mod_no, 64);
	if (value != 0x0) {
		DEBUG_EVENT("%s%d: Proslic insane (2) %d should be 0\n", AMF_USB_NAME, ausb->order, value);
		return -1;
	} else {
		A_DEBUG_USB("%s%d: Proslic insane (2): %d OK\n", AMF_USB_NAME, ausb->order, value);
	}

	value = __amf_usb_fxo_read(ausb, mod_no, 11);
	if (value != 0x33) {
		DEBUG_EVENT("%s%d: Proslic insane (3) %02X should be 0x33\n", AMF_USB_NAME, ausb->order, value);
		return -1;
	} else {
		A_DEBUG_USB("%s%d: Proslic insane (3): %d OK\n", AMF_USB_NAME, ausb->order, value);
	}

	__amf_usb_fxo_write(ausb, mod_no, 30, 0);


	return 0;
}


//===============================================================================================================================  POWERUP PROSLIC

static int amf_usb_powerup_proslic(amf_usb_t *ausb, int mod_no, int fast)
{
	amf_fxs_t	*fxs;
	unsigned long	start_ticks;
	int		loopcurrent = 20, lim,i;
	unsigned char	vbat;

	A_DEBUG_USB("%s%d: PowerUp SLIC initialization...\n", AMF_USB_NAME, ausb->order);
	fxs = &ausb->fx[mod_no].fxs;


	/* set the period of the DC-DC converter to 1/64 kHz  START OUT SLOW*/
	//	__amf_usb_fxo_write(ausb, mod_no, 93, 0x1F);						// Original
	//	__amf_usb_fxo_write(ausb, mod_no, 92, 0xF5); /*0xf5 for BJT range */
	__amf_usb_fxo_write(ausb, mod_no, 93, 5);								// Michael, new settings for POWER CONSUMPTION May, 2014
	__amf_usb_fxo_write(ausb, mod_no, 92, 193);         					//

	if (fast) return 0;

	/* powerup */

	__amf_usb_fxo_write(ausb, mod_no, 14, 0x00);	/* DIFF DEMO 0x10 */

	start_ticks = jiffies;
	i = 0;
	while((vbat = __amf_usb_fxo_read(ausb, mod_no, 82)) < 0xC0){
		i++;
		/* Wait no more than 500ms */
		if ( i > 10 ){
			break;
		}
		wait_just_a_bit(10, fast);
	}
	/* Step 12-c */
	if (vbat < 0xC0){
		if (fxs->proslic_power == PROSLIC_POWER_UNKNOWN){
			DEBUG_EVENT("%s%d: Failed to powerup within %d ms (%dV : %dV)!\n", AMF_USB_NAME, ausb->order,
					(u_int32_t)(((jiffies - start_ticks) * 1000 / HZ)),
					(vbat * 375)/1000, (0xc0 * 375)/1000);
			DEBUG_EVENT("%s%d: Did you remember to plug in the power cable?\n", AMF_USB_NAME, ausb->order);

		}
		fxs->proslic_power = PROSLIC_POWER_WARNED;
		return -1;
	}
	fxs->proslic_power = PROSLIC_POWER_ON;
	A_DEBUG_USB("%s%d: Current Battery1 %dV, Battery2 %dV\n", AMF_USB_NAME, ausb->order,
					__amf_usb_fxo_read(ausb, mod_no, 82)*375/1000,
					__amf_usb_fxo_read(ausb, mod_no, 83)*375/1000);

        /* Proslic max allowed loop current, reg 71 LOOP_I_LIMIT */
        /* If out of range, just set it to the default value     */
        lim = (loopcurrent - 20) / 3;
        if ( loopcurrent > 41 ) {
                lim = 0;
		DEBUG_EVENT("%s%d: Loop current out of range (default 20mA)!\n", AMF_USB_NAME, ausb->order);
        } else {
		A_DEBUG_USB("%s%d: Loop current set to %dmA!\n", AMF_USB_NAME, ausb->order,	(lim*3)+20);
	}
    __amf_usb_fxo_write(ausb, mod_no, 71,lim);

	__amf_usb_fxo_write(ausb, mod_no, 93, 0x99);  /* DC-DC Calibration  */
	/* Wait for DC-DC Calibration to complete */
	i = 0;
	while(0x80 & __amf_usb_fxo_read(ausb, mod_no, 93)){
		i++;
		if (i > 20){
			DEBUG_EVENT("%s%d: Timeout waiting for DC-DC calibration (%02X)\n", AMF_USB_NAME, ausb->order, __amf_usb_fxo_read(ausb, mod_no, 93));
			return -EINVAL;
		}
		wait_just_a_bit(10, fast);
	}
	A_DEBUG_USB("%s%d: PowerUp SLIC initialization...Done!\n", AMF_USB_NAME, ausb->order);
	return 0;
}

//===============================================================================================================================  PROSLIC POWERLEAK

static int amf_usb_proslic_powerleak_test(amf_usb_t *ausb, int mod_no, int fast)
{
	unsigned long 			start_ticks;
	unsigned char	vbat;

	DEBUG_CFG("%s: PowerLeak ProSLIC testing...\n", fe->name);
	/* powerleak */
	__amf_usb_fxo_write(ausb, mod_no, 64, 0);
	__amf_usb_fxo_write(ausb, mod_no, 14, 0x10);

	start_ticks = jiffies;
	wait_just_a_bit(40, fast);
	vbat = __amf_usb_fxo_read(ausb, mod_no, 82);
	if (vbat < 0x6){
		DEBUG_EVENT("%s%d: Excessive leakage detected: %d volts (%02x) after %d ms\n", AMF_USB_NAME, ausb->order,
					376 * vbat / 1000,
					vbat,
					(u_int32_t)((jiffies - start_ticks) * 1000 / HZ));
		return -1;
	}
	A_DEBUG_USB("%s%d: Post-leakage voltage: %d volts\n", AMF_USB_NAME, ausb->order,	376 * vbat / 1000);
	A_DEBUG_USB("%s%d: PowerLeak ProSLIC testing...Done!\n", AMF_USB_NAME, ausb->order);

	return 0;
}


#ifdef AMF_USB_AUTO_CALIBRATE
static int amf_usb_proslic_calibrate(amf_usb_t *ausb, int mod_no, int fast)
{
	unsigned long	start_ticks;

	A_DEBUG_USB("%s%d: ProSLIC calibration...\n", AMF_USB_NAME, ausb->order);
	/* perform all calibration */
	__amf_usb_fxo_write(ausb, mod_no, 97, 0x1f);
	/* start */
	__amf_usb_fxo_write(ausb, mod_no, 96, 0x5f);

	start_ticks = jiffies;
	while(__amf_usb_fxo_read(ausb, mod_no, 96)){
		if ((jiffies - start_ticks) > 2*HZ){
			DEBUG_EVENT("%s%d: Timeout on module calibration!\n", AMF_USB_NAME, ausb->order);
			return -1;
		}
		wait_just_a_bit(10, fast);
	}
	A_DEBUG_USB("%s%d: ProSLIC calibration...Done!\n",  AMF_USB_NAME, ausb->order);
	return 0;
}
#endif

//===============================================================================================================================  PROSLIC MANUAL CALIBRATION

static int amf_usb_proslic_manual_calibrate(amf_usb_t *ausb, int mod_no, int fast)
{
	unsigned long	start_ticks;
	int				i = 0;

	A_DEBUG_USB("%s%d: ProSLIC manual calibration...\n", AMF_USB_NAME, ausb->order);
	__amf_usb_fxo_write(ausb, mod_no, 21, 0x00);
	__amf_usb_fxo_write(ausb, mod_no, 22, 0x00);
	__amf_usb_fxo_write(ausb, mod_no, 23, 0x00);
	/* Step 13 */
	__amf_usb_fxo_write(ausb, mod_no, 64, 0x00);

	/* Step 14 */
	__amf_usb_fxo_write(ausb, mod_no, 97, 0x1E);
	__amf_usb_fxo_write(ausb, mod_no, 96, 0x47);

	/* Step 15 */
	i = 0;
	while(__amf_usb_fxo_read(ausb, mod_no, 96) != 0){
		i++;
		if (i > 50){
			DEBUG_EVENT("%s%d: Timeout on SLIC calibration (Step 15)!\n", AMF_USB_NAME, ausb->order);
			return -1;
		}
		wait_just_a_bit(40, fast);

	}

	amf_usb_proslic_setreg_indirect(ausb, mod_no, 88, 0x00, fast);
	amf_usb_proslic_setreg_indirect(ausb, mod_no, 89, 0x00, fast);
	amf_usb_proslic_setreg_indirect(ausb, mod_no, 90, 0x00, fast);
	amf_usb_proslic_setreg_indirect(ausb, mod_no, 91, 0x00, fast);
	amf_usb_proslic_setreg_indirect(ausb, mod_no, 92, 0x00, fast);
	amf_usb_proslic_setreg_indirect(ausb, mod_no, 93, 0x00, fast);

	/* Step 16 */
	/* Insert manual calibration */

	for (i = 0x1f; i > 0; i--){
		__amf_usb_fxo_write(ausb, mod_no, 98, i);
		wait_just_a_bit(25, fast);
		if ((__amf_usb_fxo_read(ausb, mod_no, 88)) == 0){
			break;
		}
	}
	for (i = 0x1f; i > 0; i--){
		__amf_usb_fxo_write(ausb, mod_no, 99, i);
		wait_just_a_bit(25, fast);
		if ((__amf_usb_fxo_read(ausb, mod_no, 89)) == 0){
			break;
		}
	}

	/* Step 17 */
	__amf_usb_fxo_write(ausb, mod_no, 23, 0x04);

	/* Step 18 */
	/* DAC offset and without common mode calibration. */
	__amf_usb_fxo_write(ausb, mod_no, 97, 0x01);	/* Manual after */
	/* Calibrate common mode and differential DAC mode DAC + ILIM */
	__amf_usb_fxo_write(ausb, mod_no, 96, 0x40);

	/* Step 19 */
	i = 0;
	start_ticks = jiffies;
	while(__amf_usb_fxo_read(ausb, mod_no, 96) != 0){
		i++;
		if (i > 50){
			DEBUG_EVENT("%s%d: Timeout on SLIC calibration (%ld:%ld)!\n", AMF_USB_NAME, ausb->order, (unsigned long)start_ticks, (unsigned long)jiffies);
			return -1;
		}
		wait_just_a_bit(40, fast);
	}

	A_DEBUG_USB("%s%d: ProSLIC manual calibration...Done!\n", AMF_USB_NAME, ausb->order);

	return 0;
}


//===============================================================================================================================  PROSLIC VERIFY INDIRECT


static int amf_usb_proslic_verify_indirect_regs(amf_usb_t *ausb, int mod_no, int fast)
{
	int passed = 1;
	unsigned short i, initial;
	int j;

	A_DEBUG_USB("%s%d: Indirect Registers verification...\n", AMF_USB_NAME, ausb->order);

	for (i = 0; i < sizeof(indirect_regs) / sizeof(indirect_regs[0]); i++){

		j = amf_usb_proslic_getreg_indirect(ausb, mod_no, (unsigned char) indirect_regs[i].address, fast);
		if (j < 0){
			DEBUG_EVENT("%s%d: Failed to read indirect register %d\n", AMF_USB_NAME, ausb->order, i);
			return -1;
		}
		initial= indirect_regs[i].initial;

		if ( j != initial && indirect_regs[i].altaddr != 255){
			DEBUG_EVENT("%s%d: Internal Error: iReg=%s (%d) Value=%X (%X)\n", AMF_USB_NAME, ausb->order, indirect_regs[i].name, indirect_regs[i].address, j, initial);
			passed = 0;
		}
	}

	if (!passed) {
		return -1;
	}
	return 0;
}


//===============================================================================================================================  INIT PROSLIC
//===============================================================================================================================  INIT  PROSLIC
//===============================================================================================================================  INIT   PROSLIC


int amf_usb_init_proslic(amf_usb_t *ausb, int mod_no, int fast, int sane)
{
	unsigned short		tmp[5];
	unsigned char		value;
	unsigned char		value1;
	unsigned char		value_old = 0x00;
	unsigned char		value1_old = 0x00;
	volatile int		x;

	A_DEBUG_USB("%s%d: STARTING PROSLIC INIT...\n", AMF_USB_NAME, ausb->order);


#ifdef AMF_INIT_PROSLIC_IN_CPU

	err = __amf_usb_cpu_init_proslic(ausb, 0, &value);

	A_DEBUG("%s%d: ...CPU INIT PROSLIC returned: %02X, going to software post calibration ...\n", AMF_USB_NAME, ausb->order, value);

#else

	// INIT PROSLIC IN DRIVER

	// Read/Write test:
	value_old = __amf_usb_fxo_read(ausb, mod_no, 2);
	value1_old = __amf_usb_fxo_read(ausb, mod_no, 4);

	__amf_usb_fxo_write(ausb, mod_no, 2, 0x55);
	__amf_usb_fxo_write(ausb, mod_no, 4, 0xAA);
	wait_just_a_bit(10, fast);

	value = __amf_usb_fxo_read(ausb, mod_no, 2);
	value1 = __amf_usb_fxo_read(ausb, mod_no, 4);

	if ((value == 0x55) & (value1 == 0xAA)) {
		A_DEBUG_USB("%s%d: Proslic Write/Read test: OK!\n", AMF_USB_NAME, ausb->order);
	} else {
		DEBUG_EVENT("%s%d: Proslic Write/Read test Failed! After Write: Reg2:%02X   Reg4:%02X   jf%ld\n", AMF_USB_NAME, ausb->order, value, value1, jiffies);
		return -2;
	}
	__amf_usb_fxo_write(ausb, mod_no, 2, value_old);
	__amf_usb_fxo_write(ausb, mod_no, 4, value1_old);

	wait_just_a_bit(10, fast);

	// Write/Read test END

	/* By default, don't send on hook */ // DIGIUM default = 0; set by DAHDI_SET_POLARITY...
	if (ausb->reversepolarity){
		ausb->fx[mod_no].fxs.idletxhookstate = 5;
	}else{
		ausb->fx[mod_no].fxs.idletxhookstate = 1;
	}

	/* Step 8 */
	if (!sane && amf_usb_init_proslic_insane(ausb, mod_no)){
		return -2;
	}

	if (sane){
		__amf_usb_fxo_write(ausb, mod_no, 14, 0x10);
	}

	if (!fast){
		ausb->fx[mod_no].fxs.proslic_power = PROSLIC_POWER_UNKNOWN;
	}


	/* Step 9 */
	// No need in Indirect Write/Read test here: indirect access is monitored by MCPU 2 (bit 5) status!
	A_DEBUG_USB("%s%d: Initializing Indirect Registers...\n", AMF_USB_NAME, ausb->order);
	if (amf_usb_proslic_init_indirect_regs(ausb, mod_no, fast)) {
		DEBUG_EVENT("%s%d: Initializing Indirect Registers...Failed\n", AMF_USB_NAME, ausb->order);
		return -1;
	}
	A_DEBUG_USB("%s%d: Initializing Indirect Registers...Done!\n", AMF_USB_NAME, ausb->order);

#endif

	// Do it in both cases - software or firmware: (??)
	amf_usb_proslic_setreg_indirect(ausb, mod_no, 97,0, fast);


	/* Step 10 */
	__amf_usb_fxo_write(ausb, mod_no, 8, 0);			/*DIGIUM*/
	__amf_usb_fxo_write(ausb, mod_no, 108, 0xeb);		/*DIGIUM*/
	__amf_usb_fxo_write(ausb, mod_no, 67, 0x17);
	__amf_usb_fxo_write(ausb, mod_no, 66, 1);

	// Do it in both cases - software or firmware:
	/* Flush ProSLIC digital filters by setting to clear, while
	** saving old values */
	for (x = 0; x < 5; x++) {
		tmp[x] = (unsigned short)amf_usb_proslic_getreg_indirect(ausb, mod_no, x + 35, fast);
		amf_usb_proslic_setreg_indirect(ausb, mod_no, x + 35, 0x8000, fast);
	}

#ifndef AMF_INIT_PROSLIC_IN_CPU

	/* Power up the DC-DC converter */
	if (amf_usb_powerup_proslic(ausb, mod_no, fast)) {
		DEBUG_EVENT("%s%d: Unable to do INITIAL ProSLIC powerup!\n", AMF_USB_NAME, ausb->order);
		return -1;
	}


	if (!fast){
		if (amf_usb_proslic_powerleak_test(ausb, mod_no, fast)){
			DEBUG_EVENT("%s%d: Proslic failed leakage the short circuit\n", AMF_USB_NAME, ausb->order);
		}

		/* Step 12 */
		if (amf_usb_powerup_proslic(ausb, mod_no, fast)) {
			DEBUG_EVENT("%s%d: Unable to do FINAL ProSLIC powerup!\n", AMF_USB_NAME, ausb->order);
			return -1;
		}

		/* Step 13 */
		__amf_usb_fxo_write(ausb, mod_no, 64, 0);

#ifdef AMF_USB_AUTO_CALIBRATE
		if (amf_usb_proslic_calibrate(ausb, mod_no, fast)){
			return -1;
		}
#else
		if (amf_usb_proslic_manual_calibrate(ausb, mod_no, fast)){
			return -1;
		}
#endif

	}

#endif



	if (!fast){

		/* Doesn't look neccessary to calibrate again! */
		/* Perform DC-DC calibration */
		__amf_usb_fxo_write(ausb, mod_no, 93, 0x99);
		wait_just_a_bit(100, fast);
		value = __amf_usb_fxo_read(ausb, mod_no, 107);
		if ((value < 0x2) || (value > 0xd)) {
			DEBUG_EVENT("%s%d: DC-DC calibration has a surprising direct 107 of 0x%02x!\n",	AMF_USB_NAME, ausb->order,	value);
			__amf_usb_fxo_write(ausb, mod_no, 107, 0x8);
		}

		/* Save calibration vectors */
		for (x = 0; x < NUM_CAL_REGS; x++){
			ausb->fx[mod_no].fxs.callregs.vals[x] = __amf_usb_fxo_read(ausb, mod_no, 96 + x);
		}

	} else {
		/* Restore calibration vectors */
		for (x = 0; x < NUM_CAL_REGS; x++){
			__amf_usb_fxo_write(ausb, mod_no, 96 + x, ausb->fx[mod_no].fxs.callregs.vals[x]);
		}

	}

	for (x = 0; x < 5; x++) {
		amf_usb_proslic_setreg_indirect(ausb, mod_no, x + 35, tmp[x], fast);
	}

	if (amf_usb_proslic_verify_indirect_regs(ausb, mod_no, fast)) {
		DEBUG_EVENT("%s%d: Indirect Registers verification failed.\n", AMF_USB_NAME, ausb->order);
		return -1;           // CPU intit: Err in 40 only bypass!
	} else {
		A_DEBUG_USB("%s%d: Indirect Registers verification: Done!\n", AMF_USB_NAME, ausb->order);
	}

	__amf_usb_fxo_write(ausb, mod_no, 67, 0x17);

	/* Enable PCM, ulaw */
	if (ausb->alawoverride) {
		__amf_usb_fxo_write(ausb, mod_no, 1, 0x20);					// Europe
	}
	else {
		__amf_usb_fxo_write(ausb, mod_no, 1, 0x28);					// America
	}

	/* U-Law 8-bit interface */
	/* Tx Start count low byte  0 */
	//__amf_usb_fxo_write(ausb, mod_no, 2, mod_no * 8 + 1);			// original
	__amf_usb_fxo_write(ausb, mod_no, 2, 1);						// Michael
	/* Tx Start count high byte 0 */
	__amf_usb_fxo_write(ausb, mod_no, 3, 0);
	/* Rx Start count low byte  0 */
	//__amf_usb_fxo_write(ausb, mod_no, 4, mod_no * 8 + 1);			// original
	__amf_usb_fxo_write(ausb, mod_no, 4, 1);						// Michael
	/* Rx Start count high byte 0 */
	__amf_usb_fxo_write(ausb, mod_no, 5, 0);
	/* Clear all interrupt */
	__amf_usb_fxo_write(ausb, mod_no, 18, 0xff);
	__amf_usb_fxo_write(ausb, mod_no, 19, 0xff);
	__amf_usb_fxo_write(ausb, mod_no, 20, 0xff);
	__amf_usb_fxo_write(ausb, mod_no, 73, 0x04);

	if (!strcmp(fxo_modes[ausb->opermode].name, "AUSTRALIA") ||
	    !strcmp(fxo_modes[ausb->opermode].name, "TBR21")) {

		value = (unsigned char)acim2tiss[fxo_modes[ausb->opermode].acim];
		__amf_usb_fxo_write(ausb, mod_no, 10, 0x8 | value);
		if (fxo_modes[ausb->opermode].ring_osc){
			amf_usb_proslic_setreg_indirect(ausb, mod_no, 20, (unsigned short)fxo_modes[ausb->opermode].ring_osc, fast);
		}
		if (fxo_modes[ausb->opermode].ring_x){
			amf_usb_proslic_setreg_indirect(ausb, mod_no, 21, (unsigned short)fxo_modes[ausb->opermode].ring_x, fast);
		}
	}

	/* lowpower */
	if (ausb->lowpower != 0){
		__amf_usb_fxo_write(ausb, mod_no, 72, 0x10);
	}

	if (ausb->fastringer != 0){
		/* Speed up Ringer */
		amf_usb_proslic_setreg_indirect(ausb, mod_no, 20, 0x7e6d, fast);
		amf_usb_proslic_setreg_indirect(ausb, mod_no, 21, 0x01b9, fast);
		/* Beef up Ringing voltage to 89V */
		if (!strcmp(fxo_modes[ausb->opermode].name, "AUSTRALIA")) {
			__amf_usb_fxo_write(ausb, mod_no, 74, 0x3f);
			if (amf_usb_proslic_setreg_indirect(ausb, mod_no, 21, 0x247, fast)){
				return -1;
			}
			DEBUG_EVENT("%s%d: Boosting fast ringer (89V peak)\n", AMF_USB_NAME, ausb->order);
		} else if (ausb->lowpower != 0){
			if (amf_usb_proslic_setreg_indirect(ausb, mod_no, 21, 0x14b, fast)){
				return -1;
			}
			DEBUG_EVENT("%s%d: Reducing fast ring power (50V peak)\n", AMF_USB_NAME, ausb->order);
		} else {
			DEBUG_EVENT("%s%d: Speeding up ringer (25Hz)\n", AMF_USB_NAME, ausb->order);
		}
	}else{
		if (!strcmp(fxo_modes[ausb->opermode].name, "AUSTRALIA")) {
			if (ausb->ringampl){
				u16	ringx = 0x00;
				u8	vbath = 0x00;
				switch(ausb->ringampl){
				case 47: ringx = 0x163; vbath = 0x31; break;
				case 45: ringx = 0x154; vbath = 0x2f; break;
				case 40: ringx = 0x12e; vbath = 0x2b; break;
				case 35: ringx = 0x108; vbath = 0x26; break;
				case 30: ringx = 0xe2; vbath = 0x21; break;
				case 25: ringx = 0xbc; vbath = 0x1d; break;
				case 20: ringx = 0x97; vbath = 0x1b; break;
				case 15: ringx = 0x71; vbath = 0x13; break;
				case 10: ringx = 0x4b; vbath = 0x0e; break;
				}
				if (ringx && vbath){
					DEBUG_EVENT("%s%d: Ringing Amplitude %d (RNGX:%04X VBATH:%02X)\n", AMF_USB_NAME, ausb->order, ausb->ringampl, ringx, vbath);
					__amf_usb_fxo_write(ausb, mod_no, 74, vbath);
					if (amf_usb_proslic_setreg_indirect(ausb, mod_no, 21, ringx, fast)){
						DEBUG_EVENT("%s%d: Failed to set RingX value!\n", AMF_USB_NAME, ausb->order);
					}

				}else{
					DEBUG_EVENT("%s%d: Invalid Ringing Amplitude value %d\n", AMF_USB_NAME, ausb->order, ausb->ringampl);
				}
			}else{
				__amf_usb_fxo_write(ausb, mod_no, 74, 0x3f);
				if (amf_usb_proslic_setreg_indirect(ausb, mod_no, 21, 0x1d1, fast)){
					return -1;
				}
				DEBUG_EVENT("%s%d: Boosting ringer (89V peak)\n", AMF_USB_NAME, ausb->order);
			}
		} else if (ausb->lowpower != 0){
			if (amf_usb_proslic_setreg_indirect(ausb, mod_no, 21, 0x108, fast)){
				return -1;
			}
			DEBUG_EVENT("%s: Module %d: Reducing ring power (50V peak)\n", AMF_USB_NAME, ausb->order);
		}
	}

	/* Adjust RX/TX gains */
	if (ausb->fxs_txgain || ausb->fxs_rxgain) {
		DEBUG_EVENT("%s%d: Adjust TX Gain to %s\n", AMF_USB_NAME, ausb->order,
					(ausb->fxs_txgain == 35) ? "3.5dB":
					(ausb->fxs_txgain == -35) ? "-3.5dB":"0dB");
		value = __amf_usb_fxo_read(ausb, mod_no, 9);
		switch (ausb->fxs_txgain) {
		case 35:
			value |= 0x8;
			break;
		case -35:
			value |= 0x4;
			break;
		case 0:
			break;
		}

		DEBUG_EVENT("%s%d: Adjust RX Gain to %s\n", AMF_USB_NAME, ausb->order,
					(ausb->fxs_rxgain == 35) ? "3.5dB":
					(ausb->fxs_rxgain == -35) ? "-3.5dB":"0dB");
		switch (ausb->fxs_rxgain) {
		case 35:
			value |= 0x2;
			break;
		case -35:
			value |= 0x01;
			break;
		case 0:
			break;
		}
		__amf_usb_fxo_write(ausb, mod_no, 9, value);
	}

	if (!fast){
		/* Disable interrupt while full initialization */
		__amf_usb_fxo_write(ausb, mod_no, 21, 0);
		__amf_usb_fxo_write(ausb, mod_no, 22, 0xFC);
		__amf_usb_fxo_write(ausb, mod_no, 23, 0);

	}

	__amf_usb_fxo_write(ausb, mod_no, 64, 0x1);

	wait_just_a_bit(10, fast);


	//First read always returns 0!
	value = __amf_usb_fxo_read(ausb, mod_no, 81);
	value = __amf_usb_fxo_read(ausb, mod_no, 81);
	if (value < 0x0A){
		DEBUG_EVENT("%s%d: TIP/RING is too low on FXS %d!\n", AMF_USB_NAME, ausb->order, value * 375 / 1000);
		// return -1;
	}

	A_DEBUG_USB("%s%d: Current Battery1 %dV, Battery2 %dV\n", AMF_USB_NAME, ausb->order,
					__amf_usb_fxo_read(ausb, mod_no, 82)*375/1000,
					__amf_usb_fxo_read(ausb, mod_no, 83)*375/1000);

	A_DEBUG("%s%d: PROSLIC INITIAL CONFIGURATION: Done!\n", AMF_USB_NAME, ausb->order);
	{
		u8 reg8;
		u8 reg6;
//		__amf_usb_cpu_read(ausb, 8, &reg8);		/* Loopback inside proslic */
//printk(KERN_ERR "chunk len: %X\n", reg8);
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
//__amf_usb_cpu_write(ausb, 8, 1);		/* Loopback inside proslic */

/*
//RS>>> Read registers:
{
	int i, j = 0;
	for(i = 0; i < 109; i++){
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
#if 0
	DEBUG_EVENT("...Here we can print REGISTERS and INDIRECT REGISTERS:\n");
	DEBUG_EVENT("...DIRECT REGISTERS:\n");
	for (i = 0; i < 7; i++) {
		res = 0xFF;
		res = __amf_usb_fxo_read(ausb, mod_no, i);
		DEBUG_EVENT("%d   0x%02x\n", i, res);
	}
	for (i = 8; i < 12; i++) {
		res = 0xFF;
		res = __amf_usb_fxo_read(ausb, mod_no, i);
		DEBUG_EVENT("%d   0x%02x\n", i, res);
	}
	for (i = 14; i < 16; i++) {
		res = 0xFF;
		res = __amf_usb_fxo_read(ausb, mod_no, i);
		DEBUG_EVENT("%d   0x%02x\n", i, res);
	}
	for (i = 18; i < 53; i++) {
		res = 0xFF;
		res = __amf_usb_fxo_read(ausb, mod_no, i);
		DEBUG_EVENT("%d   0x%02x\n", i, res);
	}
	for (i = 63; i < 109; i++) {
		res = 0xFF;
		res = __amf_usb_fxo_read(ausb, mod_no, i);
		DEBUG_EVENT("%d   0x%02x\n", i, res);
	}
	DEBUG_EVENT("...INDIRECT REGISTERS:\n");
	for (i = 0; i < 42; i++) {
		res16 = 0xFFFF;
		res16 = amf_usb_proslic_getreg_indirect(ausb, mod_no, i, fast);
		DEBUG_EVENT("%d   0x%04x\n", i, res16);
	}
	for (i = 43; i < 44; i++) {
		res16 = 0xFFFF;
		res16 = amf_usb_proslic_getreg_indirect(ausb, mod_no, i, fast);
		DEBUG_EVENT("%d   0x%04x\n", i, res16);
	}
	for (i = 99; i < 105; i++) {
		res16 = 0xFFFF;
		res16 = amf_usb_proslic_getreg_indirect(ausb, mod_no, i, fast);
		DEBUG_EVENT("%d   0x%04x\n", i, res16);
	}
	//DEBUG_EVENT("...Printing REGISTERS: END\n");
#endif

	return 0;

}

//===============================================================================================================================  INIT   PROSLIC END
//===============================================================================================================================  INIT  PROSLIC END
//===============================================================================================================================  INIT PROSLIC END



//====================================================================================================================  Recheck Sanity

void amf_proslic_recheck_sanity(amf_usb_t *ausb, int mod_no) {

	int res;

	res = ausb->regs[mod_no][64];

	if (!res && (res != ausb->fx[mod_no].fxs.lasttxhook)) {

		res = ausb->regs[mod_no][8];

		if (res) {
			DEBUG_EVENT("%s:%d: Proslic reinitialization: (%02X)\n", AMF_USB_NAME, ausb->order, res);
			amf_usb_init_proslic(ausb, mod_no, 1, 1);
		} else {
			if (ausb->fx[mod_no].fxs.palarms++ < MAX_ALARMS) {
				DEBUG_EVENT("%s%d: Power alarm, resetting!\n", AMF_USB_NAME, ausb->order);
				if (ausb->fx[mod_no].fxs.lasttxhook == 4) {
					ausb->fx[mod_no].fxs.lasttxhook = 1;
				}

				__amf_usb_fxo_write(ausb, mod_no, 64, ausb->fx[mod_no].fxs.lasttxhook);

			} else {
				if (ausb->fx[mod_no].fxs.palarms == MAX_ALARMS) {
					DEBUG_EVENT("%s%d: Too many power alarms, NOT resetting!\n", AMF_USB_NAME, ausb->order);
				}
			}
		}
	}

	return;
}


//====================================================================================================================  Check OHT timer

void amf_proslic_check_ohttimer(amf_usb_t *ausb, int mod_no) {

	int err;

	if (ausb->fx[mod_no].fxs.lasttxhook == 0x4) {
		/* RINGing, prepare for OHT */
		ausb->fx[mod_no].fxs.ohttimer = OHT_TIMER << 3;

		if (ausb->reversepolarity){
			/* OHT mode when idle */
			ausb->fx[mod_no].fxs.idletxhookstate = 0x6;
		} else {
			ausb->fx[mod_no].fxs.idletxhookstate = 0x2;
		}

	} else {

		if (ausb->fx[mod_no].fxs.ohttimer) {
			ausb->fx[mod_no].fxs.ohttimer-= AMF_USB_CHUNKSIZE;
			if (!ausb->fx[mod_no].fxs.ohttimer) {
				if (ausb->reversepolarity){
					/* Switch to active */
					ausb->fx[mod_no].fxs.idletxhookstate = 0x5;
				} else {
					ausb->fx[mod_no].fxs.idletxhookstate = 0x1;
				}

				if ((ausb->fx[mod_no].fxs.lasttxhook == 0x2) || (ausb->fx[mod_no].fxs.lasttxhook == 0x6)) {
					/* Apply the change if appropriate */
					if (ausb->reversepolarity){
						ausb->fx[mod_no].fxs.lasttxhook = 0x5;
					} else {
						ausb->fx[mod_no].fxs.lasttxhook = 0x1;
					}

					A_DEBUG_USB("%s%d: OHT timer: Writing to SLIC (Reg 64): %d\n", AMF_USB_NAME, ausb->order, ausb->fx[mod_no].fxs.lasttxhook);
					err = __amf_usb_fxo_write(ausb, mod_no, 64, ausb->fx[mod_no].fxs.lasttxhook);

				}
			}
		}
	}

	return;
}


//====================================================================================================================  Check Hook

void amf_proslic_check_hook(amf_usb_t *ausb, int mod_no) {

	int		hook;
	char	res;

	/* For some reason we have to debounce the hook detector.  */

	res = ausb->regs[mod_no][68];
	hook = (res & 1);

	if (hook != ausb->fx[mod_no].fxs.lastrxhook) {
		/* Reset the debounce (must be multiple of 4ms) */
		ausb->fx[mod_no].fxs.debounce = 4 * (4 * 8);
		A_DEBUG_USB("%s%d: Resetting debounce hook %d, %d\n", AMF_USB_NAME, ausb->order, hook, ausb->fx[mod_no].fxs.debounce);
	} else {
		if (ausb->fx[mod_no].fxs.debounce > 0) {
			ausb->fx[mod_no].fxs.debounce-= 16 * DAHDI_CHUNKSIZE;
			A_DEBUG_USB("%s%d: Sustaining hook %d, %d\n", AMF_USB_NAME, ausb->order, hook, ausb->fx[mod_no].fxs.debounce);
			if (!ausb->fx[mod_no].fxs.debounce) {
				A_DEBUG_USB("%s%d: Counted down debounce, newhook: %d\n", AMF_USB_NAME, ausb->order, hook);
				ausb->fx[mod_no].fxs.debouncehook = hook;
			}
			if (!ausb->fx[mod_no].fxs.oldrxhook && ausb->fx[mod_no].fxs.debouncehook) {
				/* Off hook */
				A_DEBUG_USB("%s%d: Proslic check_hook: goes off-hook\n", AMF_USB_NAME, ausb->order);
				dahdi_hooksig(&ausb->chans[mod_no], DAHDI_RXSIG_OFFHOOK);
//print_dahdi_call(ausb, "dahdi_hooksig");
				ausb->fx[mod_no].fxs.oldrxhook = 1;
				//	CPU loopback on OFFHOOK:
				//	DEBUG_EVENT("%s%d: Activating CPU loopback on OFFHOOK...\n", AMF_USB_NAME, ausb->order);
				//	__amf_usb_cpu_write(ausb, 8, 0x01);

			} else if (ausb->fx[mod_no].fxs.oldrxhook && !ausb->fx[mod_no].fxs.debouncehook) {
				/* On hook */
				A_DEBUG_USB("%s%d: Proslic check_hook: goes on-hook\n", AMF_USB_NAME, ausb->order);
				dahdi_hooksig(&ausb->chans[mod_no], DAHDI_RXSIG_ONHOOK);
//print_dahdi_call(ausb, "dahdi_hooksig");
				ausb->fx[mod_no].fxs.oldrxhook = 0;
				//	CPU loopback on ONHOOK:
				//	DEBUG_EVENT("%s%d: Stopping CPU loopback on ONHOOK...\n", AMF_USB_NAME, ausb->order);
				//	__amf_usb_cpu_write(ausb, 8, 0x00);
			}
		}
	}
	ausb->fx[mod_no].fxs.lastrxhook = hook;

	return;
}


//====================================================================================================================  FXS READ / WRITE
// u_int8_t __amf_usb_fxo_read(amf_usb_t *ausb, int mod_no, unsigned char off)
//      int __amf_usb_fxo_write(amf_usb_t *ausb, int mod_no, unsigned char off, unsigned char data)





//====================================================================================================================  INDIRECT REGISTERS OPERATIONS




static int amf_usb_proslic_init_indirect_regs(amf_usb_t *ausb, int mod_no, int fast)
{
	unsigned char i;

	for (i = 0; i < sizeof(indirect_regs) / sizeof(indirect_regs[0]); i++){

		if(amf_usb_proslic_setreg_indirect(ausb, mod_no, indirect_regs[i].address, indirect_regs[i].initial, fast)) {
			A_DEBUG_USB("%s%d: Initializing Indirect Register Failed: Reg:%d  initial:%04X\n", AMF_USB_NAME, ausb->order, indirect_regs[i].address, indirect_regs[i].initial);
			return -1;
		}
	}
	return 0;
}


static int amf_usb_proslic_setreg_indirect(amf_usb_t *ausb, int mod_no, unsigned char address, unsigned short data, int fast)
{
	int res = -1;

	A_DEBUG_TX("%s%d: Setting Indirect Register %d = %04X\n", AMF_USB_NAME, ausb->order, address, data);

	// Traditional way
	#if 0
	__amf_usb_fxo_write(ausb, mod_no, IDA_LO,(unsigned char)(data & 0xFF));
	__amf_usb_fxo_write(ausb, mod_no, IDA_HI,(unsigned char)((data & 0xFF00)>>8));
	__amf_usb_fxo_write(ausb, mod_no, IAA, address);
	/* ProSLIC indirect access updates at 16Khz */
	for (i = 0; i < 100; i++) {
		amf_delay(10);
		if (!(__amf_usb_fxo_read(ausb, mod_no, 31) & 0x1)) {
			break;
		}
	}
	if (__amf_usb_fxo_read(ausb, mod_no, 31) & 0x1) {
		A_DEBUG("%s%d: Critical error: ProSLIC indirect write failed (add:0x%X)\n",	AMF_USB_NAME, ausb->order, address);
		return -EINVAL;
	}
	#endif

	// New wave:
	res = __amf_usb_fxs_write_indirect(ausb, mod_no, address,(unsigned short)(data & 0xFFFF), fast);
	// no 100ms delay here: we have it in read_indirect sub (max. 1 sec)

	if (res) {
		DEBUG_EVENT("%s%d: SLIC indirect access failed on Set\n", AMF_USB_NAME, ausb->order);
	}

	return res;

}


static int amf_usb_proslic_getreg_indirect(amf_usb_t *ausb, int mod_no, unsigned char address, int fast)
{
//	int i;
	int res = -1;
//	unsigned char data1, data2;
	long err_start;
	long err_end;

	A_DEBUG_RX("%s%d: Getting Indirect Register %d\n", AMF_USB_NAME, ausb->order, address);
	err_start = ausb->stats.slic_indirect_access_err_cnt;

	// Traditional way
	#if 0
	__amf_usb_fxo_write(ausb, mod_no, IAA, address);
	for (i = 0; i < 100; i++) {
		amf_delay(10);
		if (!(__amf_usb_fxo_read(ausb, mod_no, 31) & 0x1)) {
			break;
		}
	}
	if (__amf_usb_fxo_read(ausb, mod_no, 31) & 0x1) {
		A_DEBUG("%s%d: Critical error:ProSLIC indirect read failed\n", AMF_USB_NAME, ausb->order);
		return -EINVAL;
	}
	data1 = __amf_usb_fxo_read(ausb, mod_no, IDA_LO);
	data2 = __amf_usb_fxo_read(ausb, mod_no, IDA_HI);
	res = data1 | (data2 << 8);
	#endif

	// New wave:
	res = __amf_usb_fxs_read_indirect(ausb, mod_no, address, fast);
	// no 100ms delay here: we have it in read_indirect sub (max. 1 sec)

	err_end = ausb->stats.slic_indirect_access_err_cnt;
	if (err_end - err_start) {
		DEBUG_EVENT("%s%d: SLIC indirect access failed on Get: errors=%ld\n", AMF_USB_NAME, ausb->order, err_end - err_start);
		return -1;
	}

	return res;
}



static int __amf_usb_fxs_write_indirect(amf_usb_t *ausb, int mod_no, unsigned char off, unsigned short data, int fast)
{
	struct sk_buff		*skb;
	u8					*cmd_data;
	int				retry = 0;
	int res = -1;

	return amf_usb_write_value(WR_FXS_COMMAND, ausb, off, data&0xFF, (data>>8)&0xFF, 100);

	A_BUG(ausb == NULL);
	spin_lock(&ausb->cmd_lock);

	if (!amf_skb_queue_len(&ausb->tx_cmd_free_list)){
		A_DEBUG("%s%d: WARNING: USB FX Write Command Overrun (%d commands in process)!\n", AMF_USB_NAME, ausb->order,
					amf_skb_queue_len(&ausb->tx_cmd_list));
		ausb->stats.cmd_overrun++;
		spin_unlock(&ausb->cmd_lock);
		return 0;
	}
	skb = amf_skb_dequeue(&ausb->tx_cmd_free_list);
	cmd_data = amf_skb_put(skb, 4);
	cmd_data[0] = AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_WRITE_FXS_INDIRECT) | mod_no;
	cmd_data[1] = off;
	cmd_data[2] = data & 0xFF;
	cmd_data[3] = (data >> 8) & 0xFF;

	amf_skb_queue_tail(&ausb->tx_cmd_list, skb);

	// now we want to get acknowledge:

	do {

		wait_just_a_bit(AMF_USBFXS_READ_INDIRECT_DELAY, fast);
		if (++retry > AMF_USBFXS_READ_INDIRECT_RETRIES) break;

	} while(!amf_skb_queue_len(&ausb->rx_cmd_list));

	if (!amf_skb_queue_len(&ausb->rx_cmd_list)){
		A_DEBUG("%s%d: WARNING: Timeout on Write USB-FXS Indirect Reg!\n", AMF_USB_NAME, ausb->order);
		ausb->stats.cmd_timeout++;
		goto fxs_write_done;
	}

	skb = amf_skb_dequeue(&ausb->rx_cmd_list);
	cmd_data = amf_skb_data(skb);
	if (cmd_data[1] != off){
		A_DEBUG("%s%d: USB FXS Write Indirect response is out of order (%02X:%02X)!\n",	AMF_USB_NAME, ausb->order,
                   cmd_data[1], off);
		ausb->stats.cmd_invalid++;
		goto fxs_write_done;
	}

	if(cmd_data[2]) {
		res = -1;
		ausb->stats.cmd_invalid++;
	} else {
		res = 0;
	}

	amf_skb_init(skb, 0);
	amf_skb_queue_tail(&ausb->rx_cmd_free_list, skb);

fxs_write_done:
	amf_clear_bit(AMF_USB_STATUS_TX_CMD, &ausb->status);
	spin_unlock(&ausb->cmd_lock);
	return res;
}


static u_int16_t __amf_usb_fxs_read_indirect(amf_usb_t *ausb, int mod_no, unsigned char off, int fast)
{
	struct sk_buff	*skb;
	int				retry = 0;
	u16				data = 0xFFFF;
	u8				*cmd_data;
	int retval;

	retval = amf_usb_read_value(RD_FXS_COMMAND, ausb, off, 1000);


	if(0 > retval){
		printk(KERN_ERR "Read error Proslic: %d\n", off);
	}

	return retval; 

	A_BUG(ausb == NULL);
	spin_lock(&ausb->cmd_lock);

	if (amf_test_and_set_bit(AMF_USB_STATUS_TX_CMD, &ausb->status)){
		A_DEBUG("%s%d: WARNING: USB FXS Read Command Overrun (Read command in process)!\n", AMF_USB_NAME, ausb->order);
		ausb->stats.cmd_overrun++;
		goto fxs_read_done;
	}

	if (!amf_skb_queue_len(&ausb->tx_cmd_free_list)){
		A_DEBUG("%s%d: WARNING: USB FXS Read Command Overrun (%d commands in process)!\n",
					AMF_USB_NAME, ausb->order, amf_skb_queue_len(&ausb->tx_cmd_list));
		ausb->stats.cmd_overrun++;
		goto fxs_read_done;
	}
	skb = amf_skb_dequeue(&ausb->tx_cmd_free_list);
	cmd_data = amf_skb_put(skb, 2);
	cmd_data[0] = AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_READ_FXS_INDIRECT) | mod_no;
	cmd_data[1] = off;
	amf_skb_queue_tail(&ausb->tx_cmd_list, skb);

	do {

		wait_just_a_bit(AMF_USBFXS_READ_INDIRECT_DELAY, fast);
		if (++retry > AMF_USBFXS_READ_INDIRECT_RETRIES) break;

	} while(!amf_skb_queue_len(&ausb->rx_cmd_list));

	if (!amf_skb_queue_len(&ausb->rx_cmd_list)){
		A_DEBUG("%s%d: WARNING: Timeout on Read USB-FXS Indirect Reg!\n", AMF_USB_NAME, ausb->order);
		ausb->stats.cmd_timeout++;
		goto fxs_read_done;
	}

	skb = amf_skb_dequeue(&ausb->rx_cmd_list);
	cmd_data = amf_skb_data(skb);
	if (cmd_data[1] != off){
		A_DEBUG("%s%d: USB FXO Read response is out of order (%02X:%02X)!\n",	AMF_USB_NAME, ausb->order,
                   cmd_data[1], off);
		ausb->stats.cmd_invalid++;
		goto fxs_read_done;
	}

	data = (unsigned char)cmd_data[2];
	data = data | ((unsigned char)cmd_data[3] << 8);

	amf_skb_init(skb, 0);
	amf_skb_queue_tail(&ausb->rx_cmd_free_list, skb);

fxs_read_done:
	amf_clear_bit(AMF_USB_STATUS_TX_CMD, &ausb->status);
	spin_unlock(&ausb->cmd_lock);
	return data;
}

