 /*
  *  amf_usb_cpu.c - 'cpu related functions'
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

#ifndef __AMF_USB_CPU__
#define __AMF_USB_CPU__


#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

#include "amf_usb.h"
#include "amf_usb_utils.h"

//================================================================================  Microchip dsPIC33 CPU Setup

int amf_usb_set_cpu(amf_usb_t *ausb) {

	int err;
	u8	data8;
	u8	data8_test;
	char dev_type;

	ausb->hw_rev = 0xFF;
	ausb->core_rev = 0xFF;

	wait_just_a_bit(1000, 0);
	err = amf_usb_search_rxsync(ausb);

	if (!err && ausb->rx_sync){

		// Read / Write test
		err = __amf_usb_cpu_read(ausb, AMF_USB_CPU_REG_DEBUG, &data8_test);
		if (err){
			A_DEBUG("%s%d: Failed to read CPU reg (err=%d)!\n", AMF_USB_NAME, ausb->order, err);
			goto sc_error;
		}
		__amf_usb_cpu_write(ausb, AMF_USB_CPU_REG_DEBUG, 0x5A);
		err = __amf_usb_cpu_read(ausb, AMF_USB_CPU_REG_DEBUG, &data8);
		if (err){
			A_DEBUG("%s%d: Failed to read CPU reg (err=%d)!\n", AMF_USB_NAME, ausb->order, err);
			goto sc_error;
		}

		if (data8 == 0x5A) {
			A_DEBUG_USB("%s: CPU Read/Write test: OK! (%02X:%02X)\n", AMF_USB_NAME, 0x5A, data8);
		} else {
			DEBUG_EVENT("%s: CPU Read/Write test: Failed (%02X:%02X)\n", AMF_USB_NAME, 0x5A, data8);
			goto sc_error;
		}
		__amf_usb_cpu_write(ausb, AMF_USB_CPU_REG_DEBUG, data8_test);
		// Read / Write test (end)

		// Hardware ID: high nibble is Device type: FXO:1 FXS:2 PA:3, low is a Rotary Switch
		err = __amf_usb_cpu_read(ausb, AMF_USB_CPU_REG_DEVICEID, &data8);
		if (err){
			goto sc_error;
		}

		ausb->order =  0xF - (data8 & 0xF);
		A_DEBUG("%s%d: Rotary switch setting: %d\n", AMF_USB_NAME, ausb->order, ausb->order);

		// We have already detected device type by ProductID, just check firmware setting here:
		dev_type = (data8 >> 4) & 0xF;
		if (ausb->amf_device_type != dev_type) {
			A_DEBUG("%s: Firmware error: device type mismatch (dr: %d, fw: %d)\n", AMF_USB_NAME, ausb->amf_device_type, dev_type);
			goto sc_error;
		}

		switch(dev_type){
		case AMF_USB_FXO:
			DEBUG_EVENT("%s%d: USB-FXO device\n", AMF_USB_NAME, ausb->order);
			break;
		case AMF_USB_FXS:
			DEBUG_EVENT("%s%d: USB-FXS device\n", AMF_USB_NAME, ausb->order);
			break;
		case AMF_USB_PA:
			DEBUG_EVENT("%s%d: USB-PA device\n", AMF_USB_NAME, ausb->order);
			break;
		default:
			DEBUG_EVENT("%s%d: Unknown USB device (id: %d)\n", AMF_USB_NAME, ausb->order, dev_type);
			goto sc_error;
		}

		err = __amf_usb_cpu_read(ausb, AMF_USB_CPU_REG_HARDWAREVER, &data8);
		if (err){
			DEBUG_EVENT("%s%d: Failed to read USB Hardware Version (err=%d)\n", AMF_USB_NAME, ausb->order, err);
			goto sc_error;
		}
		ausb->hw_rev = data8;
		if ((data8 & 0xF) == 0x3) {
			ausb->failover_enabled = 1;
			if (dev_type == AMF_USB_FXO) {
				DEBUG_EVENT("%s%d: USB Hardware Version: %02X (with Fail-over)\n", AMF_USB_NAME, ausb->order, data8);
			} else {
				DEBUG_EVENT("%s%d: USB Hardware Version: %02X\n", AMF_USB_NAME, ausb->order, data8);
			}
		} else {
			ausb->failover_enabled = 0;
			DEBUG_EVENT("%s%d: USB Hardware Version: %02X\n", AMF_USB_NAME, ausb->order, data8);
		}

		err = __amf_usb_cpu_read(ausb, AMF_USB_CPU_REG_FIRMWAREVER, &data8);
		if (err){
			DEBUG_EVENT("%s%d: Failed to read USB Firmware Version (err=%d)\n", AMF_USB_NAME, ausb->order, err);
			goto sc_error;
		}
		DEBUG_EVENT("%s%d: USB Firmware Version: %02X\n", AMF_USB_NAME, ausb->order, data8);
		ausb->core_rev = data8;

		DEBUG_EVENT("%s%d: USB Driver Version: %s\n", AMF_USB_NAME, ausb->order, AMF_USB_DRIVER_VERSION);

		/* Reset register to default values */
		__amf_usb_cpu_write(ausb, AMF_USB_CPU_REG_CTRL, AMF_USB_CPU_BIT_CTRL_RESET);
		ausb->reg_cpu_ctrl = 0x00;
		// reset: wait for 0.1s
		wait_just_a_bit(100, 0);
	}else{
		A_DEBUG("%s: Not Syncronized...\n", AMF_USB_NAME);
		ausb->hw_rev	= 0xFF;
		ausb->core_rev= 0xFF;
		goto sc_error;
	}

	return 0;

