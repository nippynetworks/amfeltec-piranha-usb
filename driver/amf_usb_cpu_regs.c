 /*
  *  amf_usb_cpu_regs.c - 'command exchange API'
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

#include <linux/wait.h>
#include <linux/list.h>
#include <linux/usb.h>

#include "amf_usb.h"


//struct reg_access_item *get_cmd_from_queue(struct reg_access_head* head);
//void add_cmd_queue(struct reg_access_item* ra_item, struct reg_access_head *head);
//void add_cmd_fill_data(struct reg_access_item* ra_item, struct reg_access_head *head);

void amf_usb_init_read_write(amf_usb_t *ausb)
{
	init_waitqueue_head(&ausb->wait_read);
	INIT_LIST_HEAD(&ausb->reg_access_tx_head.head);
	spin_lock_init(&ausb->reg_access_tx_head.list_lock);
	INIT_LIST_HEAD(&ausb->reg_access_rx_head.head);
	spin_lock_init(&ausb->reg_access_rx_head.list_lock);
}


struct reg_access_item*  amf_usb_create_command(u8 command, u8 addr, u8 low, u8 high)
{
	struct reg_access_item* ra_item = KMALLOC(sizeof(struct reg_access_item), GFP_ATOMIC, 11);
	if(NULL == ra_item){
		return NULL;
	}
	INIT_LIST_HEAD(&ra_item->list);
	ra_item->cmd.ctrl = command;
	ra_item->cmd.addr = addr;
	ra_item->cmd.low = low;
	ra_item->cmd.high = high;
	return ra_item;
}


int amf_usb_read_value(u8 cmd, amf_usb_t *ausb, u8 address, int timeout)
{
	struct reg_access_item* ra_item;
	u32	retval;

	ra_item = amf_usb_create_command(cmd, address, 0, 0);
	if(NULL == ra_item){
		return -ENOMEM;
	}

	add_cmd_queue(ra_item, &ausb->reg_access_tx_head);
	wait_event_timeout(ausb->wait_read, !is_list_empty(&ausb->reg_access_rx_head), timeout);
	ra_item = get_cmd_from_queue(&(ausb->reg_access_rx_head.head));

	if(NULL ==  ra_item){
		printk(KERN_ERR"%s Timeout error\n", __func__);
		return -ETIME;
	}


	if((address != ra_item->cmd.addr) || (cmd != ra_item->cmd.ctrl)){
		KFREE(ra_item, 26);
		printk(KERN_ERR"%s Syncronization error address %02X ra_item->cmd.addr %02X cmd %02X ra_item->cmd.ctrl %02X\n",
			__func__, address, ra_item->cmd.addr, cmd, ra_item->cmd.ctrl);
		return -EBADR;
	}
	retval = ra_item->cmd.low | (ra_item->cmd.high << 8);
	retval &= 0xFFFF;
	
	KFREE(ra_item, 27);
	return retval;
}

int amf_usb_write_value(u8 cmd, amf_usb_t *ausb, u8 address, u8 low, u8 high, int timeout)
{
	struct reg_access_item* ra_item;

	ra_item = amf_usb_create_command(cmd, address, low, high);
	if(NULL == ra_item){
		return -ENOMEM;
	}

	add_cmd_queue(ra_item, &ausb->reg_access_tx_head);
	return 0;
}

int amf_usb_cmd_xtactor(amf_usb_t *ausb, struct cpu_reg_cmd* cmd)
{
	switch(cmd->ctrl){
		case RD_CPU_COMMAND:
		{
			struct reg_access_item* ra_item = KMALLOC(sizeof(struct reg_access_item), GFP_ATOMIC, 12);
			memcpy((u8*)&ra_item->cmd, (u8*)cmd, sizeof(*cmd));
			add_cmd_queue(ra_item, &ausb->reg_access_rx_head.head);
			wake_up(&ausb->wait_read);
			//printk(KERN_INFO "Read --CPU--command ack %02x %02x %02x %02x\n", 
			// cmd->ctrl, cmd->addr, cmd->low, cmd->high);
			return 0;
		}

		case WR_CPU_COMMAND:
			//printk(KERN_INFO "Write --CPU-- command ack %02x %02x %02x %02x\n", 
			// cmd->ctrl, cmd->addr, cmd->low, cmd->high);
			break;

		case RD_FXO_COMMAND:
		{
			struct reg_access_item* ra_item = KMALLOC(sizeof(struct reg_access_item), GFP_ATOMIC, 13);
			memcpy((u8*)&ra_item->cmd, (u8*)cmd, sizeof(*cmd));
			add_cmd_queue(ra_item, &ausb->reg_access_rx_head.head);
			wake_up(&ausb->wait_read);
			//printk(KERN_INFO "Read --FXO-- command ack %02x %02x %02x %02x\n", 
			// cmd->ctrl, cmd->addr, cmd->low, cmd->high);
			return 0;
		}

		case WR_FXO_COMMAND:
			//printk(KERN_INFO "Write --FXO-- command ack %02x %02x %02x %02x\n", 
			// cmd->ctrl, cmd->addr, cmd->low, cmd->high);
			break;

		case RD_FXS_COMMAND:
		{
			struct reg_access_item* ra_item = KMALLOC(sizeof(struct reg_access_item), GFP_ATOMIC, 14);
			memcpy((u8*)&ra_item->cmd, (u8*)cmd, sizeof(*cmd));
			add_cmd_queue(ra_item, &ausb->reg_access_rx_head.head);
			wake_up(&ausb->wait_read);

			//printk(KERN_INFO "Read --FXS indirect-- command ack %02x %02x %02x %02x\n", 
			// cmd->ctrl, cmd->addr, cmd->low, cmd->high);

			return 0;
		}

		case WR_FXS_COMMAND:

			//printk(KERN_INFO "Write --FXS indirect-- command ack %02x %02x %02x %02x\n", 
			// cmd->ctrl, cmd->addr, cmd->low, cmd->high);

			break;

		case NO_COMMAND:
			//do nothing
			//printk(KERN_INFO "no command %02x %02x %02x %02x\n", 
			// cmd->ctrl, cmd->addr, cmd->low, cmd->high);
			break;

		default:
			break;
	}
}

//////////////////////////////////
/* RS>>> QUEUE */


