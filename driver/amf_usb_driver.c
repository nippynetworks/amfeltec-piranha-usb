/*****************************************************************************
* amf_usb_driver.c  Amfeltec USB Device Driver
*
* Copyright (C) 2013-2014, Amfeltec Corp.
******************************************************************************
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

#include "amf_usb.h"
#include "amf_usb_ioctl.h"

#include <linux/miscdevice.h>  /* miscellaneous device with Major Number = 10 */

//===================================== PROTOTYPES ========================================

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
static long amf_usb_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static int amf_usb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif

//===================================== GLOBALS ===========================================

static int alawoverride = 0;
static char *opermode = "FCC";
static int battthresh = 0;
static int battdebounce = 0;
static int battalarm = 0;
static int fxo_txgain = 0;
static int fxo_rxgain = 0;

static int reversepolarity = 0;
static int lowpower = 0;
static int fastringer = 0;
static int ringampl = 0;
static int fxs_txgain = 0;
static int fxs_rxgain = 0;

static int firmwareupdate = 0;
static int dahdicfg_on = 0;
static int watchdog_off = 0;
static int no_master = 0;
static int chunk_size = AMF_USB_DEFAULT_CHUNK_SIZE;

static struct amf_usb_t		*devices[MAX_USB_DEVICES];
static struct amf_usb		*ausb_current;
static int 					devices_cnt = 0;

char *argv[] = {"/usr/sbin/dahdi_cfg", "help!", NULL};
static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/bin", NULL};


struct usb_tx_urb* amf_usb_init_tx_urb(struct amf_usb *ausb, struct usb_tx_urb* tx_urb_item);
struct usb_tx_urb* amf_usb_init_tx_urb_len(struct amf_usb *ausb, struct usb_tx_urb* tx_urb_item, int len);

int amf_usb_txdata_send(amf_usb_t *ausb);
void print_arr();

//===================================== GLOBALS. ==========================================


//===================================== MISC

struct file_operations fops = {
	.owner	= THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
	.unlocked_ioctl = (void*)&amf_usb_ioctl,
	.compat_ioctl = (void*)&amf_usb_ioctl,
#else
	.ioctl	= amf_usb_ioctl,
#endif
};

static struct miscdevice amf_dev = {
	minor:	MISC_DYNAMIC_MINOR,
	name:	"amf_usb",
	fops:	&fops,
};

void print_dahdi_call(struct amf_usb* ausb, char* method)
{
	char* type;
	static int prev = 0;
	int delta = jiffies - prev;
	prev = jiffies;
	if(AMF_USB_FXO == ausb->amf_device_type)
			type = "FXO";
		
	if(AMF_USB_FXS == ausb->amf_device_type)
			type = "FXS";
	printk(KERN_INFO "++++ %s %s    %d\n", type, method, delta);
}

void wait_just_a_bit(int ms, int fast)
{
//	udelay(ms);
	unsigned long	start_ticks = jiffies;
	int		delay_ticks = (HZ*ms)/1000;

	while((jiffies-start_ticks) < delay_ticks){
		amf_delay(100);
		if (!fast) schedule();
	}
}


