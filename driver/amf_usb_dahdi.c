 /*
  *  amf_usb_dahdi.c - 'dahdi related functions'
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

#ifndef __AMF_USB_DAHDI__
#define __AMF_USB_DAHDI__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>
#include "amf_usb.h"
#include "amf_usb_utils.h"


//============================================================================================= HOOKSIG ================

static int amf_ioctl(struct dahdi_chan *chan, unsigned int cmd, unsigned long data)
{
	amf_usb_t *ausb = chan->pvt;
	int err = 0;
	int x;

	switch(cmd) {

	case DAHDI_ONHOOKTRANSFER:


		if (ausb->mod_type[0] == AMF_USB_FXO) {
			return -EINVAL;
		}

		err = copy_from_user(&x, (int*)data, sizeof(int));
		if (err) {
			return -EFAULT;
		}
		ausb->fx[chan->chanpos - 1].fxs.ohttimer = x << 3;
		if (ausb->reversepolarity){
			/* OHT mode when idle */
			ausb->fx[chan->chanpos - 1].fxs.idletxhookstate  = 0x6;
		}else{
			ausb->fx[chan->chanpos - 1].fxs.idletxhookstate  = 0x2;
		}
		if (ausb->fx[chan->chanpos - 1].fxs.lasttxhook == 0x1) {
			/* Apply the change if appropriate */
			if (ausb->reversepolarity){
				ausb->fx[chan->chanpos - 1].fxs.lasttxhook = 0x6;
			}else{
				ausb->fx[chan->chanpos - 1].fxs.lasttxhook = 0x2;
			}

			__amf_usb_fxo_write(ausb, chan->chanpos - 1, 64, ausb->fx[chan->chanpos - 1].fxs.lasttxhook);
		}
		break;

	case DAHDI_SETPOLARITY:


		if (ausb->mod_type[0] == AMF_USB_FXO) {
			return -EINVAL;
		}

		err = copy_from_user(&x, (int*)data, sizeof(int));
		if (err) {
			return -EFAULT;
		}
		/* Can't change polarity while ringing or when open */
		if ((ausb->fx[chan->chanpos - 1].fxs.lasttxhook == 0x04) ||
		    (ausb->fx[chan->chanpos - 1].fxs.lasttxhook == 0x00)){
			return -EINVAL;
		}

		if ((x && !ausb->reversepolarity) || (!x && ausb->reversepolarity)){
			ausb->fx[chan->chanpos - 1].fxs.lasttxhook |= 0x04;
		}else{
			ausb->fx[chan->chanpos - 1].fxs.lasttxhook &= ~0x04;
		}

		__amf_usb_fxo_write(ausb, chan->chanpos - 1, 64, ausb->fx[chan->chanpos - 1].fxs.lasttxhook);
		break;

	case DAHDI_TONEDETECT:

		// DTMF issue: chan_dahdi asks about our tonedetect mode.
		// We should answer (no HW Tone Support, so i->hardwaredtmf = 0 => dahdi_pvt->dsp = yes!):
		return -ENOSYS;

	default:
		return -ENOTTY;
	}
	return 0;
}


