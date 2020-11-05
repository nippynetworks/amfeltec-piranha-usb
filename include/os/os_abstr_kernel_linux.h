/*
 ****************************************************************************
 * os_abstr_kernel_linux.h							
 *									
 * ==========================================================================
 ****************************************************************************
 */


#ifndef __OS_ABSTR_LINUX_KERNEL_H
#define __OS_ABSTR_LINUX_KERNEL_H

#if defined(OS_KERNEL)

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,9) 
# define MODULE_LICENSE(a)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,10)
# define snprintf(a,b,c,d...)	sprintf(a,c,##d)
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
/* KERNEL 2.6.X */

 #define LINUX_2_6	
 #define netdevice_t struct net_device

 #if !defined(MOD_INC_USE_COUNT)
 # define MOD_INC_USE_COUNT
 #endif
 #if !defined(MOD_DEC_USE_COUNT)
 # define MOD_DEC_USE_COUNT
 #endif

 #define FREE_READ 1
 #define FREE_WRITE 0

 #define stop_net_queue(a) 	netif_stop_queue(a) 
 #define start_net_queue(a) 	netif_start_queue(a)
 #define is_queue_stopped(a)	netif_queue_stopped(a)
 #define wake_net_dev(a)	netif_wake_queue(a)
 #define is_dev_running(a)	netif_running(a)
 #define os_dev_kfree_skb(a,b)	dev_kfree_skb_any(a)

 #define tq_struct		work_struct

 #define os_call_usermodehelper(a,b,c)   call_usermodehelper(a,b,c,0)

 #define pci_present() 	1

 static inline void os_schedule_task(struct tq_struct *tq)
 {
	schedule_work(tq);
 }

 #define ADMIN_CHECK()  {if (!capable(CAP_SYS_ADMIN)) {\
                             DEBUG_EVENT("wanpipe: ADMIN_CHECK: Failed Cap=0x%X Fsuid=0x%X Euid=0x%X\n", \
				 current->cap_effective,current->fsuid,current->euid);\
	                     return -EPERM; \
 		             }\
                        }

 #define NET_ADMIN_CHECK()  {if (!capable(CAP_NET_ADMIN)){\
	                          DEBUG_EVENT("wanpipe: NET_ADMIN_CHECK: Failed Cap=0x%X Fsuid=0x%X Euid=0x%X\n", \
					 current->cap_effective,current->fsuid,current->euid);\
	                          return -EPERM; \
                                 }\
                            }

 #define OS_IRQ_RETVAL(a) 	return a

 #define mark_bh(a)

 #define os_clear_bit(a,b)  	    clear_bit((a),(unsigned long*)(b))
 #define os_set_bit(a,b)    	    set_bit((a),(unsigned long*)(b))
 #define os_test_bit(a,b)   	    test_bit((a),(unsigned long*)(b))
 #define os_test_and_set_bit(a,b)  test_and_set_bit((a),(unsigned long*)(b))
 #define os_test_and_clear_bit(a,b)  test_and_clear_bit((a),(unsigned long*)(b))

 #define dev_init_buffers(a)

 #define WP_PDE(_a)		PDE(_a)


 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9)
 # define os_rcu_read_lock(in_dev)     rcu_read_lock()
 # define os_rcu_read_unlock(in_dev)   rcu_read_unlock()
 # define os_readb(ptr)		       readb((void __iomem *)(ptr))
 # define os_reads(ptr)		       reads((void __iomem *)(ptr))
 # define os_readl(ptr)		       readl((void __iomem *)(ptr))
 # define os_writeb(data,ptr)	       writeb(data,(void __iomem *)(ptr))
 # define os_writew(data,ptr)	       writew(data,(void __iomem *)(ptr))
 # define os_writel(data,ptr)	       writel(data,(void __iomem *)(ptr))
 # define os_memset_io(ptr,data,len)   memset_io((void __iomem *)(ptr),data,len)
 #else
 # define os_rcu_read_lock(in_dev)     read_lock_bh(&in_dev->lock) 
 # define os_rcu_read_unlock(in_dev)   read_unlock_bh(&in_dev->lock) 
 # define os_readb(ptr)		       readb((ptr))
 # define os_reads(ptr)		       reads((ptr))
 # define os_readl(ptr)		       readl((ptr))
 # define os_writeb(data,ptr)	       writeb(data,(ptr))
 # define os_writew(data,ptr)	       writew(data,(ptr))
 # define os_writel(data,ptr)	       writel(data,(ptr))
 # define os_memset_io(ptr,data,len)   memset_io((ptr),data,len)
 #endif

# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34)
#  define os_usb_buffer_allocate	usb_buffer_allocate
#  define os_usb_buffer_free		usb_buffer_free
# else
#  define os_usb_buffer_allocate	usb_allocate_coherent
#  define os_usb_buffer_free		usb_free_coherent
#endif
 

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
/* --------------------------------------------------
 * KERNEL 2.4.X 
 * -------------------------------------------------*/
 
 #define LINUX_2_4
 #define netdevice_t struct net_device

 #define FREE_READ 1
 #define FREE_WRITE 0

 #define stop_net_queue(a) 	netif_stop_queue(a) 
 #define start_net_queue(a) 	netif_start_queue(a)
 #define is_queue_stopped(a)	netif_queue_stopped(a)
 #define wake_net_dev(a)	netif_wake_queue(a)
 #define is_dev_running(a)	netif_running(a)
 #define os_dev_kfree_skb(a,b)	dev_kfree_skb_any(a)
 #define pci_get_device(a,b,c)  pci_find_device(a,b,c)

 #define __dev_get(a)		dev_get(a)

 static inline void os_schedule_task(struct tq_struct *tq)
 {
	schedule_task(tq);
 }

 #ifndef INIT_WORK
 # define INIT_WORK INIT_TQUEUE
 #endif

 #define os_call_usermodehelper(a,b,c)   call_usermodehelper(a,b,c)

 #define ADMIN_CHECK()  {if (!capable(CAP_SYS_ADMIN)) {\
                             DEBUG_EVENT("wanpipe: ADMIN_CHECK: Failed Cap=0x%X Fsuid=0x%X Euid=0x%X\n", \
				 current->cap_effective,current->fsuid,current->euid);\
	                     return -EPERM; \
 		             }\
                        }

 #define NET_ADMIN_CHECK()  {if (!capable(CAP_NET_ADMIN)){\
	                          DEBUG_EVENT("wanpipe: NET_ADMIN_CHECK: Failed Cap=0x%X Fsuid=0x%X Euid=0x%X\n", \
					 current->cap_effective,current->fsuid,current->euid);\
	                          return -EPERM; \
                                 }\
                            }

 #define OS_IRQ_RETVAL(a)	return
 #ifndef IRQ_NONE
 # define IRQ_NONE	(0)
 #endif

 #ifndef IRQ_HANDLED
 # define IRQ_HANDLED	(1)
 #endif

 #define irqreturn_t    void

 #define os_clear_bit(a,b)  	    clear_bit((a),(b))
 #define os_set_bit(a,b)    	    set_bit((a),(b))
 #define os_test_bit(a,b)   	    test_bit((a),(b))
 #define os_test_and_set_bit(a,b)  test_and_set_bit((a),(b))
 #define os_test_and_clear_bit(a,b)  test_and_clear_bit((a),(b))

 static inline struct proc_dir_entry *WP_PDE(const struct inode *inode)
 {
        return (struct proc_dir_entry *)inode->u.generic_ip;
 }

 #define os_rcu_read_lock(in_dev)     read_lock_bh(&in_dev->lock) 
 #define os_rcu_read_unlock(in_dev)   read_unlock_bh(&in_dev->lock) 
 #define os_readb(ptr)		      readb((ptr))
 #define os_reads(ptr)		      reads((ptr))
 #define os_readl(ptr)		      readl((ptr))

 #define os_writeb(data,ptr)	       writeb(data,(ptr))
 #define os_writew(data,ptr)	       writew(data,(ptr))
 #define os_writel(data,ptr)	       writel(data,(ptr))
 #define os_memset_io(ptr,data,len)   memset_io((ptr),data,len)