//===============================================================================================    IOCTL   ===

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
static long amf_usb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int amf_usb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	amf_arg_t *q;
	int err = 0;

	int len;
	unsigned char Buffer[8];
	int cnt = 0;
	int ret = 0;

	q = KMALLOC(sizeof(amf_arg_t), GFP_KERNEL, 16);
	memset(q, 0, sizeof(amf_arg_t));

	switch(cmd){
	case AMF_GET_STATUS:

		q->dev_num = devices_cnt;
		q->amf_device_type = ausb_current->amf_device_type;

		Buffer[0] = 0x0A;

		len = amf_usb_txdata_raw(ausb_current, &Buffer[0], 1);
		wait_just_a_bit(1, 0);

		cnt = 0;
		while (ret) {
			wait_just_a_bit(1, 0);
			ret = amf_usb_txdata_raw_ready(ausb_current);
			cnt++;
			if (cnt > 100){
				A_DEBUG("%s: Failed to read conf register (tx), cnt=%d\n", AMF_USB_NAME, cnt);
				goto ioctl_err;
			}
		}

		wait_just_a_bit(10, 0);

		cnt = 0;
		while (q->len == 0) {
			wait_just_a_bit(1, 0);
			q->len = amf_usb_rxdata_raw(ausb_current, &q->data[0], 6);
			cnt++;
			if (cnt > 100){
				A_DEBUG("%s: Failed to read conf register (rx), cnt=%d\n", AMF_USB_NAME, cnt);
				goto ioctl_err;
			}
		}
		A_DEBUG_RX("%s: Read conf register (rx OK), cnt=%d, len=%d\n", AMF_USB_NAME, cnt, q->len);

		if (copy_to_user((amf_arg_t *)arg, q, sizeof(amf_arg_t))) {
			A_DEBUG("%s: Failed copy to user...\n", AMF_USB_NAME);
			err = -EFAULT;
			goto ioctl_err;
		}
		break;


	case AMF_GET_CPU_ID:

		Buffer[0] = 0x09;
		len = amf_usb_txdata_raw(ausb_current, &Buffer[0], 1);
		wait_just_a_bit(1, 0);

		cnt = 0;
		while (ret) {
			wait_just_a_bit(1, 0);
			ret = amf_usb_txdata_raw_ready(ausb_current);
			cnt++;
			if (cnt > 100){
				A_DEBUG("%s: Failed to read CPU ID (tx), cnt=%d\n", AMF_USB_NAME, cnt);
				goto ioctl_err;
			}
		}

		wait_just_a_bit(10, 0);

		cnt = 0;
		while (q->len == 0) {
			wait_just_a_bit(1, 0);
			q->len = amf_usb_rxdata_raw(ausb_current, &q->data[0], 8);
			cnt++;
			if (cnt > 100){
				A_DEBUG("%s: Failed to read CPU ID (rx), cnt=%d\n", AMF_USB_NAME, cnt);
				goto ioctl_err;
			}
		}
		A_DEBUG_RX("%s: Read CPU ID (rx OK), cnt=%d, len=%d\n", AMF_USB_NAME, cnt, q->len);

		if (copy_to_user((amf_arg_t *)arg, q, sizeof(amf_arg_t))) {
			A_DEBUG("%s: Failed copy to user...\n", AMF_USB_NAME);
			err = -EFAULT;
			goto ioctl_err;
		}
		break;

	case AMF_CPU_READ_REG:
		break;
	case AMF_CPU_WRITE_REG:
		break;
	case AMF_FW_UPDATE_ENABLE:
		amf_usb_rxurb_reset(ausb_current);
		amf_usb_txurb_reset(ausb_current);
		break;

	case AMF_FW_DATA_WRITE:

		if (copy_from_user(q,(amf_arg_t *)arg,sizeof(amf_arg_t))) {
			A_DEBUG("%s: Failed copy from user...\n", AMF_USB_NAME);
			err = -EFAULT;
			goto ioctl_err;
		}

		if (q->len) {
			q->len = amf_usb_txdata_raw(ausb_current, (unsigned char *)q->data, q->len);
			q->ret = 0;
		} else {
			// TX_READY?
			q->ret = 0;
		}

		if (copy_to_user((amf_arg_t *)arg, q, sizeof(amf_arg_t))) {
			A_DEBUG("%s: Failed copy to user...\n", AMF_USB_NAME);
			err = -EFAULT;
			goto ioctl_err;
		}

	break;

	case AMF_FW_DATA_READ:

		if (copy_from_user(q,(amf_arg_t *)arg,sizeof(amf_arg_t))) {
			A_DEBUG("%s: Failed copy from user...\n", AMF_USB_NAME);
			err = -EFAULT;
			goto ioctl_err;
		}

		q->len = amf_usb_rxdata_raw(ausb_current, &q->data[0], q->len);
		q->ret = 0;

		if (copy_to_user((amf_arg_t *)arg, q, sizeof(amf_arg_t))) {
			A_DEBUG("%s: Failed copy to user...\n", AMF_USB_NAME);
			err = -EFAULT;
			goto ioctl_err;
		}

		break;

	default:
		A_DEBUG("%s: unknown ioctl command: %d\n", AMF_USB_NAME, cmd);
		err = -EFAULT;
		goto ioctl_err;
	}

