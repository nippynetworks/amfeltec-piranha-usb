/*
 ****************************************************************************
 * os_abstr_debug.h	Global definition for Debugging messages	
 *									
 * ==========================================================================
 ****************************************************************************
 */

#ifndef __OS_ABSTR_DEBUG_H
# define __OS_ABSTR_DEBUG_H

#define OS_DEBUG_EVENT
#undef OS_DEBUG_KERNEL
#undef OS_DEBUG_MOD
#undef OS_DEBUG_CFG
#undef OS_DEBUG_IOCTL
#undef OS_DEBUG_CMD
#undef OS_DEBUG_ISR
#undef OS_DEBUG_RX
#undef OS_DEBUG_TX
#undef OS_DEBUG_TIMER
#undef OS_DEBUG_UDP
#undef OS_DEBUG_PROCFS
#undef OS_DEBUG_TEST

#undef OS_DEBUG_MEM

#if defined (__WINDOWS__)

# define DEBUG_NONE	if (0)	DbgPrint
# define PRINT		OutputLogString
# define DEBUG_PRINT	DbgPrint
# define _DEBUG_PRINT	DbgPrint

# define DEBUG_KERNEL	DEBUG_NONE
# define DEBUG_EVENT	DEBUG_NONE
# define DEBUG_MOD	DEBUG_NONE
# define DEBUG_CFG	DEBUG_NONE
# define DEBUG_INIT	DEBUG_NONE
# define DEBUG_IOCTL	DEBUG_NONE
# define DEBUG_CMD	DEBUG_NONE
# define DEBUG_ISR	DEBUG_NONE
# define DEBUG_RX	DEBUG_NONE
# define DEBUG_TX	DEBUG_NONE
# define _DEBUG_TX	DEBUG_NONE
# define DEBUG_TIMER	DEBUG_NONE
# define DEBUG_UDP	DEBUG_NONE
# define DEBUG_PROCFS	DEBUG_NONE
# define DEBUG_TEST	DEGUG_NONE

# ifdef OS_DEBUG_KERNEL
#  undef  DEBUG_KERNEL
#  define DEBUG_KERNEL	DEBUG_PRINT
# endif 
# ifdef OS_DEBUG_EVENT
#  undef  DEBUG_EVENT
#  define DEBUG_EVENT	PRINT
# endif 
# ifdef OS_DEBUG_MOD
#  undef  DEBUG_MOD
#  define DEBUG_MOD	DEBUG_PRINT
# endif 
# ifdef OS_DEBUG_CFG
#  undef  DEBUG_CFG
#  define DEBUG_CFG	DEBUG_PRINT
# endif 
# ifdef OS_DEBUG_IOCTL
#  undef  DEBUG_IOCTL
#  define DEBUG_IOCTL	DEBUG_PRINT
# endif
# ifdef OS_DEBUG_CMD
#  undef  DEBUG_CMD
#  define DEBUG_CMD	DEBUG_PRINT
# endif
# ifdef OS_DEBUG_ISR
#  undef  DEBUG_ISR
#  define DEBUG_ISR	DEBUG_PRINT
# endif 
# ifdef OS_DEBUG_RX
#  undef  DEBUG_RX
#  define DEBUG_RX	DEBUG_PRINT
# endif 
# ifdef OS_DEBUG_TX
#  undef  DEBUG_TX
#  define DEBUG_TX	DEBUG_PRINT
#  undef  _DEBUG_TX
#  define _DEBUG_TX	_DEBUG_PRINT
# endif 
# ifdef OS_DEBUG_TIMER
#  undef  DEBUG_TIMER
#  define DEBUG_TIMER	DEBUG_PRINT
# endif 
# ifdef OS_DEBUG_UDP
#  undef  DEBUG_UDP
#  define DEBUG_UDP	DEBUG_PRINT
# endif 
# ifdef OS_DEBUG_PROCFS
#  undef  DEBUG_PROCFS
#  define DEBUG_PROCFS	DEBUG_PRINT
# endif 
# ifdef OS_DEBUG_TEST
#  undef  DEBUG_TEST
#  define DEBUG_TEST	DEBUG_PRINT
# endif



#else	/* !__WINDOWS__*/

# define DEBUG_KERNEL(format,msg...)
# define DEBUG_EVENT(format,msg...)
# define DEBUG_MOD(format,msg...)
# define DEBUG_CFG(format,msg...)
# define DEBUG_INIT(format,msg...)
# define DEBUG_IOCTL(format,msg...)
# define DEBUG_CMD(format,msg...)
# define DEBUG_ISR(format,msg...)
# define DEBUG_RX(format,msg...)
# define DEBUG_TX(format,msg...)	
# define _DEBUG_TX(format,msg...)
# define DEBUG_TIMER(format,msg...)	
# define DEBUG_UDP(format,msg...)
# define DEBUG_PROCFS(format,msg...)
# define DEBUG_TEST(format,msg...)
# define DEBUG_ADD_MEM(a)
# define DEBUG_SUB_MEM(a)

# if (defined __FreeBSD__) || (defined __OpenBSD__) || defined(__NetBSD__)