static int amf_hooksig(struct dahdi_chan *chan, enum dahdi_txsig txsig)
{
	amf_usb_t *ausb = chan->pvt;
	int err = 0;


	if (ausb->mod_type[0] == AMF_USB_FXO) {

		switch(txsig) {

		case DAHDI_TXSIG_START:
			break;

		case DAHDI_TXSIG_OFFHOOK:
			A_DEBUG("%s%d: goes off-hook (txsig %d)\n", AMF_USB_NAME, ausb->order, txsig);
			ausb->fx[chan->chanpos - 1].fxo.offhook = 1;
			err = __amf_usb_fxo_write(ausb, chan->chanpos - 1, 5, 0x9);

			break;

		case DAHDI_TXSIG_ONHOOK:
			A_DEBUG("%s%d: goes on-hook (txsig %d)\n", AMF_USB_NAME, ausb->order, txsig);
			ausb->fx[chan->chanpos - 1].fxo.offhook = 0;
			err = __amf_usb_fxo_write(ausb, chan->chanpos - 1, 5, 0x8);
			// ausb->time_to_reset_fifo_pointers = 1; 	// FXO has this operation in firmware!

			break;

		default:
			A_DEBUG("%s%d: Can't set tx state to %d (chan %d)\n",	AMF_USB_NAME, ausb->order, txsig, chan->chanpos);
		}

	} else {


		switch(txsig) {

		case DAHDI_TXSIG_ONHOOK:

			A_DEBUG_USB("%s%d: txsig ONHOOK (txsig %d)\n", AMF_USB_NAME, ausb->order, txsig);
			switch(chan->sig) {
			case DAHDI_SIG_EM:
			case DAHDI_SIG_FXOKS:
			case DAHDI_SIG_FXOLS:

				A_DEBUG_USB("%s%d: txsig ONHOOK  & FXOLS (txsig %d, lasttxhook %d -> idletxhookstate %d)\n", AMF_USB_NAME, ausb->order, txsig,
																	ausb->fx[chan->chanpos - 1].fxs.lasttxhook, ausb->fx[chan->chanpos - 1].fxs.idletxhookstate);

				ausb->fx[chan->chanpos - 1].fxs.lasttxhook = ausb->fx[chan->chanpos - 1].fxs.idletxhookstate;
				break;
			case DAHDI_SIG_FXOGS:

				A_DEBUG_USB("%s%d: txsig ONHOOK  & FXOGS (txsig %d)\n", AMF_USB_NAME, ausb->order, txsig);

				ausb->fx[chan->chanpos - 1].fxs.lasttxhook = 3;
				break;
			}
			break;
		case DAHDI_TXSIG_OFFHOOK:

			ausb->time_to_reset_fifo_pointers = 1;

			A_DEBUG_USB("%s%d: txsig OFFHOOK (txsig %d, lasttxhook %d -> idletxhookstate %d)\n", AMF_USB_NAME, ausb->order, txsig,
																	ausb->fx[chan->chanpos - 1].fxs.lasttxhook, ausb->fx[chan->chanpos - 1].fxs.idletxhookstate);

			switch(chan->sig) {
			case DAHDI_SIG_EM:

				A_DEBUG_USB("%s%d: txsig OFFHOOK & SIG_EM (txsig %d, lasttxhook %d -> 5)\n", AMF_USB_NAME, ausb->order, txsig,
																	ausb->fx[chan->chanpos - 1].fxs.lasttxhook);

				ausb->fx[chan->chanpos - 1].fxs.lasttxhook = 5;
				break;
			default:

				A_DEBUG_USB("%s%d: txsig OFFHOOK & default: (txsig %d, lasttxhook %d -> idletxhookstate %d)\n", AMF_USB_NAME, ausb->order, txsig,
																	ausb->fx[chan->chanpos - 1].fxs.lasttxhook, ausb->fx[chan->chanpos - 1].fxs.idletxhookstate);

				ausb->fx[chan->chanpos - 1].fxs.lasttxhook = ausb->fx[chan->chanpos - 1].fxs.idletxhookstate;

				break;
			}
			break;
		case DAHDI_TXSIG_START:

			A_DEBUG_USB("%s%d: txsig START (txsig %d).\n", AMF_USB_NAME, ausb->order, txsig);
			ausb->fx[chan->chanpos - 1].fxs.lasttxhook = 4;
			break;
		case DAHDI_TXSIG_KEWL:
			ausb->fx[chan->chanpos - 1].fxs.lasttxhook = 0;
			break;
		default:
			DEBUG_EVENT("%s%d: Can't set tx state to %d\n", AMF_USB_NAME, ausb->order, txsig);
			return 0;
			break;
		}

		A_DEBUG_USB("%s%d: Writing to SLIC (Reg 64): %d\n", AMF_USB_NAME, ausb->order, ausb->fx[chan->chanpos - 1].fxs.lasttxhook);
		err = __amf_usb_fxo_write(ausb, chan->chanpos - 1, 64, ausb->fx[chan->chanpos - 1].fxs.lasttxhook);

	}

	return 0;
}


static int amf_watchdog(struct dahdi_span *span, int event)
{
	return 0;
}