ioctl_err:
	if (q != NULL) {
		KFREE(q, 30);
	}
	return err;
}

//===============================================================================================    IOCTL END



//===============================================================================================    PROBE   ===
//===============================================================================================    PROBE   ===
//===============================================================================================    PROBE   ===

static int amf_usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device 				*udev = interface_to_usbdev(interface);
	struct usb_device_descriptor 	udesc = udev->descriptor;
	struct amf_usb_desc				*desc = (struct amf_usb_desc *)id->driver_info;
	struct amf_usb 					*ausb = NULL;
	int retval = -ENOMEM;

	int mod_no;
	int x;
	int retries;
	int err = 0;

	int i;
	u16 control_sum;
	unsigned char	writebuf[MAX_USB_TX_LEN];

	A_DEBUG("%s: Probing %s (%X) on %d ...\n", AMF_USB_NAME, desc->name, desc->adptr_type, udev->devnum);

	for (x = 0; x < MAX_USB_DEVICES; x++)
		if (!devices[x]) break;
	if (x >=  MAX_USB_DEVICES) {
		DEBUG_EVENT("%s: Too many interfaces: Found %d, can only handle %d!\n", AMF_USB_NAME, x, MAX_USB_DEVICES - 1);
		return -EIO;
	}

	/* cp210x buffers behave strangely unless device is reset */
	if (usb_reset_device(udev)) {
		DEBUG_EVENT("%s: Failed to reset USB device!\n", AMF_USB_NAME);
		goto probe_error_memory;
	}
	ausb = KMALLOC(sizeof(struct amf_usb), GFP_KERNEL, 17);
	if (ausb == NULL) {
		dev_err(&interface->dev, "Out of memory\n");
		goto probe_error_memory;
	}
	memset(ausb, 0x00, sizeof(*ausb));
//RX>>>
//print_arr();

	ausb->udev = usb_get_dev(udev);
	amf_set_bit(AMF_USB_LSTATUS_GETDEV, &ausb->lstatus);

	usb_set_intfdata(interface, ausb);
	amf_set_bit(AMF_USB_LSTATUS_INTF, &ausb->lstatus);

	devices[x] = ausb;
	ausb->num = x;
	devices_cnt++;
	ausb_current = ausb;
	ausb->chunk_amount = 0;
	amf_usb_init_read_write(ausb);
	memset(ausb->status_hist, 0, STATUS_BITS);


ausb->handled = ausb->incoming = 0;

	switch(udesc.idProduct){
	case AMF_USB_PRODUCTID_FXO:
		ausb->amf_device_type = AMF_USB_FXO;
		ausb->mod_cnt = 1;
		ausb->mod_type[0] = AMF_USB_FXO;
		//printk(KERN_INFO "FXO %s\n",  __func__);

		break;
	case AMF_USB_PRODUCTID_FXS:
		ausb->amf_device_type = AMF_USB_FXS;
		ausb->mod_cnt = 1;
		ausb->mod_type[0] = AMF_USB_FXS;
		//printk(KERN_INFO "FXS %s\n",  __func__);

		break;
	case AMF_USB_PRODUCTID_PA:
		ausb->amf_device_type = AMF_USB_PA;
		ausb->mod_cnt = 1;
		ausb->mod_type[0] = AMF_USB_FXS;
		//printk(KERN_INFO "PA %s\n",  __func__);

		break;
	default:
		ausb->amf_device_type = AMF_USB_FXO;
		ausb->mod_cnt = 1;
		ausb->mod_type[0] = AMF_USB_FXO;
		//printk(KERN_INFO "FXO ? s\n",  __func__);

	}

	ausb->alawoverride = alawoverride;

	if(amf_usb_set_opermode(ausb, opermode)) {
		DEBUG_EVENT("%s: Unknown operating mode specified: %s. Using default (FCC)!\n", AMF_USB_NAME, opermode);
		opermode = "FCC";
		amf_usb_set_opermode(ausb, opermode);
	}

	if (battthresh == 0) {
		ausb->battthresh = 3;
	} else {
		ausb->battthresh = battthresh;
	}
	if (battdebounce == 0) {
		ausb->battdebounce = 64;
	} else {
		ausb->battdebounce = battdebounce;
	}
	if (battalarm == 0) {
		ausb->battalarm = 1000;
	} else {
		ausb->battalarm = battalarm;
	}

	ausb->fxo_txgain = fxo_txgain;
	ausb->fxo_rxgain = fxo_rxgain;

	ausb->reversepolarity = reversepolarity;
	ausb->lowpower = lowpower;
	ausb->fastringer = fastringer;
	ausb->ringampl = ringampl;
	ausb->fxs_txgain = fxs_txgain;
	ausb->fxs_rxgain = fxs_rxgain;
	if((chunk_size < 1) || (chunk_size > AMF_USB_CHUNKS_PER_PACKET)){
		chunk_size = AMF_USB_DEFAULT_CHUNK_SIZE;
	}
	ausb->chunk_size = chunk_size;

	tasklet_init(&ausb->bh_task, amf_usb_bh, (unsigned long)ausb);
	amf_set_bit(AMF_USB_LSTATUS_TASKLET, &ausb->lstatus);

	if (amf_usb_rxtx_buffers_alloc(ausb)) {
		DEBUG_EVENT("%s: Failed to allocate TX/RX buffers!\n", AMF_USB_NAME);
		goto probe_error;
	}

	spin_lock_init(&ausb->cmd_lock);
	spin_lock_init(&ausb->lock);

	ausb->ctrl_idle_pattern  = AMF_USB_CTRL_IDLE_PATTERN;
	if (firmwareupdate) {
		ausb->firmwareupdate = 1;
	}