#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,1,0)
/*-----------------------------------------------------
 * KERNEL 2.2.X 
 * ---------------------------------------------------*/

 #define LINUX_2_1
 #define net_device  device
 #define netdevice_t struct device
 #define FREE_READ 1
 #define FREE_WRITE 0

 #define S_IRUGO	0

 #define __exit

 #ifndef get_order
 # define get_order(x) __get_order(x)
 #endif

 #define pci_get_device(a,b,c)  pci_find_device(a,b,c)
 #define os_dev_kfree_skb(a,b)	kfree_skb(a)
 #define dev_kfree_skb_any(a)   kfree_skb(a)

 #define netif_wake_queue(dev)   do { \
                                    clear_bit(0, &(dev)->tbusy); \
                                    mark_bh(NET_BH); \
                                } while(0)
 #define netif_start_queue(dev)  do { \
                                    (dev)->tbusy = 0; \
                                    (dev)->interrupt = 0; \
                                    (dev)->start = 1; \
                                } while (0)

 #define netif_stop_queue(dev)    (set_bit(0, &(dev)->tbusy))
 #define netif_running(dev)       (dev)->start
 #define netdevice_start(dev)     (dev)->start = 1
 #define netdevice_stop(dev)      (dev)->start = 0
 #define netif_queue_stopped(dev) (test_bit(0,&(dev)->tbusy))
 #define netif_set_tx_timeout(dev, tf, tm)

 #define stop_net_queue(dev) 	netif_stop_queue(dev) 
 #define start_net_queue(dev) 	netif_start_queue(dev)
 #define is_queue_stopped(dev)	netif_queue_stopped(dev)
 #define wake_net_dev(dev)	netif_wake_queue(dev)
 #define is_dev_running(dev)	netif_running(dev)

 #define dev_kfree_skb_irq(x)   kfree_skb(x)

 #define tasklet_struct 	tq_struct

 #define __dev_get(a)		dev_get(a)
 
 #ifndef DECLARE_WAITQUEUE
 #define DECLARE_WAITQUEUE(wait, current)	struct wait_queue wait = { current, NULL }
 #endif

 #define tasklet_kill(a)  { if ((a)->sync) {} }

 #define request_mem_region(addr, size, name)	((void *)1)
 #define release_mem_region(addr, size)
 #define pci_enable_device(x)           (0)
 #define pci_resource_start(dev, bar)   dev->base_address[bar]

 #define os_rcu_read_lock(in_dev)    
 #define os_rcu_read_unlock(in_dev)   
 #define os_readb(ptr)		       readb((ptr))
 #define os_reads(ptr)		       reads((ptr))
 #define os_readl(ptr)		       readl((ptr))
 #define os_writeb(data,ptr)	       writeb(data,(ptr))
 #define os_writew(data,ptr)	       writew(data,(ptr))
 #define os_writel(data,ptr)	       writel(data,(ptr))
 #define os_memset_io(ptr,data,len)   memset_io((ptr),data,len)

 static inline void tasklet_hi_schedule(struct tasklet_struct *tasklet)
 {
	queue_task(tasklet, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
 } 

 static inline void tasklet_schedule(struct tasklet_struct *tasklet)
 {
	queue_task(tasklet, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
 }

 static inline void tasklet_init(struct tasklet_struct *tasklet,
				void (*func)(unsigned long),
				unsigned long data)
 {
	tasklet->next = NULL;
	tasklet->sync = 0;
	tasklet->routine = (void (*)(void *))func;
	tasklet->data = (void *)data;
 }

 static inline void os_schedule_task(struct tq_struct *tq)
 {
	queue_task(tq, &tq_scheduler);
 }

 /* Setup Dma Memory size copied directly from 3c505.c */
 static inline int __get_order(unsigned long size)
 {
        int order;

        size = (size - 1) >> (PAGE_SHIFT - 1);
        order = -1;
        do {
                size >>= 1;
                order++;
        } while (size);
        return order;
 }

 typedef int (get_info_t)(char *, char **, off_t, int, int);

 #define ADMIN_CHECK()  {if (!capable(CAP_SYS_ADMIN)) return -EPERM;}
 #define NET_ADMIN_CHECK()  {if (!capable(CAP_NET_ADMIN)) return -EPERM;}

 #define OS_IRQ_RETVAL(a)      return
 #ifndef IRQ_NONE
 # define IRQ_NONE      (0)
 #endif

 #ifndef IRQ_HANDLED
 # define IRQ_HANDLED   (1)
 #endif

 typedef unsigned long mm_segment_t;
 #define irqreturn_t    void

 #ifndef INIT_WORK
 # define INIT_WORK INIT_TQUEUE
 #endif
 
 #define os_clear_bit(a,b)  	    clear_bit((a),(b))
 #define os_set_bit(a,b)    	    set_bit((a),(b))
 #define os_test_bit(a,b)   	    test_bit((a),(b))
 #define os_test_and_set_bit(a,b)  test_and_set_bit((a),(b))
 #define os_test_and_clear_bit(a,b)  test_and_clear_bit((a),(b))

 static inline struct proc_dir_entry *WP_PDE(const struct inode *inode)
 {
        return (struct proc_dir_entry *)inode->u.generic_ip;
 }

#else
/* KERNEL 2.0.X */

 
 #define LINUX_2_0
 #define netdevice_t struct device

 static inline struct proc_dir_entry *WP_PDE(const struct inode *inode)
 {
        return (struct proc_dir_entry *)inode->u.generic_ip;
 }

 #define test_and_set_bit set_bit
 #define net_ratelimit() 1 

 #define stop_net_queue(a) 	(set_bit(0, &a->tbusy)) 
 #define start_net_queue(a) 	(clear_bit(0,&a->tbusy))
 #define is_queue_stopped(a)	(a->tbusy)
 #define wake_net_dev(a)	{clear_bit(0,&a->tbusy);mark_bh(NET_BH);}
 #define is_dev_running(a)	(test_bit(0,(void*)&a->start))
 #define os_dev_kfree_skb(a,b) kfree_skb(a,b)  		 
 #define pci_get_device(a,b,c)  pci_find_device(a,b,c)
 #define spin_lock_init(a)
 #define spin_lock(a)
 #define spin_unlock(a)

 #define __dev_get(a)		dev_get(a)

 #define netif_wake_queue(dev)   do { \
                                    clear_bit(0, &dev->tbusy); \
                                    mark_bh(NET_BH); \
                                } while(0)
 #define netif_start_queue(dev)  do { \
                                    dev->tbusy = 0; \
                                    dev->interrupt = 0; \
                                    dev->start = 1; \
                                } while (0)
 #define netif_stop_queue(dev)   set_bit(0, &dev->tbusy)
 #define netif_running(dev)      dev->start
 #define netdevice_start(dev)    dev->start = 1
 #define netdevice_stop(dev)     dev->start = 0
 #define netif_set_tx_timeout(dev, tf, tm)
 #define dev_kfree_skb_irq(x)    kfree_skb(x)

 typedef int (write_proc_t)(char *, char **, off_t, int, int);

 #define net_device_stats	enet_statistics	

 static inline int copy_from_user(void *a, void *b, int len){
	int err = verify_area(VERIFY_READ, b, len);
	if (err)
		return err;
		
	memcpy_fromfs (a, b, len);
	return 0;
 }

 static inline int copy_to_user(void *a, void *b, int len){
	int err = verify_area(VERIFY_WRITE, b, len);
	if (err)
		return err;
	memcpy_tofs (a, b,len);
	return 0;
 }

 #define OS_IRQ_RETVAL(a)      return
 #ifndef IRQ_NONE
 # define IRQ_NONE      (0)
 #endif

 #ifndef IRQ_HANDLED
 # define IRQ_HANDLED   (1)
 #endif

 typedef unsigned long mm_segment_t;
 #define irqreturn_t    void

#endif

static inline int open_dev_check(netdevice_t *dev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,18)
	return is_dev_running(dev);
#else
	return 0;
#endif
}

#else

#include <linux/version.h>

/* This file is not being included from kernel space
 * we need to define what kersdladrv_pci_tblnel version we are
 * running */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	#define LINUX_2_6
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
 	#define LINUX_2_4
#else
	#define LINUX_2_4
#endif


#endif
#endif