int add_new_cmd_queue(u8* data, struct reg_access_head *head)
{
	struct reg_access_item* ra_item = (struct reg_access_item*)KMALLOC(sizeof(struct reg_access_item), GFP_ATOMIC, 15);
	unsigned long flags;

	if(ra_item != NULL){

	    	memcpy(&ra_item->cmd, data, AMF_USB_CHUNKSIZE);
	    	INIT_LIST_HEAD(&ra_item->list);
		spin_lock_irqsave(&head->list_lock, flags);
	    	list_add(&ra_item->list, &head->head);
		spin_unlock_irqrestore(&head->list_lock, flags);
		return 0;
	} else {
		/* RS>>> todo: handle error */
		return -ENOMEM;
	}
}

void add_cmd_queue(struct reg_access_item* ra_item, struct reg_access_head *head)
{
	unsigned long flags;

	spin_lock_irqsave(&head->list_lock, flags);
	list_add(&ra_item->list, &head->head);
	spin_unlock_irqrestore(&head->list_lock, flags);
}


void add_cmd_fill_data(struct reg_access_item* ra_item, struct reg_access_head *head)
{
	unsigned long flags;
	spin_lock_irqsave(&head->list_lock, flags);
	list_add(&ra_item->list, &head->head);
	spin_unlock_irqrestore(&head->list_lock, flags);
}


/*
void delete_all(struct list_head *head)
{
    struct list_head *iter;
    struct reg_access_item *objPtr;

redo:
    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct reg_access_item, list);
        list_del(&objPtr->list);
        KFREE(objPtr, 29);
        goto redo;
    }
}
*/


//struct reg_access_item *get_cmd_queue(struct list_head* head)
struct reg_access_item *get_cmd_from_queue(struct reg_access_head* head)
{
	struct reg_access_item *objPtr;
	unsigned long flags;

	spin_lock_irqsave(&head->list_lock, flags);
	objPtr = list_last_entry_or_null(&head->head, struct reg_access_item, list);
	spin_unlock_irqrestore(&head->list_lock, flags);
	if(objPtr){
		spin_lock_irqsave(&head->list_lock, flags);
		list_del(&objPtr->list);
		spin_unlock_irqrestore(&head->list_lock, flags);
	}
	return objPtr;
}


struct reg_access_item *peek_cmd_queue(struct reg_access_head* head)
{
	struct reg_access_item *objPtr;
	unsigned long flags;

	spin_lock_irqsave(&head->list_lock, flags);
	objPtr = list_last_entry_or_null(&head->head, struct reg_access_item, list);
	spin_unlock_irqrestore(&head->list_lock, flags);
	return objPtr;
}


