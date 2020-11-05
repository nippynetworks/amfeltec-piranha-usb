/****************************************************************************
 * os_abstr_basic.h	
 *
 *
 * ==========================================================================
 ****************************************************************************
 */

#ifndef	__OS_ABSTR_BASIC_H
# define __OS_ABSTR_BASIC_H

#if defined(__Linux__)
# include "os_abstr_kernel_linux.h"
#elif defined(__FreeBSD__)
# include "os_abstr_kernel_freebsd.h"
#endif

/*
*****************************************************************************
**			D E F I N E S				
*****************************************************************************
*/

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# define OS_LIST_HEAD(name, type)			LIST_HEAD(name, type)
# define OS_LIST_HEAD_INITIALIZER(head)		LIST_HEAD_INITIALIZER(head)
# define OS_LIST_ENTRY(type) 				LIST_ENTRY(type)
# define OS_LIST_EMPTY(head)				LIST_EMPTY(head)
# define OS_LIST_FIRST(head)				LIST_FIRST(head)		
# define OS_LIST_FOREACH(var, head, field)		LIST_FOREACH(var, head, field)
# define OS_LIST_INIT(head)				LIST_INIT(head)
# define OS_LIST_INSERT_AFTER(listelm, elm, field)	LIST_INSERT_AFTER(listelm, elm, field)
/*# define OS_LIST_INSERT_BEFORE(listelm, elm, field)	LIST_INSERT_BEFORE(listelm, elm, field)*/
# define OS_LIST_INSERT_HEAD(head, elm, field) 	LIST_INSERT_HEAD(head, elm, field)
# define OS_LIST_NEXT(elm, field)			LIST_NEXT(elm, field)
# define OS_LIST_REMOVE(elm, field)			LIST_REMOVE(elm, field)

#elif defined(__Linux__)
	
# define OS_LIST_HEAD(name, type)		struct name { struct type * lh_first; }
# define OS_LIST_HEAD_INITIALIZER(head)	{ NULL }
# define OS_LIST_ENTRY(type) 			struct { struct type *le_next; struct type **le_prev; }
# define OS_LIST_FIRST(head)			((head)->lh_first)
# define OS_LIST_END(head)			NULL
# define OS_LIST_EMPTY(head)			(OS_LIST_FIRST(head) == OS_LIST_END(head))
# define OS_LIST_NEXT(elm, field)		((elm)->field.le_next)
# define OS_LIST_FOREACH(var, head, field)	for((var) = OS_LIST_FIRST(head);	\
							(var);				\
							(var) = OS_LIST_NEXT(var, field))
# define OS_LIST_INIT(head)		do { OS_LIST_FIRST(head) = NULL;}\
		while(0)

#define	OS_LIST_INSERT_HEAD(head, elm, field) do {				\
	if ((OS_LIST_NEXT((elm), field) = OS_LIST_FIRST((head))) != NULL)	\
		OS_LIST_FIRST((head))->field.le_prev = &OS_LIST_NEXT((elm), field);\
	OS_LIST_FIRST((head)) = (elm);					\
	(elm)->field.le_prev = &OS_LIST_FIRST((head));			\
} while (0)
#define	OS_LIST_INSERT_AFTER(listelm, elm, field) do {			\
	if ((OS_LIST_NEXT((elm), field) = OS_LIST_NEXT((listelm), field)) != NULL)\
		OS_LIST_NEXT((listelm), field)->field.le_prev =		\
		    &OS_LIST_NEXT((elm), field);				\
	OS_LIST_NEXT((listelm), field) = (elm);				\
	(elm)->field.le_prev = &OS_LIST_NEXT((listelm), field);		\
} while (0)
#define	OS_LIST_REMOVE(elm, field) do {					\
	if (OS_LIST_NEXT((elm), field) != NULL)				\
		OS_LIST_NEXT((elm), field)->field.le_prev = 		\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = OS_LIST_NEXT((elm), field);		\
} while (0)

#else
# error "OS_LISTx macros not supported yet!"
#endif

