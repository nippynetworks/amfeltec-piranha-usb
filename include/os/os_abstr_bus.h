/****************************************************************************
 * os_abstr_bus.h
 *
 *
* ==========================================================================
 ****************************************************************************
 */

#ifndef	__OS_ABSTR_BUS_H
# define __OS_ABSTR_BUS_H

#if defined(OS_KERNEL)

#if defined(__Linux__)
# if defined(__iomem)
typedef void __iomem*		os_mem_handle_t;
# else
typedef void *		        os_mem_handle_t;
# endif
typedef	unsigned long 		os_base_addr_t;
#elif defined(__FreeBSD__)
typedef bus_space_handle_t	os_mem_handle_t;
typedef	bus_addr_t 		os_base_addr_t;
#endif

/******************* VIRT<->BUS SPACE FUNCTION ******************/
/*
** os_virt2bus
*/
static __inline unsigned long os_virt2bus(unsigned long* ptr)
{
#if defined(__Linux__)
	return virt_to_bus(ptr);
#elif defined(__FreeBSD__)
	return vtophys((vm_offset_t)ptr);
#elif defined(__OpenBSD__) || defined(__NetBSD__)
	return vtophys((vaddr_t)ptr);
#elif defined(__WINDOWS__)
#else
# error "os_virt2bus() function is not supported yet!"
#endif
}

/*
** os_bus2virt
*/
static __inline unsigned long* os_bus2virt(unsigned long virt_addr)
{
#if defined(__Linux__)
	return bus_to_virt(virt_addr);
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	return (unsigned long*)virt_addr;
#elif defined(__WINDOWS__)
#else
# error "os_bus2virt() function is not supported yet!"
#endif
}

static __inline int 
os_bus_space_map(os_base_addr_t base_addr, int reg, int size, os_mem_handle_t* handle)
{
	int		err = 0;
	
#if defined(__FreeBSD__)
	*handle = (os_mem_handle_t)pmap_mapdev(base_addr + reg, size);
#elif defined(__Linux__)
	*handle = (os_mem_handle_t)ioremap(base_addr+reg, size);
#elif defined(__WINDOWS__)
	{
	PHYSICAL_ADDRESS tmpPA;
	tmpPA.QuadPart = (base_addr+reg);
	*handle = MmMapIoSpace(tmpPA, (ULONG)size, MmNonCached);
	}
	if(*handle == NULL){
		err = 1;
	}
#else
# warning "os_bus_space_map: Not supported yet!"
#endif
	return err;

}

static __inline void 
os_bus_space_unmap(os_mem_handle_t offset, int size)
{

#if defined(__FreeBSD__)
	int i = 0;
	for (i = 0; i < size; i += PAGE_SIZE){
	    pmap_kremove((vm_offset_t)offset + i);
	}
	kmem_free(kernel_map, (vm_offset_t)offset, size);
#elif defined(__Linux__)
	iounmap((void*)offset);
#elif defined(__WINDOWS__)
	MmUnmapIoSpace(offset, size);
#else
# warning "os_bus_space_unmap: Not supported yet!"
#endif

}


static __inline int
os_bus_read_4(os_mem_handle_t memh, unsigned int offset, uint32_t* value)
{

#if defined(__FreeBSD__)
	*value = readl(((u8*)memh + offset));
#elif defined(__Linux__)
	*value = os_readl((uint8_t*)memh + offset);
#elif defined(__WINDOWS__)
	*value = READ_REGISTER_ULONG((PULONG)((PUCHAR)memh + offset));
#else
	*value = 0;
# warning "os_bus_read_4: Not supported yet!"
#endif

	return 0;
}

static __inline int
os_bus_read_2(os_mem_handle_t memh, unsigned int offset, u16* value)
{

#if defined(__FreeBSD__)
	*value = readw(((u8*)memh + offset));
#elif defined(__Linux__)
	*value = readw(((unsigned char*)memh+offset));
#elif defined(__WINDOWS__)
	*value = READ_REGISTER_USHORT((PUSHORT)((PUCHAR)memh + offset));
#else
# warning "os_bus_read_2: Not supported yet!"
#endif
	return 0;
}

static __inline int
os_bus_read_1(os_mem_handle_t memh, unsigned int offset, u8* value)
{

#if defined(__FreeBSD__)
	*value = readb(((u8*)memh + offset));
#elif defined(__Linux__)
	*value = os_readb((memh + offset));
#elif defined(__WINDOWS__)
	*value = READ_REGISTER_UCHAR((PUCHAR)((PUCHAR)memh + offset));
#else
# warning "os_bus_read_1: Not supported yet!"
#endif
	return 0;
}

static __inline int 
os_bus_write_4(os_mem_handle_t memh, unsigned int offset, u32 value) 
{

#if defined(__FreeBSD__)
	writel(((u8*)memh + offset), value);
#elif defined(__Linux__)
	os_writel(value,(uint8_t*)memh + offset);
#elif defined(__WINDOWS__)
	WRITE_REGISTER_ULONG((PULONG)((PUCHAR)memh + offset), value);
#else
# warning "os_bus_write_4: Not supported yet!"
#endif
	return 0;
}

static __inline int 
os_bus_write_2(os_mem_handle_t memh, unsigned int offset, u16 value) 
{

#if defined(__FreeBSD__)
	writew(((u8*)memh + offset), value);
#elif defined(__Linux__)
	os_writew(value,memh+offset);
#elif defined(__WINDOWS__)
	WRITE_REGISTER_USHORT((PUSHORT)((PUCHAR)memh + offset), value);
#else
# warning "os_bus_write_2: Not supported yet!"
#endif
	return 0;
}

static __inline int 
os_bus_write_1(os_mem_handle_t memh, unsigned int offset, u8 value) 
{

#if defined(__FreeBSD__)
	writeb(((u8*)memh + offset), value);
#elif defined(__Linux__)
	os_writeb(value, memh + offset);
#elif defined(__WINDOWS__)
	WRITE_REGISTER_UCHAR((PUCHAR)((PUCHAR)memh + offset), value);
#else
# warning "os_bus_write_1: Not supported yet!"
#endif
	return 0;
}




#endif  /* OS_KERNEL */
#endif	/* __OS_ABSTR_BUS_H */