void amf_dahdi_sync_tick(struct dahdi_span *span, int is_master)
{
	struct dahdi_chan *chan = 	span->chans[0];
	amf_usb_t *ausb = 			chan->pvt;

	ausb->dahdi_ticks++;

#if 1
	if ((ausb->dahdi_ticks % 60000 == 0)){

		if (ausb->mod_type[0] == AMF_USB_FXO) {
			A_DEBUG_USB("%s%d: Status Regs: 5:%02X, 29:%02X, 34:%02X, 4:%02X   jf %ld\n", AMF_USB_NAME, ausb->order,
							  ausb->regs[0][5], ausb->regs[0][29], ausb->regs[0][34], ausb->regs[0][4], jiffies);
		} else {
			A_DEBUG_USB("%s%d: Status Regs: 24:%02X, 19:%02X, 20:%02X, 64:%02X, 68:%02X, 8:%02X  T: %ld  T_max: %d   jf %ld\n", AMF_USB_NAME, ausb->order,
					ausb->regs[0][24], ausb->regs[0][19], ausb->regs[0][20], ausb->regs[0][64], ausb->regs[0][68], ausb->regs[0][8],
					ausb->temperature, ausb->temperature_max, jiffies);
		}

		A_DEBUG_USB("%s%d: dahdi_ticks: %ld  rxcount: %ld  rxbytes: %ld  bhcount: %ld  txcount: %ld  usb_master: %ld  jf %ld\n", AMF_USB_NAME, ausb->order,
				  ausb->dahdi_ticks, ausb->rxcount, ausb->rxbytes, ausb->bhcount, ausb->txcount, ausb->usb_master, jiffies);

		if (ausb->mod_type[0] == AMF_USB_FXS) {
			A_DEBUG_USB("%s%d: MCPU status: 0x%02X, errs: %ld MCPU2(m/s): 0x%02X  RX: 0x(%02X:%02X:%02X)  TX: 0x(%02X:%02X:%02X) errs: %ld   %ld\n", AMF_USB_NAME, ausb->order,
				ausb->regs[0][119], ausb->stats.mcpu_status_err_cnt,
				ausb->regs[0][126],
				ausb->regs[0][120], ausb->regs[0][121], ausb->regs[0][122],
 				ausb->regs[0][123], ausb->regs[0][124], ausb->regs[0][125],
				ausb->stats.mcpu_queue_threshold_err_cnt,
				jiffies);
		}

	}
#endif


#ifdef AMF_USB_MODE_SLAVE

	/* SLAVE tasklet: sync verify and indexes calculations are in rx_completion or bh_rx... */
	if (ausb->master_span_on){
		tasklet_schedule(&ausb->bh_task);
	}

#endif

	return;
}



static int amf_open(struct dahdi_chan *chan)
{
	amf_usb_t *ausb = chan->pvt;
	if (ausb->dead)
		return -ENODEV;
	ausb->usecount++;
	A_DEBUG("%s%d: Open (usecount=%d, channo=%d, chanpos=%d)\n", AMF_USB_NAME, ausb->order,	ausb->usecount, chan->channo, chan->chanpos);
	return 0;
}

static int amf_close(struct dahdi_chan *chan)
{
	amf_usb_t *ausb = chan->pvt;
	ausb->usecount--;
	return 0;
}

//=============================================================================================  SPAN ================


static const struct dahdi_span_ops amf_usb_ops = {
#if DAHDI_VERSION >= 24
	.owner = THIS_MODULE,
#endif
	.open = amf_open,
	.close = amf_close,
	.hooksig = amf_hooksig,
	.ioctl = amf_ioctl,
	.watchdog =	amf_watchdog,
	.sync_tick = amf_dahdi_sync_tick,
};

int amf_usb_dahdi_register(amf_usb_t *ausb) {

	int err;

	// if we're in a Slave mode - use our Masterspan for timing:
#ifdef AMF_USB_MODE_SLAVE
	ausb->span.cannot_provide_timing = 1;
#else
	ausb->span.cannot_provide_timing = 0;
#endif

	if (ausb->amf_device_type == AMF_USB_FXO) {
		sprintf(ausb->span.name, "AMF_USB_FXO_%d", ausb->order);
		sprintf(ausb->span.desc, "AMF_USB_FXO %d", ausb->order);
#if DAHDI_VERSION >= 27
		ausb->span.spantype = SPANTYPE_ANALOG_FXO;
#endif
	} else {
		sprintf(ausb->span.name, "AMF_USB_FXS_%d", ausb->order);
		sprintf(ausb->span.desc, "AMF_USB_FXS %d", ausb->order);
#if DAHDI_VERSION >= 27
		ausb->span.spantype = SPANTYPE_ANALOG_FXS;
#endif
	}

	/*snprintf(ausb->span.desc, sizeof(ausb->span.desc) - 1, "%s %d", ausb->span.name, ausb->order);*/

#if DAHDI_VERSION <= 25
	ausb->span.manufacturer	= "Amfeltec";
#endif
	ausb->chan_ptrs[0] = &ausb->chans[0];

	if (ausb->amf_device_type == AMF_USB_FXO) {
		sprintf(ausb->chans[0].name, "AMF_USB_FXO/%d/%d", ausb->order, 1);

		ausb->chans[0].sigcap =	DAHDI_SIG_FXSKS |
								DAHDI_SIG_FXSLS |
								DAHDI_SIG_SF    |
								DAHDI_SIG_CLEAR;

	} else {
		sprintf(ausb->chans[0].name, "AMF_USB_FXS/%d/%d", ausb->order, 1);

		ausb->chans[0].sigcap =	DAHDI_SIG_FXOKS |
								DAHDI_SIG_FXOLS |
								DAHDI_SIG_FXOGS |
								DAHDI_SIG_SF    |
								DAHDI_SIG_EM    |
								DAHDI_SIG_CLEAR;
	}

#if DAHDI_VERSION >= 26
	ausb->ddev = dahdi_create_device();
	dev_set_name(&ausb->ddev->dev, ausb->device_name);
	ausb->ddev->manufacturer = "Amfeltec";
	ausb->ddev->devicetype = "Amfeltec USB device";
	ausb->ddev->location = kasprintf(GFP_KERNEL, "USB bus");
#endif

	ausb->chans[0].chanpos = 1;
	ausb->chans[0].pvt = ausb;
	ausb->span.chans 		= ausb->chan_ptrs;
	ausb->span.channels 	= 1;

#if DAHDI_VERSION == 23
	dahdi->span.owner = THIS_MODULE;
#endif
#if DAHDI_VERSION < 25
	init_waitqueue_head(&ausb->span.maintq);
#endif
#if DAHDI_VERSION < 24
	ausb->span.pvt = ausb;
#endif

	if (ausb->alawoverride) {
		ausb->span.deflaw 		= DAHDI_LAW_ALAW;			// Europe
	}
	else {
		ausb->span.deflaw 		= DAHDI_LAW_MULAW;			// America
	}


	ausb->battthresh		= DEFAULT_BATT_THRESH;
	ausb->battdebounce		= DEFAULT_BATT_DEBOUNCE;

#if DAHDI_VERSION >= 24
	ausb->span.ops			= &amf_usb_ops;
#endif

	ausb->span.flags		= DAHDI_FLAG_RBS;           // Robbed-bit signalling, otherwise no hooksigs!

#if DAHDI_VERSION >= 26
	list_add_tail(&ausb->span.device_node, &ausb->ddev->spans);
	err = dahdi_register_device(ausb->ddev, NULL);
//print_dahdi_call(ausb, "dahdi_register_device");

#else
	err = dahdi_register(&ausb->span, 0);
//print_dahdi_call(ausb, "dahdi_register");
#endif

	return err;

}