//======================================================================================   STARTING  UART

	if (amf_usb_start_uart(ausb)) {

		DEBUG_EVENT("%s: Failed to start UART!\n", AMF_USB_NAME);
		goto probe_error;
	}

	ausb->main_state = INIT_STATE;

//======================================================================================   SETTING URBs

	amf_usb_set_transfer_urbs(interface);
	amf_set_bit(AMF_USB_LSTATUS_URB, &ausb->lstatus);
//	amf_usb_set_urb_status_ready(ausb);

	amf_set_bit(AMF_USB_STATUS_READY, &ausb->status);



	/* first init list heads */
	INIT_LIST_HEAD(&ausb->dahdi_rx_queue.head);
	INIT_LIST_HEAD(&ausb->dahdi_tx_queue.head);
	INIT_LIST_HEAD(&ausb->usb_tx_queue.head);
	spin_lock_init(&ausb->dahdi_rx_queue.list_lock);
	spin_lock_init(&ausb->dahdi_tx_queue.list_lock);
	spin_lock_init(&ausb->usb_tx_queue.list_lock);



//======================================================================================   FIRMWARE Update
	if (firmwareupdate) {
		ausb->firmwareupdate = 1;
		x = 0;
		for (i = 0; i < AMF_URB_AMOUNT; i++){
			ausb->urbread[x][i].indx = i;
			ausb->urbread[x][i].pvt = ausb;
			err = usb_submit_urb(&ausb->urbread[x][i].urb, GFP_KERNEL);
			if ((err != 0) && (err != -ENOENT)) {
				A_DEBUG("%s: Failed to start RX transfer: %d %d\n", AMF_USB_NAME, x, i);
				goto probe_error;
			}
		}
		goto probe_misc;
	}


//======================================================================================   START  TRANSFER

	// First, start as Slave:
	// Reset MCPU, SLIC/DAA framer, Command queue
	// Switch device to Master and reset Watchdog
	{
		struct reg_access_item* ra_item;
		ra_item = amf_usb_create_command(	AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_WRITE_CPU), 
								AMF_USB_CPU_REG_CTRL, 
								AMF_USB_CPU_BIT_CTRL_RESET_ALL, 
								0);
		add_cmd_queue(ra_item, &ausb->reg_access_tx_head);
		amf_usb_create_packet_64(ausb);
	}

	wait_just_a_bit(100, 0);

	// Second, start as Master:
	/* Start Transfer Process anf Verify sync with USB device */

	x = 0;
	for (i = 0; i < AMF_URB_AMOUNT; i++){
		ausb->urbread[x][i].indx = i;
		ausb->urbread[x][i].pvt = ausb;
		err = usb_submit_urb(&ausb->urbread[x][i].urb, GFP_KERNEL);
		if ((err != 0) && (err != -ENOENT)) {
			A_DEBUG("%s: Failed to start RX transfer: %d %d\n", AMF_USB_NAME, x, i);
			goto probe_error;
		}
	}