#  define DEBUG_PRINT(format,msg...)	log(LOG_INFO, format, ##msg)
#  define _DEBUG_PRINT(format,msg...)   log(LOG_INFO, format, ##msg)

# else	/* !__FreeBSD__ && !__OpenBSD__ */

#  define DEBUG_PRINT(format,msg...)	printk(KERN_INFO format, ##msg)
#  define _DEBUG_PRINT(format,msg...)   printk(format,##msg)

# endif	/* __FreeBSD__ || __OpenBSD__ */

# ifdef OS_DEBUG_KERNEL
#  undef  DEBUG_KERNEL
#  define DEBUG_KERNEL(format,msg...)		DEBUG_PRINT(format,##msg)
# endif
# ifdef OS_DEBUG_EVENT
#  undef  DEBUG_EVENT
#  define DEBUG_EVENT(format,msg...)		DEBUG_PRINT(format,##msg)
#  undef  _DEBUG_EVENT
#  define _DEBUG_EVENT(format,msg...)		_DEBUG_PRINT(format,##msg)
# endif 
# ifdef OS_DEBUG_MOD
#  undef  DEBUG_MOD
#  define DEBUG_MOD(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef OS_DEBUG_CFG
#  undef  DEBUG_CFG
#  define DEBUG_CFG(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef OS_DEBUG_IOCTL
#  undef  DEBUG_IOCTL
#  define DEBUG_IOCTL(format,msg...)		DEBUG_PRINT(format,##msg)
# endif
# ifdef OS_DEBUG_CMD
#  undef  DEBUG_CMD
#  define DEBUG_CMD(format,msg...)		DEBUG_PRINT(format,##msg)
# endif
# ifdef OS_DEBUG_ISR
#  undef  DEBUG_ISR
#  define DEBUG_ISR(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef OS_DEBUG_RX
#  undef  DEBUG_RX
#  define DEBUG_RX(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef OS_DEBUG_TX
#  undef  DEBUG_TX
#  define DEBUG_TX(format,msg...)		DEBUG_PRINT(format,##msg)
#  undef  _DEBUG_TX
#  define _DEBUG_TX(format,msg...)		_DEBUG_PRINT(format,##msg)
# endif 
# ifdef OS_DEBUG_TIMER
#  undef  DEBUG_TIMER
#  define DEBUG_TIMER(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef OS_DEBUG_UDP
#  undef  DEBUG_UDP
#  define DEBUG_UDP(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef OS_DEBUG_PROCFS
#  undef  DEBUG_PROCFS
#  define DEBUG_PROCFS(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef OS_DEBUG_TEST
#  undef  DEBUG_TEST
#  define DEBUG_TEST(format,msg...) 		DEBUG_PRINT(format,##msg)
# endif
# ifdef OS_DEBUG_MEM
#  undef  DEBUG_ADD_MEM
#  define DEBUG_ADD_MEM(a)  (atomic_add(a,&wan_debug_mem))
#  undef  DEBUG_SUB_MEM
#  define DEBUG_SUB_MEM(a)  (atomic_sub(a,&wan_debug_mem))
#endif


#endif	/* __WINDOWS__ */

#define OS_DEBUG_FLINE						\
	DEBUG_EVENT("[%s]: %s:%d\n",					\
				__FILE__,__FUNCTION__,__LINE__);

#define OS_ASSERT(val) if (val){					\
	DEBUG_EVENT("************** ASSERT FAILED **************\n");	\
	DEBUG_EVENT("%s:%d - Critical error\n",__FILE__,__LINE__);	\
	return -EINVAL;							\
			}
#define OS_ASSERT_EINVAL(val) OS_ASSERT(val)

#define OS_ASSERT1(val) if (val){					\
	DEBUG_EVENT("************** ASSERT FAILED **************\n");	\
	DEBUG_EVENT("%s:%d - Critical error\n",__FILE__,__LINE__);	\
	return;								\
			}
#define OS_ASSERT_VOID(val) OS_ASSERT1(val)

#define OS_ASSERT2(val, ret) if (val){				\
	DEBUG_EVENT("************** ASSERT FAILED **************\n");	\
	DEBUG_EVENT("%s:%d - Critical error\n",__FILE__,__LINE__);	\
	return ret;							\
			}
#define OS_ASSERT_RC(val,ret) OS_ASSERT2(val, ret)

#define OS_MEM_ASSERT(str) {if (str){				\
		DEBUG_EVENT("%s: Error: No memory in %s:%d\n",		\
					str,__FILE__,__LINE__);		\
	}else{								\
		DEBUG_EVENT("wanpipe: Error: No memory in %s:%d\n",	\
					__FILE__,__LINE__);		\
	}								\
	}

#define OS_OPP_FLAG_ASSERT(val,cmd) if (val){			\
	DEBUG_EVENT("%s:%d - Critical error: Opp Flag Set Cmd=0x%x!\n",	\
					__FILE__,__LINE__,cmd);		\
	}



#define OS_MEM_INIT(id)		unsigned long mem_in_used_##id = 0x0l
#define OS_MEM_INC(id,size)	mem_in_used_##id += size
#define OS_MEM_DEC(id,size)	mem_in_used_##id -= size

#endif /* __OS_ABSTR_DEBUG_H */