int amf_usb_dahdi_unregister(amf_usb_t *ausb) {

#if DAHDI_VERSION >= 26
	dahdi_unregister_device(ausb->ddev);
//print_dahdi_call(ausb, "print_dahdi_call");
#else
	dahdi_unregister(&ausb->span);
//print_dahdi_call(ausb, "dahdi_unregister");
#endif

	return 0;
}


//============================================================================================= MASTER SPAN ================


static const struct dahdi_span_ops amf_usb_ops_master = {
#if DAHDI_VERSION >= 24
	.owner = THIS_MODULE,
#endif
};


int amf_usb_dahdi_register_master(amf_usb_t *ausb) {

	int err;

	ausb->span.cannot_provide_timing = 0;

	sprintf(ausb->span_master.name, "AMF_USB_MASTER");
	sprintf(ausb->span_master.desc, "AMF_USB_MASTER");

#if DAHDI_VERSION >= 27
	ausb->span_master.spantype = SPANTYPE_ANALOG_FXO;
#endif

#if DAHDI_VERSION <= 25
	ausb->span_master.manufacturer	= "Amfeltec";
#endif

#if DAHDI_VERSION >= 26
	ausb->ddev_master = dahdi_create_device();
	dev_set_name(&ausb->ddev_master->dev, ausb->device_name_master);
	ausb->ddev_master->manufacturer = "Amfeltec";
	ausb->ddev_master->devicetype = "Amfeltec USB device";
	ausb->ddev_master->location = kasprintf(GFP_KERNEL, "USB bus");
#endif

	ausb->span_master.channels 	= 0;

#if DAHDI_VERSION == 23
	dahdi->span_master.owner = THIS_MODULE;
#endif
#if DAHDI_VERSION < 25
	init_waitqueue_head(&ausb->span_master.maintq);
#endif
#if DAHDI_VERSION < 24
	ausb->span_master.pvt = ausb;
#endif

#if DAHDI_VERSION >= 24
	ausb->span_master.ops			= &amf_usb_ops_master;
#endif

	if (ausb->alawoverride) {
		ausb->span_master.deflaw 		= DAHDI_LAW_ALAW;			// Europe
	}
	else {
		ausb->span_master.deflaw 		= DAHDI_LAW_MULAW;			// America
	}

#if DAHDI_VERSION >= 26
	list_add_tail(&ausb->span_master.device_node, &ausb->ddev_master->spans);
	err = dahdi_register_device(ausb->ddev_master, NULL);
//print_dahdi_call(ausb, "dahdi_register_device");
#else
	err = dahdi_register(&ausb->span_master, 0);
//print_dahdi_call(ausb, "dahdi_register");
#endif
	return err;

}

int amf_usb_dahdi_unregister_master(amf_usb_t *ausb) {

#if DAHDI_VERSION >= 26
	dahdi_unregister_device(ausb->ddev_master);
//print_dahdi_call(ausb, "dahdi_unregister_device");
#else
	dahdi_unregister(&ausb->span_master);
//print_dahdi_call(ausb, "dahdi_unregister");
#endif
	return 0;
}




#endif  //__AMF_USB_DAHDI__