//======================================================================================   SETUP CPU

	retries = 0;
	while (retries < 3) {
		if (amf_usb_set_cpu(ausb)) {
			retries++;
		} else {
			break;
		}
	}
	if (retries == 3) {
		DEBUG_EVENT("%s: WARNING: device in Firmware Update mode!\n", AMF_USB_NAME);
		goto probe_error;
	}

	sprintf(ausb->device_name,"%s%d%s%d", "amf_usb", ausb->order, "-", ausb->num);


//=================================================================== DAA/Proslic initialization:
	for (mod_no = 0; mod_no < ausb->mod_cnt; mod_no++){

		if (ausb->mod_type[mod_no] == AMF_USB_FXO) {
			retval = amf_usb_init_daa(ausb, mod_no, 0, 0);
			if (!retval) {
				amf_set_bit(AMF_USB_LSTATUS_DAA, &ausb->lstatus);
				A_DEBUG("%s%d: USB Device in Auto FXO mode...\n", AMF_USB_NAME, ausb->order);
			}
		} else if (ausb->mod_type[mod_no] == AMF_USB_FXS) {
			retval = amf_usb_init_proslic(ausb, mod_no, 0, 0);
			if (!retval) {
				amf_set_bit(AMF_USB_LSTATUS_PROSLIC, &ausb->lstatus);
				ausb->fx[mod_no].fxs.oldrxhook = 0;					/* default (on-hook) */
				A_DEBUG("%s%d: USB Device in Auto FXS mode...\n", AMF_USB_NAME, ausb->order);
			}
		}
	}
	ausb->stats.mcpu_status_err_cnt = 0;
	ausb->stats.mcpu_queue_threshold_err_cnt = 0;

	// CPU is OK: activate Fail-over Relay Control and enable Watchdog, mod_no = 0
	if (ausb->mod_type[0] == AMF_USB_FXO) {
		__amf_usb_cpu_write(ausb, AMF_USB_CPU_REG_FAILOVER_RELAY, AMF_USB_CPU_FAILOVER_RELAY_ON);
		if (ausb->failover_enabled) {
			__amf_usb_cpu_write(ausb, AMF_USB_CPU_REG_WATCHDOG, AMF_USB_CPU_WATCHDOG_RESTART);
			if (watchdog_off) {
			} else {
				__amf_usb_cpu_write(ausb, AMF_USB_CPU_REG_FAILOVER_RELAY, (AMF_USB_CPU_FAILOVER_RELAY_ON | AMF_USB_CPU_FAILOVER_WATCHDOG_ON));
			}
		}
	}


//======================================================================================  DAHDI
	// Driver just started? Leave DAHDI registration for init, else here for hotplug:

//	if (amf_get_driver_started()) {
		retval = amf_usb_dahdi_register(ausb);
		if (!retval) {
			DEBUG_EVENT("%s%d: DAHDI span registered\n", AMF_USB_NAME, ausb->order);
			amf_set_bit(AMF_USB_LSTATUS_DAHDI, &ausb->lstatus);
			ausb->main_state = OPERATION_STATE;
		}
			
//		} else {
//			DEBUG_EVENT("%s%d: Unable to register DAHDI span\n", AMF_USB_NAME, ausb->order);
//			goto probe_error;
//		}


#ifdef AMF_USB_MODE_SLAVE
		if((!no_master) && (ausb->master_span_on == 0)) {
		if (!(amf_test_bit(AMF_USB_LSTATUS_DAHDI_MASTER, &ausb->lstatus))){
			retval = amf_usb_dahdi_register_master(ausb);
			if (!retval) {
				DEBUG_EVENT("%s%d: DAHDI MASTER span registered\n", AMF_USB_NAME, ausb->order);
				amf_set_bit(AMF_USB_LSTATUS_DAHDI_MASTER, &ausb->lstatus);
				ausb->master_span_on = 1;
			} else {
				DEBUG_EVENT("%s%d: Unable to register DAHDI MASTER span\n", AMF_USB_NAME, ausb->order);
				goto probe_error;
			}
		}
		}
