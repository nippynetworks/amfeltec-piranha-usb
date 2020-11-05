 /*
  *  amf_usb_cp210x.c - 'usb bridge related functions'
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

#ifndef __AMF_USB_CP210X__
#define __AMF_USB_CP210X__

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

#include "amf_usb.h"
#include "amf_usb_utils.h"

//======================================================================================   CP210X UART

#define CP2102_PARTNUM			0x02
#define CP2104_PARTNUM			0x04

#define REQTYPE_HOST_TO_DEVICE	0x41
#define REQTYPE_DEVICE_TO_HOST	0xC1

#define CP210X_IFC_ENABLE		0x00	/* Enable / Disable */
#define UART_ENABLE				0x0001
#define UART_DISABLE			0x0000

#define CP210X_SET_BAUDRATE 	0x1E
#define CP210X_GET_BAUDRATE 	0x1D
#define CP210X_SET_LINE_CTL 	0x03
#define CP210X_GET_LINE_CTL 	0x04

#define CP210X_VENDOR_SPECIFIC 	0xFF
#define CP210X_WRITE_LATCH		0x37E1
#define CP210X_READ_LATCH		0x00C2
#define CP210X_GET_PARTNUM		0x370B

#define BITS_DATA_MASK			0x0f00
#define BITS_DATA_5				0x0500
#define BITS_DATA_6				0x0600
#define BITS_DATA_7				0x0700
#define BITS_DATA_8				0x0800
#define BITS_DATA_9				0x0900

#define BITS_PARITY_MASK		0x00f0
#define BITS_PARITY_NONE		0x0000
#define BITS_PARITY_ODD			0x0010
#define BITS_PARITY_EVEN		0x0020
#define BITS_PARITY_MARK		0x0030
#define BITS_PARITY_SPACE		0x0040

#define BITS_STOP_MASK			0x000f
#define BITS_STOP_1				0x0000
#define BITS_STOP_1_5			0x0001
#define BITS_STOP_2				0x0002

#define BAUD_RATE_GEN_FREQ		0x384000
#define AMF_USB_BAUD_RATE		500000


/*
 * cp210x_set_config
 * Writes to CP210X configuration registers
 * 'size' is specified in bytes.
 * 'data' is a pointer to a pre-allocated array of integers large
 * enough to hold 'size' bytes (with 4 bytes to each integer)
 */
