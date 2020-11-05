/*
 ****************************************************************************
 * os_abstr_defines.h							
 *									
 * ==========================================================================
 ****************************************************************************
 */

#ifndef __OS_ABSTR_DEFINES_H
# define __OS_ABSTR_DEFINES_H

/************************************************
 *	SET COMMON KERNEL DEFINE		*
 ************************************************/
#if defined (__KERNEL__) || defined (KERNEL) || defined (_KERNEL)
# ifndef OS_KERNEL
# define OS_KERNEL
# endif
#endif

/************************************************
 *	GLOBAL PLATFORM DEFINITIONS	*
 ************************************************/
#define OS_LINUX_PLATFORM	0x01
#define OS_WIN98_PLATFORM	0x02
#define OS_WINNT_PLATFORM	0x03
#define OS_WIN2K_PLATFORM	0x04
#define OS_FREEBSD_PLATFORM	0x05
#define OS_OPENBSD_PLATFORM	0x06
#define OS_SOLARIS_PLATFORM	0x07
#define OS_SCO_PLATFORM		0x08
#define OS_NETBSD_PLATFORM	0x09

#if defined(__FreeBSD__)
# define OS_PLATFORM_ID		OS_FREEBSD_PLATFORM
#elif defined(__OpenBSD__)
# define OS_PLATFORM_ID		OS_OPENBSD_PLATFORM
#elif defined(__NetBSD__)
# define OS_PLATFORM_ID		OS_NETBSD_PLATFORM
#elif defined(__Linux__)
# define OS_PLATFORM_ID		OS_LINUX_PLATFORM
#elif defined(__WINDOWS__)
# define OS_PLATFORM_ID		OS_WIN2K_PLATFORM
#endif

/*
************************************************
**	GLOBAL DEFINITIONS			
************************************************
*/
#define OS_FALSE	0
#define OS_TRUE	1

/*************************************************
**	GLOBAL TYPEDEF			
*************************************************/

#if defined(__Linux__)
# define os_time_t	 	time_t
# define os_suseconds_t 	suseconds_t	//long
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# if (__FreeBSD_version > 700000)
#  define os_time_t 		time_t
# else /* includes FreeBSD-5/6/OpenBSD/NetBSD */
#  define os_time_t 		long
# endif
# define os_suseconds_t 	suseconds_t
#endif


#if defined(OS_KERNEL)

/*
******************************************************************
**	D E F I N E S
******************************************************************
*/
#if defined(__FreeBSD__)
/******************* F R E E B S D ******************************/
# define OS_MOD_LOAD		MOD_LOAD
# define OS_MOD_UNLOAD		MOD_UNLOAD
# if (__FreeBSD_version > 503000)
#  define OS_MOD_SHUTDOWN		MOD_SHUTDOWN
#  define OS_MOD_QUIESCE		MOD_QUIESCE
# else 
#  define OS_MOD_SHUTDOWN		MOD_UNLOAD+1
#  define OS_MOD_QUIESCE		MOD_UNLOAD+2
# endif
# define OS_DELAY			DELAY
# define OS_NDELAY		DELAY
# define OS_UDELAY		DELAY
# define OS_SYSTEM_TICKS		ticks
# define OS_HZ			hz
# define OS_INB(port)		inb((port))
# define OS_OUTB(port,value)	outb((port),(value))	
# define OS_IOCTL_GROUP(cmd,gr)	(((cmd) >> 8) & (gr))
#elif defined(__OpenBSD__)
/******************* O P E N B S D ******************************/
# define OS_MOD_LOAD	LKM_E_LOAD
# define OS_MOD_UNLOAD	LKM_E_UNLOAD
# define OS_DELAY		DELAY
# define OS_UDELAY	DELAY
# define OS_SYSTEM_TICKS	ticks
# define OS_HZ		hz
#elif defined(__NetBSD__)
/******************* N E T B S D ******************************/
# define OS_MOD_LOAD	LKM_E_LOAD
# define OS_MOD_UNLOAD	LKM_E_UNLOAD
# define OS_DELAY		DELAY
# define OS_UDELAY	DELAY
# define OS_SYSTEM_TICKS	tick
# define OS_HZ		hz
#elif defined(__Linux__)
/*********************** L I N U X ******************************/
# define OS_NDELAY	ndelay
# define OS_UDELAY	udelay
# define OS_SYSTEM_TICKS	jiffies
# define OS_MOD_LOAD	MOD_LOAD
# define OS_MOD_UNLOAD	MOD_UNLOAD
# define OS_INB(port)		inb((port))
# define OS_OUTB(port,value)	outb((value),(port))	
# define OS_IOCTL_GROUP(cmd,gr)	((cmd) >> 8)
#elif defined(__WINDOWS__)
/******************* W I N D O W S ******************************/
# define EINVAL		22
# define IFNAMESIZ	16
#endif

#if defined(__FreeBSD__)
# define OS_MODULE_INC_USE_COUNT
# define OS_MODULE_DEC_USE_COUNT
# define OS_MODULE_VERSION(module, version)			\
	MODULE_VERSION(module, version)