#endif

		// on reconnect we can call DAHDI module
		if (dahdicfg_on) {
			call_usermodehelper(argv[0], argv, envp, 0);
//print_dahdi_call(ausb, "call_usermodehelper");
		}

		//===============================  SOFT_IRQ
		ausb->isr_func	= amf_usb_isr;
		ausb->isr_arg	= ausb;

		//===============================  Channels init
		for (mod_no = 0; mod_no < ausb->mod_cnt; mod_no++){
			ausb->chans[mod_no].readchunk = &ausb->readchunk[mod_no][0];
			ausb->chans[mod_no].writechunk = &ausb->writechunk[mod_no][0];

			amf_usb_rxevent_enable(ausb, mod_no, 1);
			amf_usb_rxevent(ausb, mod_no, 1);
		}

		amf_usb_rxdata_enable(ausb, 1);

		if (ausb->mod_type[0] == AMF_USB_FXS) {
			ausb->time_to_reset_fifo_pointers = 1;
		}

//	} //if (amf_get_driver_started())

	//	CPU loopback on startup:
	//	DEBUG_EVENT("%s%d: Activating CPU loopback on startup...\n", AMF_USB_NAME, ausb->order);
	//		__amf_usb_cpu_write(ausb, 8, 0x02);

	//	SLIC Digital loopback on startup:
	//	DEBUG_EVENT("%s%d: Activating SLIC loopback on startup...\n", AMF_USB_NAME, ausb->order);
	//	__amf_usb_fxo_write(ausb, 0, 8, 0x02);




return 0;

//=============================================================================  Misc driver  =================
probe_misc:


	if (misc_register(&amf_dev)) {
		DEBUG_EVENT("%s: Unable to register misc device!\n", AMF_USB_NAME);
	} else {
		A_DEBUG("%s: Misc device registered!\n", AMF_USB_NAME);
		amf_set_bit(AMF_USB_LSTATUS_MISC, &ausb->lstatus);
	}

return 0;

probe_error:
//return -ENODEV; //> crash on disconnect
return 0;

probe_error_memory:
return -ENOMEM;

}

//==========================================================================    PROBE END
//=============================================================================================================


