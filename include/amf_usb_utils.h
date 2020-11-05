 /*
  *  amf_usb_utils.h - 'macro and inline utils'
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

#ifndef	__AMF_USB_UTILS_H
#define __AMF_USB_UTILS_H

#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include "os_abstr_debug.h"

#define amf_clear_bit(a,b)				clear_bit((a),(unsigned long*)(b))
#define amf_set_bit(a,b)				set_bit((a),(unsigned long*)(b))
#define amf_test_bit(a,b)				test_bit((a),(unsigned long*)(b))
#define amf_test_and_set_bit(a,b)		test_and_set_bit((a),(unsigned long*)(b))
#define amf_test_and_clear_bit(a,b)		test_and_clear_bit((a),(unsigned long*)(b))

static __inline void amf_delay(int usecs)
{
   if ((usecs) <= 1000) {
   		udelay(usecs) ;
   } else {
       int delay=usecs/1000;
   	   int i;
	   if (delay < 1) {
	   		delay=1;
	   }
	   for (i=0;i<delay;i++) {
           	udelay(1000);
	   }
   }
   return;
}



/********************** KERNEL  **************************/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#define AMF_DEV_NAME(device) dev_name(&(device->dev))
#else
#define AMF_DEV_NAME(device) device->dev.bus_id
#endif

#define AMF_USB_BUSID(hwcard)   ((hwcard)->usb.udev) ? AMF_DEV_NAME((hwcard)->usb.udev) : "Unknown"


//============================== KERNEL BUFFER ==============================

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#define __amf_skb_reset_mac_header(skb)  skb_reset_mac_header(skb)
#define __amf_skb_reset_network_header(skb) skb_reset_network_header(skb)
#define __amf_skb_reset_tail_pointer(skb) skb_reset_tail_pointer(skb)
#define __amf_skb_tail_pointer(skb) skb_tail_pointer(skb)
#define __amf_skb_set_tail_pointer(skb,offset) skb_set_tail_pointer(skb,offset)
#define __amf_skb_end_pointer(skb) skb_end_pointer(skb)
#else
#define __amf_skb_reset_mac_header(skb) (skb->mac.raw = skb->data)
#define __amf_skb_reset_network_header(skb) (skb->nh.raw  = skb->data)
#define __amf_skb_reset_tail_pointer(skb) (skb->tail = skb->data)
#define __amf_skb_tail_pointer(skb) ((netskb_t*)skb)->tail
#define __amf_skb_set_tail_pointer(skb,offset) ((skb)->tail = ((skb)->data + offset))
#define __amf_skb_end_pointer(skb) ((skb)->end)
#endif



static __inline unsigned char* amf_skb_pull(void* skb, int len)
{
	return skb_pull((struct sk_buff*)skb, len);
}

static __inline unsigned char* amf_skb_put(void* skb, int len)
{
	return skb_put((struct sk_buff*)skb, len);
}

static __inline unsigned char* amf_skb_push(void* skb, int len)
{
	return skb_push((struct sk_buff*)skb, len);
}

static __inline void amf_skb_init(void* pskb, unsigned int len)
{
	struct sk_buff* skb = (struct sk_buff*)pskb;
	skb->data = skb->head + len;
	__amf_skb_reset_tail_pointer(skb);
	skb->len  = 0;
	skb->data_len = 0;
}

static __inline void amf_skb_queue_init(void *list)
{
	skb_queue_head_init(list);
}

static __inline void amf_skb_queue_purge(void *list)
{
	struct sk_buff *skb;
	while ((skb=skb_dequeue(list))!=NULL)
	{
		kfree_skb(skb);
	}
}

static __inline void amf_skb_queue_tail(void* list, void* newsk)
{
	skb_queue_tail(list, newsk);
}

static __inline void amf_skb_queue_head(void* list, void* newsk)
{
	skb_queue_head(list, newsk);
}

static __inline int amf_skb_queue_len(void* list)
{
	return skb_queue_len(list);
}

static __inline void *amf_skb_dequeue(void* list)
{
	return skb_dequeue(list);
}


static __inline unsigned char* amf_skb_data(void* skb)
{
	return ((struct sk_buff*)skb)->data;
}

static __inline int amf_skb_len(void* skb)
{
	return ((struct sk_buff*)skb)->len;
}


static __inline void* amf_skb_alloc(unsigned int len)
{
	struct sk_buff *skb = dev_alloc_skb(len+64);
	if (skb){
		skb_reserve(skb,64);
	}
	return (void*)skb;
}