sc_error:
	return err;

}

//========================================================================================= CPU READ / WRITE

int __amf_usb_cpu_read(amf_usb_t *ausb, unsigned char off, unsigned char *data)
{

	struct sk_buff 	*skb;
	int				err = 0, retry = 0;
	u8				*cmd_data;

	err = amf_usb_read_value(RD_CPU_COMMAND,ausb, off, 1000);
	if (0 > err){
		return err;
	} else {
		*data = err & 0xFF;
	}
	return 0;

	A_BUG(ausb == NULL);

	*data = 0xFF;
	if (!amf_test_bit(AMF_USB_STATUS_READY, &ausb->status)){
		A_DEBUG("%s: WARNING: RX USB core is not ready (%ld)\n", AMF_USB_NAME,	(unsigned long)jiffies);
		ausb->stats.core_notready_cnt++;
		return -ENODEV;
	}

	spin_lock(&ausb->cmd_lock);

	if (amf_test_and_set_bit(AMF_USB_STATUS_TX_CMD, &ausb->status)){
		A_DEBUG("%s: WARNING: USB CPU Read Command Overrun (Read command in process)\n", AMF_USB_NAME);
		ausb->stats.cmd_overrun++;
		err = -EBUSY;
		goto cpu_read_done;
	}

	if (!amf_skb_queue_len(&ausb->tx_cmd_free_list)){
		A_DEBUG("%s: WARNING: USB CPU Read Command Overrun (%d commands in process)\n",
					AMF_USB_NAME, amf_skb_queue_len(&ausb->tx_cmd_list));
		ausb->stats.cmd_overrun++;
		err = -EBUSY;
		goto cpu_read_done;
	}
	skb = amf_skb_dequeue(&ausb->tx_cmd_free_list);
	cmd_data = amf_skb_put(skb, 2);
	cmd_data[0] = AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_READ_CPU);
	cmd_data[1] = off;
	amf_skb_queue_tail(&ausb->tx_cmd_list, skb);
	ausb->tx_cmd_start = jiffies;


//	spin_unlock(&ausb->cmd_lock);

	do {
		wait_just_a_bit(AMF_USBFXO_READ_DELAY, 1);
		if (++retry > AMF_USBFXO_READ_RETRIES) break;
	} while(!amf_skb_queue_len(&ausb->rx_cmd_list));

//	spin_lock(&ausb->cmd_lock);

	if (!amf_skb_queue_len(&ausb->rx_cmd_list)){
		A_DEBUG("%s: WARNING: Timeout on Read USB-CPU Reg (%02X:%02X  %d:%ld:%ld:%ld)\n",
					AMF_USB_NAME,
					AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_READ_CPU),
					off,
					retry,
					(unsigned long)(jiffies - ausb->tx_cmd_start),
					(unsigned long)ausb->tx_cmd_start,
					(unsigned long)jiffies);
		ausb->stats.cmd_timeout++;
		err = -EINVAL;
		goto cpu_read_done;
	}
	skb = amf_skb_dequeue(&ausb->rx_cmd_list);
	cmd_data = skb->data;

	if (cmd_data[1] != off){
		A_DEBUG("%s: USB Read response is out of order 0x(waited: %02X:%02X, got %02X:%02X:%02X)\n",	AMF_USB_NAME,
														AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_READ_CPU), off,
														cmd_data[0], cmd_data[1], cmd_data[2]);

		ausb->stats.cmd_invalid++;
		amf_skb_init(skb, 0);
		amf_skb_queue_tail(&ausb->rx_cmd_free_list, skb);
		err = -EINVAL;
		goto cpu_read_done;
	}
	*data = (unsigned char)cmd_data[2];

	amf_skb_init(skb, 0);
	amf_skb_queue_tail(&ausb->rx_cmd_free_list, skb);


cpu_read_done:
	amf_clear_bit(AMF_USB_STATUS_TX_CMD, &ausb->status);
	spin_unlock(&ausb->cmd_lock);
	return err;
}