static void amf_usb_disconnect(struct usb_interface *interface)
{
	//int minor = interface->minor;
	struct amf_usb *ausb;
	int x;
	int mod_no;
	int order;

	ausb = usb_get_intfdata(interface);
	order = ausb->order;


	// trying to reset SLIC:
	__amf_usb_cpu_write(ausb, AMF_USB_CPU_REG_CTRL, AMF_USB_CPU_BIT_CTRL_RESET_ALL);
	wait_just_a_bit(100, 0);

	ausb->isr_func	= NULL;
	ausb->isr_arg	= NULL;

	amf_clear_bit(AMF_USB_STATUS_READY, &ausb->status);

	if (amf_test_bit(AMF_USB_LSTATUS_MISC, &ausb->lstatus)){
		misc_deregister(&amf_dev);
		A_DEBUG("%s: Misc device deregistered!\n", AMF_USB_NAME);
	}

	amf_usb_rxdata_enable(ausb, 0);

	for (mod_no = 0; mod_no < (ausb->mod_cnt); mod_no ++){
		amf_usb_rxevent_enable(ausb, mod_no, 0);
	}

	if (amf_test_bit(AMF_USB_LSTATUS_DAHDI_MASTER, &ausb->lstatus)){
		amf_usb_dahdi_unregister_master(ausb);
		DEBUG_EVENT("%s%d: DAHDI MASTER span unregistered\n", AMF_USB_NAME, ausb->order);
	}

	if (amf_test_bit(AMF_USB_LSTATUS_DAHDI, &ausb->lstatus)){
		amf_usb_dahdi_unregister(ausb);
		DEBUG_EVENT("%s%d: DAHDI span unregistered\n", AMF_USB_NAME, ausb->order);
	}

	if (amf_test_bit(AMF_USB_LSTATUS_RXCMD, &ausb->lstatus)){
		AMF_QUE_CLEAN(&ausb->rx_cmd_list);
		AMF_QUE_CLEAN(&ausb->rx_cmd_free_list);
	}

	if (amf_test_bit(AMF_USB_LSTATUS_TXCMD, &ausb->lstatus)){
		AMF_QUE_CLEAN(&ausb->tx_cmd_free_list);
		AMF_QUE_CLEAN(&ausb->tx_cmd_list);
	}

	if (amf_test_bit(AMF_USB_LSTATUS_INTF, &ausb->lstatus)){
		usb_set_intfdata(interface, NULL);
	}

	if (amf_test_bit(AMF_USB_LSTATUS_READBUF, &ausb->lstatus)){
		KFREE(ausb->readbuf, 31);
	}

	if (amf_test_bit(AMF_USB_LSTATUS_WRITEBUF, &ausb->lstatus)){
		KFREE(ausb->writebuf, 32);
	}

	if (amf_test_bit(AMF_USB_LSTATUS_TASKLET, &ausb->lstatus)){
		tasklet_kill(&ausb->bh_task);
	}

	if (amf_test_bit(AMF_USB_LSTATUS_GETDEV, &ausb->lstatus)){
		usb_put_dev(ausb->udev);
	}

	KFREE(ausb, 33);
	devices_cnt--;

	dev_info(&interface->dev, "Amfeltec USB device #%d is disconnected\n", order);

}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
static int amf_usb_suspend (struct usb_interface *intf, pm_message_t message)
{
	//struct usb_device	*dev = interface_to_usbdev(intf);
	return 0;
}
#else
static int amf_usb_suspend (struct usb_interface *intf, u32 msg)
{
	//struct usb_device	*dev = interface_to_usbdev(intf);
	return 0;
}
#endif

static int amf_usb_resume (struct usb_interface *intf)
{
	//struct usb_device	*dev = interface_to_usbdev(intf);
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23)
static int amf_usb_prereset (struct usb_interface *intf)
# else
static void amf_usb_prereset (struct usb_interface *intf)
# endif
{
	//struct usb_device	*dev = interface_to_usbdev(intf);
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23)
	return 0;
# else
	return;
# endif
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23)
static int amf_usb_postreset (struct usb_interface *intf)
# else
static void amf_usb_postreset (struct usb_interface *intf)
# endif
{
	//struct usb_device	*dev = interface_to_usbdev(intf);
# if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23))
	return 0;
# else
	return;
# endif
}
#endif


static struct amf_usb_desc amfusb = { "Amfeltec USB", F100_ADAPTER };

static struct usb_device_id amf_usb_dev_ids[] = {
	{
		match_flags:		(USB_DEVICE_ID_MATCH_VENDOR|USB_DEVICE_ID_MATCH_DEVICE),
		idVendor:		AMF_USB_VENDORID,
		idProduct:		AMF_USB_PRODUCTID_FXO,
		driver_info:	(unsigned long)&amfusb,
	},
	{
		match_flags:		(USB_DEVICE_ID_MATCH_VENDOR|USB_DEVICE_ID_MATCH_DEVICE),
		idVendor:		AMF_USB_VENDORID,
		idProduct:		AMF_USB_PRODUCTID_FXS,
		driver_info:	(unsigned long)&amfusb,
	},
	{
		match_flags:		(USB_DEVICE_ID_MATCH_VENDOR|USB_DEVICE_ID_MATCH_DEVICE),
		idVendor:		AMF_USB_VENDORID,
		idProduct:		AMF_USB_PRODUCTID_PA,
		driver_info:	(unsigned long)&amfusb,
	},
	{ }     /* Terminating Entry */
};

static struct usb_driver amf_usb_driver =
{
	.name =		    AMF_USB_NAME,
	.probe = 	    amf_usb_probe,
	.disconnect =	amf_usb_disconnect,
	.suspend =	    amf_usb_suspend,
	.resume =		amf_usb_resume,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
	.pre_reset =	amf_usb_prereset,
	.post_reset =	amf_usb_postreset,
#endif
	.id_table = 	amf_usb_dev_ids,
};