int amf_usb_set_config(amf_usb_t *usb, u8 request, int value, unsigned int *data, int size)
{
	__le32		*buf;
	int		result, i, length;

	A_BUG(usb->udev == NULL);

	/* Number of integers required to contain the array */
	length = (((size - 1) | 3) + 1)/4;

	buf = KMALLOC(length * sizeof(__le32), GFP_KERNEL, 10);
	if (!buf) {
		A_DEBUG("ERROR [%s:%d]: Out of memory!\n", __FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	/* Array of integers into bytes */
	for (i = 0; i < length; i++)
		buf[i] = cpu_to_le32(data[i]);

	if ((request == CP210X_VENDOR_SPECIFIC) && (value == CP210X_WRITE_LATCH)) {
		result = usb_control_msg (
				usb->udev,
				usb_sndctrlpipe(usb->udev, 0),
				request,
				REQTYPE_HOST_TO_DEVICE,
				value,
				data,
				NULL, 0, 300);
	} else {
		result = usb_control_msg (
				usb->udev,
				usb_sndctrlpipe(usb->udev, 0),
				request,
				REQTYPE_HOST_TO_DEVICE,
				value,
				0,
				buf, size, 300);
	}

	KFREE(buf, 24);

	if (result != size){
		A_DEBUG("ERROR [%s:%d]: Unable to send request: request=0x%x size=%d result=%d!\n",
				__FUNCTION__,__LINE__, request, size, result);
		return -EPROTO;
	}

	return 0;
}


/*
 * cp210x_get_config
 * Reads from the CP210X configuration registers
 * 'size' is specified in bytes.
 * 'data' is a pointer to a pre-allocated array of integers large
 * enough to hold 'size' bytes (with 4 bytes to each integer)
 */
int amf_usb_get_config(amf_usb_t *usb, u8 request, int value, unsigned int *data, int size)
{
	__le32 *buf;
	int result, i, length;

	A_BUG(usb->udev == NULL);

	/* Number of integers required to contain the array */
	length = (((size - 1) | 3) + 1)/4;

	buf = kcalloc(length, sizeof(__le32), GFP_KERNEL);
	if (!buf) {
		A_DEBUG("ERROR [%s:%d]: Out of memory.\n", __FUNCTION__,__LINE__);
		return -ENOMEM;
	}

	/* Issue the request, attempting to read 'size' bytes */
	result = usb_control_msg (
					usb->udev,
					usb_rcvctrlpipe (usb->udev, 0),
					request,
					REQTYPE_DEVICE_TO_HOST,
					value,
					0, 								// interface number
					buf,
					size,
					300);

	/* Convert data into an array of integers */
	for (i=0; i<length; i++)
		data[i] = le32_to_cpu(buf[i]);
	KFREE(buf, 25);
	if (result != size) {
		A_DEBUG("ERROR [%s:%d]: Unable to send config request, request=0x%x size=%d result=%d\n",
				__FUNCTION__, __LINE__, request, size, result);
		return -EPROTO;
	}

	return 0;
}

int amf_usb_start_uart(amf_usb_t *ausb) {

	unsigned int bits;
	unsigned int gpio;
	//unsigned int latch_mask_setting_to_write;
	int err = -1;

	if (amf_usb_set_config(ausb, CP210X_IFC_ENABLE, UART_ENABLE, NULL, 0)) {
		DEBUG_EVENT("%s: ERROR: Unable to enable UART!\n", AMF_USB_NAME);
		goto st_error;
	}

	if (ausb->firmwareupdate) {
		bits = 500000;				//0x7A120
	} else {
		bits = 1200000;
	}

printk(KERN_ERR "%s: -------------- set UART\n", __func__);
	if (amf_usb_set_config(ausb, CP210X_SET_BAUDRATE, 0, &bits, 4)) {
printk(KERN_ERR "%s: Baud rate requested not supported by device (%d bps)!\n", AMF_USB_NAME, AMF_USB_BAUD_RATE);
		DEBUG_EVENT("%s: Baud rate requested not supported by device (%d bps)!\n",AMF_USB_NAME, AMF_USB_BAUD_RATE);
		goto st_error;
	}

	// GPIO
	if (amf_usb_get_config(ausb, CP210X_VENDOR_SPECIFIC, CP210X_GET_PARTNUM, &bits, 1)) {
		DEBUG_EVENT("%s: Failed to get Part Number!\n",AMF_USB_NAME);
		goto st_error;
	}
	ausb->partnum = bits & 0xFF;

	if (ausb->partnum == CP2104_PARTNUM) {

		if (amf_usb_get_config(ausb, CP210X_VENDOR_SPECIFIC, CP210X_READ_LATCH, &gpio, 1)) {
			DEBUG_EVENT("%s: Failed to get GPIO!\n",AMF_USB_NAME);
			goto st_error;
		}
		gpio = gpio & 0xF;

		/*
		// We can write to GPIO here:
		gpio = ~gpio;
		gpio = gpio << 16;
		gpio |= 0xFFFF;
		latch_mask_setting_to_write = gpio & 0x000000FF;
		latch_mask_setting_to_write |= (gpio & 0x00FF0000) >> 8;
		amf_usb_set_config(ausb, CP210X_VENDOR_SPECIFIC, CP210X_WRITE_LATCH, latch_mask_setting_to_write, 0);
		amf_usb_get_config(ausb, CP210X_VENDOR_SPECIFIC, CP210X_READ_LATCH, &gpio, 1);
		A_DEBUG("%s: CP210X GPIO: (%04X)!\n",AMF_USB_NAME, gpio);
		*/
	}

	// LINE_CTL
	// 8: BITS_DATA = 8
	// 0: BITS_PARITY_NONE:		nothing to do
	// 0: BITS_STOP_1:			nothing to do
	if (amf_usb_get_config(ausb, CP210X_GET_LINE_CTL, 0, &bits, 2)) {
		DEBUG_EVENT("%s: Failed to get Line CTL!\n",AMF_USB_NAME);
		goto st_error;
	}
	bits &= ~BITS_DATA_MASK;
	bits |= BITS_DATA_8;
	if (amf_usb_set_config(ausb, CP210X_SET_LINE_CTL, bits, NULL, 0)){
		DEBUG_EVENT("%s: Number of data bits requested not supported by device (%02X)!\n",AMF_USB_NAME, bits);
		goto st_error;
	}

	/*
	bits &= ~BITS_PARITY_MASK;
	amf_usb_set_config(ausb, CP210X_SET_LINE_CTL, &bits, 2);
	bits &= ~BITS_STOP_MASK;
	amf_usb_set_config(ausb, CP210X_SET_LINE_CTL, &bits, 2);
	amf_usb_get_config(ausb, CP210X_GET_LINE_CTL, &bits, 2);
	A_DEBUG("%s: Line control_2: %4X!\n",AMF_USB_NAME, bits);
	*/

	return 0;
st_error:
	return err;
}

int amf_usb_stop_uart(amf_usb_t *ausb) {

	if (amf_usb_set_config(ausb, CP210X_IFC_ENABLE, UART_DISABLE, NULL, 0)) {
		A_DEBUG("%s: ERROR: Unable to disable UART!\n", AMF_USB_NAME);
	}
	return 0;
}


#endif  //__AMF_USB_CP210X__