#if defined(__FreeBSD__)
# if (__FreeBSD_version < 410000)
#  define OS_TASKLET_INIT(task, priority, func, arg)	\
	(task)->running = 0;					\
	(task)->task_func = func; (task)->data = arg
#  define OS_TASKLET_SCHEDULE(task)			\
	if (!wan_test_bit(0, &(task)->running)){		\
		os_set_bit(0, &(task)->running);		\
		(task)->task_func((task)->data, 0);		\
	}
#  define OS_TASKLET_END(task)	os_clear_bit(0, &(task)->running)
#  define OS_TASKLET_KILL(task)
# else
#  define OS_TASKLET_INIT(task, priority, func, arg)		\
	(task)->running = 0;						\
	TASK_INIT(&(task)->task_id, priority, func, (void*)arg)
#  define OS_TASKLET_SCHEDULE(task)				\
	if (!os_test_bit(0, &(task)->running)){			\
		os_set_bit(0, &(task)->running);			\
		taskqueue_enqueue(taskqueue_swi, &(task)->task_id);	\
	}
/*		taskqueue_run(taskqueue_swi);				\*/
#  define OS_TASKLET_END(task)	os_clear_bit(0, &(task)->running)
#  define OS_TASKLET_KILL(task)
# endif
#elif defined(__OpenBSD__) || defined(__NetBSD__)
# define OS_TASKLET_INIT(task, priority, func, arg)		\
	(task)->running = 0;						\
	(task)->task_func = func; (task)->data = arg
# define OS_TASKLET_SCHEDULE(task)				\
	if (!os_test_bit(0, &(task)->running)){			\
		os_set_bit(0, &(task)->running);			\
		(task)->task_func((task)->data, 0);			\
	}
# define OS_TASKLET_END(task)	os_clear_bit(0, &(task)->running)
# define OS_TASKLET_KILL(task)
#elif defined(__Linux__)
# define OS_TASKLET_INIT(task, priority, func, arg)		\
	(task)->running = 0;						\
	tasklet_init(&(task)->task_id,func,(unsigned long)arg) 
# define OS_TASKLET_SCHEDULE(task)				\
	tasklet_hi_schedule(&(task)->task_id);			
# define OS_TASKLET_END(task)	os_clear_bit(0, &(task)->running)
# define OS_TASKLET_KILL(task)	tasklet_kill(&(task)->task_id)
#elif defined(__WINDOWS__)
#else
# error "Undefined OS_TASKLET_x macro!"
#endif

#if defined(__Linux__)
# define OS_COPY_FROM_USER(k,u,l)	copy_from_user(k,u,l)
# define OS_COPY_TO_USER(u,k,l)	copy_to_user(u,k,l)
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# define OS_COPY_FROM_USER(k,u,l)	copyin(u,k,l)
# define OS_COPY_TO_USER(u,k,l)	copyout(k,u,l)
#elif defined(__WINDOWS__)
#else
# error "Undefined OS_COPY_FROM_USER/OS_COPY_TO_USER macros!"
#endif

#if defined (__Linux__)
# define os_clear_bit(a,b)		clear_bit((a),(unsigned long*)(b))
# define os_set_bit(a,b)    		set_bit((a),(unsigned long*)(b))
# define os_test_bit(a,b)			test_bit((a),(unsigned long*)(b))
# define os_test_and_set_bit(a,b)  	test_and_set_bit((a),(unsigned long*)(b))
# define os_test_and_clear_bit(a,b)	test_and_clear_bit((a),(unsigned long*)(b))

#elif defined(__FreeBSD__)
# define os_clear_bit(bit, name)	clear_bit(bit, (name))
# define os_test_bit(bit, name)	test_bit(bit, (name))
# define os_set_bit(bit, name)	set_bit(bit, (name))
# define os_test_and_set_bit(bit, name)	\
		(test_bit((bit), (name)) || set_bit((bit),(name)))
	
#elif defined(__OpenBSD__)
# define os_clear_bit(bit, name)	bit_clear((unsigned char*)(name), bit)
# define os_set_bit(bit, name)	bit_set((unsigned char*)(name), bit)
# define os_test_bit(bit, name)	bit_test((unsigned char*)(name), bit)
# define os_test_and_set_bit(bit, name)		\
    	(bit_test((unsigned char*)(name),(bit)) || 	\
		!bit_set((unsigned char*)(name),(bit)))