#define AMF_QUE_INIT(ifq)					amf_skb_queue_init((ifq))
#define AMF_QUE_CLEAN(ifq)					amf_skb_queue_purge((ifq))
#define AMF_QUE_DMA_CLEAN(ifq)				amf_skb_queue_purge(ifq)
#define AMF_QUE_ENQUEUE(ifq, skb, arg, err)	amf_skb_queue_tail((ifq), (skb))




//============================== KERNEL BUFFER. ==============================



//==================================   DEBUG  ================================


#undef A_DEBUG
#undef A_DEBUG_USB
#undef A_DEBUG_TX
#undef A_DEBUG_RX

#ifdef AMF_DEBUG
#	ifdef __KERNEL__
#		define A_DEBUG(fmt, args...) DEBUG_EVENT(fmt, ## args)
#		define A_BUG(val)  if (val){														\
						DEBUG_EVENT("****************************************\n");		\
						DEBUG_EVENT("%s:%d - Critical error\n",__FILE__,__LINE__);		\
						}
#   else
#		define A_DEBUG(fmt, args...) fprintf(fmt, ## args)
#   	define A_BUG(val)
#	endif
#else
#	define A_DEBUG(fmt, args...)
#   define A_BUG(val)
#endif

#ifdef  AMF_DEBUG_USB
#	define A_DEBUG_USB(fmt, args...) DEBUG_EVENT(fmt, ## args)
#else
#	define A_DEBUG_USB(fmt, args...)
#endif

#ifdef  AMF_DEBUG_TX
#	define A_DEBUG_TX(fmt, args...) DEBUG_EVENT(fmt, ## args)
#else
#	define A_DEBUG_TX(fmt, args...)
#endif

#ifdef  AMF_DEBUG_RX
#	define A_DEBUG_RX(fmt, args...) DEBUG_EVENT(fmt, ## args)
#else
#	define A_DEBUG_RX(fmt, args...)
#endif


//==================================   DEBUG.  ================================



//==================================== LIST ==================================

#define AMF_LIST_HEAD(name, type)		struct name { struct type * lh_first; }
#define AMF_LIST_HEAD_INITIALIZER(head)	{ NULL }
#define AMF_LIST_ENTRY(type) 			struct { struct type *le_next; struct type **le_prev; }
#define AMF_LIST_FIRST(head)			((head)->lh_first)
#define AMF_LIST_END(head)			NULL
#define AMF_LIST_EMPTY(head)			(AMF_LIST_FIRST(head) == AMF_LIST_END(head))
#define AMF_LIST_NEXT(elm, field)		((elm)->field.le_next)
#define AMF_LIST_FOREACH(var, head, field)	for((var) = AMF_LIST_FIRST(head);	\
							(var);												\
							(var) = AMF_LIST_NEXT(var, field))
# define AMF_LIST_INIT(head)		do { AMF_LIST_FIRST(head) = NULL;}			\
		while(0)

#define	AMF_LIST_INSERT_HEAD(head, elm, field) do {								\
	if ((AMF_LIST_NEXT((elm), field) = AMF_LIST_FIRST((head))) != NULL)			\
		AMF_LIST_FIRST((head))->field.le_prev = &AMF_LIST_NEXT((elm), field);	\
	AMF_LIST_FIRST((head)) = (elm);												\
	(elm)->field.le_prev = &AMF_LIST_FIRST((head));								\
} while (0)
#define	AMF_LIST_INSERT_AFTER(listelm, elm, field) do {							\
	if ((AMF_LIST_NEXT((elm), field) = AMF_LIST_NEXT((listelm), field)) != NULL)\
		AMF_LIST_NEXT((listelm), field)->field.le_prev =						\
		    &AMF_LIST_NEXT((elm), field);										\
	AMF_LIST_NEXT((listelm), field) = (elm);									\
	(elm)->field.le_prev = &AMF_LIST_NEXT((listelm), field);					\
} while (0)
#define	AMF_LIST_REMOVE(elm, field) do {										\
	if (AMF_LIST_NEXT((elm), field) != NULL)									\
		AMF_LIST_NEXT((elm), field)->field.le_prev = 							\
		    (elm)->field.le_prev;												\
	*(elm)->field.le_prev = AMF_LIST_NEXT((elm), field);						\
} while (0)


//==================================== LIST. ==================================



#endif //__AMF_USB_UTILS_H