# define OS_MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)	\
	MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)		 
# define OS_MODULE_DEFINE(name,name_str,author,descr,lic,mod_init,mod_exit,devsw)\
	int load_##name (module_t mod, int cmd, void *arg);	\
	int load_##name (module_t mod, int cmd, void *arg){	\
		switch(cmd){					\
		case OS_MOD_LOAD: return mod_init((devsw));	\
		case OS_MOD_UNLOAD: 				\
		case OS_MOD_SHUTDOWN: return mod_exit((devsw));\
		case OS_MOD_QUIESCE: return 0;			\
		}						\
		return -EINVAL;					\
	}							\
	DEV_MODULE(name, load_##name, NULL);

#elif defined(__OpenBSD__)
# define OS_MODULE_INC_USE_COUNT
# define OS_MODULE_DEC_USE_COUNT
# define OS_MODULE_VERSION(module, version)
# define OS_MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)
# define OS_MODULE_DEFINE(name,name_str,author,descr,lic,mod_init,mod_exit,devsw)\
	int (name)(struct lkm_table* lkmtp, int cmd, int ver);\
	MOD_DEV(name_str, LM_DT_CHAR, -1, (devsw));	\
	int load_##name(struct lkm_table* lkm_tp, int cmd){	\
		switch(cmd){					\
		case OS_MOD_LOAD: return mod_init(NULL);	\
		case OS_MOD_UNLOAD: return mod_exit(NULL);	\
		}						\
		return -EINVAL;					\
	}							\
	int (name)(struct lkm_table* lkmtp, int cmd, int ver){\
		DISPATCH(lkmtp,cmd,ver,load_##name,load_##name,lkm_nofunc);\
	}

#elif defined(__NetBSD__)
# define OS_MODULE_INC_USE_COUNT
# define OS_MODULE_DEC_USE_COUNT
# define OS_MODULE_VERSION(module, version)
# define OS_MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)
# if (__NetBSD_Version__ < 200000000)
#  define OS_MOD_DEV(name,devsw) MOD_DEV(name,LM_DT_CHAR,-1,(devsw));
# else
#  define OS_MOD_DEV(name,devsw) MOD_DEV(name,name,NULL,-1,(devsw),-1);
# endif
# define OS_MODULE_DEFINE(name,name_str,author,descr,lic,mod_init,mod_exit,devsw)\
	int (##name_lkmentry)(struct lkm_table* lkmtp, int cmd, int ver);\
	OS_MOD_DEV(name_str, (devsw));				\
	int load_##name(struct lkm_table* lkm_tp, int cmd){	\
		switch(cmd){					\
		case OS_MOD_LOAD: return mod_init(NULL);	\
		case OS_MOD_UNLOAD: return mod_exit(NULL);	\
		}						\
		return -EINVAL;					\
	}							\
	int (##name_lkmentry)(struct lkm_table* lkmtp, int cmd, int ver){\
		DISPATCH(lkmtp,cmd,ver,load_##name,load_##name,lkm_nofunc);\
	}

#elif defined(__Linux__)
# define OS_MODULE_INC_USE_COUNT	MOD_INC_USE_COUNT
# define OS_MODULE_DEC_USE_COUNT	MOD_DEC_USE_COUNT
# define OS_MODULE_VERSION(module, version)
# define OS_MODULE_DEPEND(module, mdepend, vmin, vpref, vmax)
# define OS_MODULE_DEFINE(name,name_str,author,descr,lic,mod_init,mod_exit,devsw)\
	MODULE_AUTHOR (author);					\
	MODULE_DESCRIPTION (descr);				\
	MODULE_LICENSE(lic);					\
	int __init load_##name(void){return mod_init(NULL);}	\
	void __exit unload_##name(void){mod_exit(NULL);}	\
	module_init(load_##name);				\
	module_exit(unload_##name);				
#endif

/*
******************************************************************
**	T Y P E D E F
******************************************************************
*/
#if !defined(offsetof)
# define offsetof(type, member)	((size_t)(&((type*)0)->member))
#endif

#if defined(__Linux__)
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,10)
#  define vsnprintf(a,b,c,d)	vsprintf(a,c,d)
# endif
typedef struct tty_driver       os_ttydriver_t;
typedef struct tty_struct       os_ttystruct_t;

#elif defined(__FreeBSD__)
typedef int		atomic_t;
typedef int		os_spinlock_t;
typedef int		os_smp_flag_t;
typedef dev_t		os_ttydriver_t;
typedef struct tty	os_ttystruct_t;

#elif defined(__OpenBSD__)
typedef int		atomic_t;
typedef int		os_spinlock_t;
typedef int		os_smp_flag_t;

#elif defined(__NetBSD__)
typedef int		atomic_t;
typedef int		os_spinlock_t;
typedef int		os_smp_flag_t;

#elif defined(__WINDOWS__)
typedef char*		caddr_t;

#endif

#endif /* OS_KERNEL */ 

#endif /* __OS_ABSTR_DEFINES_H */
