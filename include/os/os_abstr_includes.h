/*
 ****************************************************************************
 * os_abstr_includes.h							
 *									
 * ==========================================================================
 ****************************************************************************
 */

#ifndef __OS_ABSTR_INCLUDES_H
# define __OS_ABSTR_INCLUDES_H

#if !defined(__NetBSD__) && !defined(__FreeBSD__) && !defined (__OpenBSD__) && !defined(__WINDOWS__) && !defined(__Linux__)
# if defined(__KERNEL__)
#  define __Linux__
# endif
#endif

#if defined (__KERNEL__) || defined (KERNEL) || defined (_KERNEL)
# ifndef OS_KERNEL
#  define OS_KERNEL
# endif
#endif


#if defined(__FreeBSD__)
/*
**		***	F R E E B S D	***
# include <stddef.h>
*/
# include <sys/param.h>
# if __FreeBSD_version > 600000
#  include <gnu/fs/ext2fs/i386-bitops.h>
# else
#  include <gnu/ext2fs/i386-bitops.h>
# endif
# include <sys/types.h>
# include <sys/param.h>
# include <sys/systm.h>
# include <sys/syslog.h>
# include <sys/conf.h>
# include <sys/errno.h>
# if (__FreeBSD_version > 400000)
#  include <sys/ioccom.h>
# else
#  include <i386/isa/isa_device.h> 
# endif
# if (__FreeBSD_version >= 410000)
#  include <sys/taskqueue.h>
# endif
# include <sys/malloc.h>
# include <sys/errno.h>
# include <sys/mbuf.h>
# include <sys/sockio.h>
# include <sys/ioctl_compat.h>
# include <sys/socket.h>
# include <sys/callout.h>
# include <sys/kernel.h>
# include <sys/time.h>
# include <sys/module.h>
# include <sys/proc.h>
# include <sys/tty.h>
# include <machine/param.h>
# if (__FreeBSD_version < 500000)
#  include <machine/types.h>
# endif
# include <machine/clock.h>
# include <machine/stdarg.h>
# include <machine/atomic.h>
# include <machine/clock.h>
# include <machine/bus.h>
# include <machine/md_var.h>
# include <vm/vm.h>
# include <vm/pmap.h>
# include <vm/vm_extern.h>
# include <vm/vm_kern.h>

#elif defined(__NetBSD__)
/*
**		***	N E T B S D	***
*/
# include </usr/include/bitstring.h>
# include <sys/types.h>
# include <sys/param.h>
# include <sys/systm.h>
# include <sys/syslog.h>
# include <sys/ioccom.h>
# include <sys/conf.h>
# include <sys/malloc.h>
# include <sys/errno.h>
# include <sys/exec.h>
# include <sys/lkm.h>
# include <sys/mbuf.h>
# include <sys/sockio.h>
# include <sys/socket.h>
# include <sys/kernel.h>
# include <sys/device.h>
# include <sys/time.h>
# include <sys/callout.h>
# include <sys/tty.h>
# include <sys/ttycom.h>
# include <machine/types.h>
# include <machine/param.h>
# include <machine/cpufunc.h>
# include <machine/bus.h>
# include <machine/stdarg.h>
# include <machine/intr.h>
# include <uvm/uvm_extern.h>
#elif defined(__OpenBSD__)
/*
**		***	O P E N B S D	***
*/
# include </usr/include/bitstring.h>
# include <sys/types.h>
# include <sys/systm.h>
# include <sys/param.h>
# include <sys/syslog.h>
# include <sys/ioccom.h>
# include <sys/conf.h>
# include <sys/malloc.h>
# include <sys/errno.h>
# include <sys/exec.h>
# include <sys/lkm.h>
# include <sys/mbuf.h>
# include <sys/sockio.h>
# include <sys/socket.h>
# include <sys/kernel.h>
# include <sys/device.h>
# include <sys/time.h>
# include <sys/timeout.h>
# include <sys/tty.h>
# include <sys/ttycom.h>
# include <i386/bus.h>
# include <machine/types.h>
# include <machine/param.h>
# include <machine/cpufunc.h>
# include <machine/bus.h>
/*# include <machine/stdarg.h>*/
# include <uvm/uvm_extern.h>
#elif defined(__Linux__)
#ifdef __KERNEL__
/*
**		***	L I N U X	***
*/
# include <linux/init.h>
# include <linux/version.h>	/**/
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#  include <linux/config.h>	/* OS configuration options */
# endif
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
#  if !(defined __NO_VERSION__) && !defined(_K22X_MODULE_FIX_)
#   define __NO_VERSION__	
#  endif
# endif
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
  /* Remove experimental SEQ support */
# define _LINUX_SEQ_FILE_H
#endif
# include <linux/module.h>
# include <linux/types.h>
# include <linux/sched.h>
# include <linux/mm.h>
# include <linux/slab.h>
# include <linux/stddef.h>	/* offsetof, etc. */
# include <linux/errno.h>	/* returns codes */
# include <linux/string.h>	/* inline memset, etc */
# include <linux/ctype.h>
# include <linux/kernel.h>	/* printk()m and other usefull stuff */
# include <linux/timer.h>
# include <net/ip.h>
# include <net/protocol.h>
# include <net/sock.h>
# include <net/route.h>
# include <linux/kmod.h>
# include <linux/fcntl.h>
# include <linux/skbuff.h>
# include <linux/socket.h>
# include <linux/poll.h>
# include <linux/wireless.h>
# include <linux/in.h>
# include <linux/inet.h>
# include <linux/miscdevice.h>
# include <linux/netdevice.h>
# include <linux/list.h>
# include <asm/io.h>		/* phys_to_virt() */
# include <asm/system.h>
# include <asm/byteorder.h>
# include <asm/delay.h>
# include <linux/pci.h>
# if defined(CONFIG_PRODUCT_USB)
#  if defined(CONFIG_USB) || defined(CONFIG_USB_SUPPORT)
#   include <linux/usb.h>
#  else
#   warning "USB kernel support not found... disabling usb support"
#   undef CONFIG_PRODUCT_USB
#  endif
# endif
# include <linux/ioport.h>
# include <linux/init.h>
# include <linux/pkt_sched.h>
# include <linux/random.h>
# include <asm/uaccess.h>
# include <linux/vmalloc.h>     /* vmalloc, vfree */
# include <asm/uaccess.h>        /* copy_to/from_user */
# include <linux/init.h>         /* __initfunc et al. */
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#  include <linux/seq_file.h>
# endif
# ifdef CONFIG_INET
#  include <net/inet_common.h>
# endif
#endif
#elif defined(__WINDOWS__)
/*
**		***	W I N D O W S	***
*/
# if !defined(USER_MODE)
#  include "stddcls.h"
#  include <ntddk.h>	/* PCI configuration struct */
#  include <ndis.h>	/* NDIS functions */
#  include <sdla\pnp\busenum.h>
#  include <stdarg.h>
#  include <stdio.h>
# endif
#else
# error "Unsupported Operating System!";
#endif

#endif	/* __OS_ABSTR_INCLUDES_H	*/