#elif defined(__NetBSD__)
# define os_clear_bit(bit, name)	bit_clear((unsigned char*)(name), bit)
# define os_set_bit(bit, name)	bit_set((unsigned char*)(name), bit)
# define os_test_bit(bit, name)	bit_test((unsigned char*)(name), bit)
# define os_test_and_set_bit(bit, name)		\
    	(bit_test((unsigned char*)(name),(bit)) || 	\
		!bit_set((unsigned char*)(name),(bit)))

#else
# error "ob_abstr_set_bit/os_clear_bit/os_test_bit macros doesn't supported yet!"
#endif


#if defined (__Linux__)
# define OS_HOLD(str)	wan_atomic_inc(&(str)->refcnt)
# define __OS_PUT(str)	wan_atomic_dec(&(str)->refcnt)
# define OS_PUT(str)	if (atomic_dec_and_test(&(str)->refcnt)){ \
		       			OS_kfree(str); 		\
                       		} 
#elif defined(__FreeBSD__)
# define OS_HOLD(str)	wan_atomic_inc(&(str)->refcnt)
# define __OS_PUT(str)	wan_atomic_dec(&(str)->refcnt)
# define OS_PUT(str)	wan_atomic_dec(&str->refcnt);	\
				if (str->refcnt){		\
		       			OS_FREE(str);		\
                       		} 
#elif defined(__NetBSD__) || defined(__OpenBSD__)
# define OS_HOLD(str)	str->refcnt++
# define __OS_PUT(str)	str->refcnt--
# define OS_PUT(str)	str->refcnt--;	\
				if (str->refcnt){		\
		       			OS_FREE(str);		\
	                       	} 
#elif defined(__WINDOWS__)
#else
# warning "Undefined OS_HOLD/OS_PUT macro!"
#endif

/*
** TASKLET structure
*/
#if defined(__Linux__)
typedef void 		os_tasklet_func_t(unsigned long);
#elif defined(__FreeBSD__) 
typedef task_fn_t	os_tasklet_func_t;
#elif defined(__OpenBSD__) || defined(__NetBSD__)
typedef void 		os_tasklet_func_t(void*, int);
#endif
typedef struct _os_tasklet
{
	unsigned long		running;
#if defined(__FreeBSD__) && (__FreeBSD_version >= 410000)
	struct task		task_id;
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	os_tasklet_func_t	*task_func;
	void*			data;	
#elif defined(__Linux__)
	struct tasklet_struct 	task_id;
#endif
} os_tasklet_t;

/*
*****************************************************************************
**		I N L I N E   O S   A B S T R   F U N C T I O N S				
*****************************************************************************
*/
/******************* MALLOC/FREE FUNCTION ******************/
/*
** os_malloc - 
*/
static __inline void* os_malloc(int size)
{
	void*	ptr = NULL;
#if defined(__Linux__)
	ptr = kmalloc(size, GFP_ATOMIC);
	if (ptr){
		DEBUG_ADD_MEM(size);
	}
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	ptr = malloc(size, M_DEVBUF, M_NOWAIT); 
#elif defined(__WINDOWS__)
	ptr = ExAllocatePool(NonPagedPool, size);
#else
# error "os_malloc() function is not supported yet!"
#endif
	if (ptr){
		memset(ptr, 0, size);
		DEBUG_ADD_MEM(size);
	}
	return ptr;
}

/*
** os_free - 
*/
static __inline void os_free(void* ptr)
{
	if (!ptr){
		DEBUG_EVENT("wan_free: NULL PTR !!!!!\n");
		return;
	}
	
#if defined(__Linux__)
	kfree(ptr);
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	return free(ptr, M_DEVBUF); 
#elif defined(__WINDOWS__)
	ExFreePool(ptr);
#else
# error "os_free() function is not supported yet!"
#endif
}




#endif	/* __OS_ABSTR_BASIC_H */