int __amf_usb_cpu_write(amf_usb_t *ausb, unsigned char off, unsigned char data)
{
	struct sk_buff 	*skb;
	u8		*cmd_data;


	return amf_usb_write_value(WR_CPU_COMMAND, ausb, off, data, 0, 100);

	A_BUG(ausb == NULL);
	spin_lock(&ausb->cmd_lock);

	if (!amf_skb_queue_len(&ausb->tx_cmd_free_list)){
		ausb->stats.cmd_overrun++;
		spin_unlock(&ausb->cmd_lock);
		return -EINVAL;
	}
	skb = amf_skb_dequeue(&ausb->tx_cmd_free_list);
	cmd_data = amf_skb_put(skb, 3);
	cmd_data[0] = AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_WRITE_CPU);
	cmd_data[1] = off;
	cmd_data[2] = data;
	amf_skb_queue_tail(&ausb->tx_cmd_list, skb);

	spin_unlock(&ausb->cmd_lock);
	return 0;
}


int __amf_usb_cpu_init_proslic(amf_usb_t *ausb, unsigned char tx_data, unsigned char *rx_data)
{
	// CPU read analog with write features: PROSLIC framer initialization

	struct sk_buff 	*skb;
	int				err = 0, retry = 0;
	u8				*cmd_data;


	A_BUG(ausb == NULL);

	*rx_data = 0xFF;
	if (!amf_test_bit(AMF_USB_STATUS_READY, &ausb->status)){
		A_DEBUG("%s: WARNING: RX USB core is not ready (%ld)!\n", AMF_USB_NAME,	(unsigned long)jiffies);
		ausb->stats.core_notready_cnt++;
		return -ENODEV;
	}

	spin_lock(&ausb->cmd_lock);

	if (amf_test_and_set_bit(AMF_USB_STATUS_TX_CMD, &ausb->status)){
		A_DEBUG("%s: WARNING: USB CPU Read Command Overrun (Read command in process)!\n", AMF_USB_NAME);
		ausb->stats.cmd_overrun++;
		err = -EBUSY;
		goto cpu_read_done;
	}

	if (!amf_skb_queue_len(&ausb->tx_cmd_free_list)){
		A_DEBUG("%s: WARNING: USB CPU Read Command Overrun (%d commands in process)!\n",
					AMF_USB_NAME, amf_skb_queue_len(&ausb->tx_cmd_list));
		ausb->stats.cmd_overrun++;
		err = -EBUSY;
		goto cpu_read_done;
	}
	skb = amf_skb_dequeue(&ausb->tx_cmd_free_list);

	// we send command code + 3 bytes
	cmd_data = amf_skb_put(skb, 4);
	cmd_data[0] = AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_CPU_INIT_PROSLIC);
	cmd_data[1] = 0;
	cmd_data[2] = 0;
	cmd_data[3] = 0;


	amf_skb_queue_tail(&ausb->tx_cmd_list, skb);
	ausb->tx_cmd_start = jiffies;

	do {
		wait_just_a_bit(2000, 1);
		if (++retry > 10) break;
	} while(!amf_skb_queue_len(&ausb->rx_cmd_list));

	if (!amf_skb_queue_len(&ausb->rx_cmd_list)){
		A_DEBUG("%s: WARNING: Timeout on Read USB-CPU Reg (%02X:%02X  %d:%ld:%ld:%ld)\n",
					AMF_USB_NAME,
					AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_READ_CPU),
					tx_data,
					retry,
					(unsigned long)(jiffies - ausb->tx_cmd_start),
					(unsigned long)ausb->tx_cmd_start,
					(unsigned long)jiffies);
		ausb->stats.cmd_timeout++;
		err = -EINVAL;
		goto cpu_read_done;
	}
	skb = amf_skb_dequeue(&ausb->rx_cmd_list);
	cmd_data = skb->data;
	if (cmd_data[0] != AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_CPU_INIT_PROSLIC)){
		A_DEBUG("%s: USB Read response is out of order 0x(waited: %02X, got %02X:%02X:%02X)\n",	AMF_USB_NAME,
														AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_CPU_INIT_PROSLIC),
														cmd_data[0], cmd_data[1], cmd_data[2]);

		ausb->stats.cmd_invalid++;
		amf_skb_init(skb, 0);
		amf_skb_queue_tail(&ausb->rx_cmd_free_list, skb);
		err = -EINVAL;
		goto cpu_read_done;
	}
	*rx_data = (unsigned char)cmd_data[1];

	amf_skb_init(skb, 0);
	amf_skb_queue_tail(&ausb->rx_cmd_free_list, skb);


cpu_read_done:
	amf_clear_bit(AMF_USB_STATUS_TX_CMD, &ausb->status);
	spin_unlock(&ausb->cmd_lock);
	return err;
}


//========================================================================================= CPU READ / WRITE (end)


#endif  //__AMF_USB_CPU__