int amf_usb_init(void)
{
	int retval = 0;
	int i;
	int j;
	struct amf_usb *ausb;
	int mod_no;

	retval = usb_register(&amf_usb_driver);
	if(retval) {
		DEBUG_EVENT("USB registration failed. Error number: %d\n", retval);
		return retval;
	}

	// rotary switch: 0 = firmware update mode, others = registration order
	for (i = 0; i < 10; i++) {
		for (j = 0; j < devices_cnt; j++) {

			ausb = (amf_usb_t *)devices[j];
			if (ausb->order == i && i == 0) {
				if (!ausb->firmwareupdate) {
					DEBUG_EVENT("%s: Please check Rotary Switch setting and reconnect device!\n", AMF_USB_NAME);
				}
			} else if (ausb->order == i) {

#ifdef AMF_USB_MODE_SLAVE
				if((!no_master) && (ausb->master_span_on == 0)) {
				if (!(amf_test_bit(AMF_USB_LSTATUS_DAHDI_MASTER, &ausb->lstatus))){
					retval = amf_usb_dahdi_register_master(ausb);
					if (!retval) {
						DEBUG_EVENT("%s%d: DAHDI MASTER span registered\n", AMF_USB_NAME, ausb->order);
						amf_set_bit(AMF_USB_LSTATUS_DAHDI_MASTER, &ausb->lstatus);
						ausb->master_span_on = 1;
					} else {
						DEBUG_EVENT("%s%d: Unable to register DAHDI MASTER span\n", AMF_USB_NAME, ausb->order);
					}
				}
				}
#endif


				if (!(amf_test_bit(AMF_USB_LSTATUS_DAHDI, &ausb->lstatus))){
					retval = amf_usb_dahdi_register(ausb);
					if (!retval) {
						DEBUG_EVENT("%s%d: DAHDI span registered\n", AMF_USB_NAME, ausb->order);
						A_DEBUG_USB("%s%d: DAHDI span cannot provide timing: %d\n", AMF_USB_NAME, ausb->order, ausb->span.cannot_provide_timing);
						amf_set_bit(AMF_USB_LSTATUS_DAHDI, &ausb->lstatus);
					} else {
						DEBUG_EVENT("%s%d: Unable to register DAHDI span\n", AMF_USB_NAME, ausb->order);
					}
				}


				//===============================  SOFT_IRQ
				ausb->isr_func	= amf_usb_isr;
				ausb->isr_arg	= ausb;

				//===============================  Channels init
				for (mod_no = 0; mod_no < ausb->mod_cnt; mod_no++){
					ausb->chans[mod_no].readchunk = &ausb->readchunk[mod_no][0];
					ausb->chans[mod_no].writechunk = &ausb->writechunk[mod_no][0];

					amf_usb_rxevent_enable(ausb, mod_no, 1);
					amf_usb_rxevent(ausb, mod_no, 1);


				}
				amf_usb_rxdata_enable(ausb, 1);

				if (ausb->mod_type[0] == AMF_USB_FXS) {
					ausb->time_to_reset_fifo_pointers = 1;
				}

			}
		}
	}


	if (!dahdicfg_on) {
		call_usermodehelper(argv[0], argv, envp, 0);
	}

	amf_set_driver_started();

	return retval;
}

void amf_usb_exit(void)
{
	usb_deregister(&amf_usb_driver);
	return ;
}


module_param(alawoverride, int, 0600);
module_param(opermode, charp, 0600);
module_param(battthresh, int, 0600);
module_param(battdebounce, int, 0600);
module_param(battalarm, int, 0600);
module_param(fxo_txgain, int, 0600);
module_param(fxo_rxgain, int, 0600);

module_param(reversepolarity, int, 0600);
module_param(lowpower, int, 0600);
module_param(fastringer, int, 0600);
module_param(ringampl, int, 0600);
module_param(fxs_txgain, int, 0600);
module_param(fxs_rxgain, int, 0600);

module_param(firmwareupdate, int, 0600);
module_param(dahdicfg_on, int, 0600);
module_param(watchdog_off, int, 0600);
module_param(no_master, int, 0600);
module_param(chunk_size, int, 0600);





