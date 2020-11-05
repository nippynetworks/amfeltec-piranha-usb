/*
 ****************************************************************************
 * os_abstr_pci.h	
 *
 * ==========================================================================
 ****************************************************************************
 */

#ifndef	__OS_ABSTR_PCI_H
# define __OS_ABSTR_PCI_H

#if defined(__FreeBSD__)
# include <sys/bus.h>
# include <sys/pciio.h>
# include <dev/pci/pcivar.h>
# include <dev/pci/pcireg.h>
#elif defined(__Linux__)
#endif

#if defined(__Linux__)
# define OS_PCI_HEADER_TYPE_BRIDGE	PCI_HEADER_TYPE_BRIDGE
# define OS_PCI_SECONDARY_BUS		PCI_SECONDARY_BUS
# define OS_PCI_SUBORDINATE_BUS		PCI_SUBORDINATE_BUS
#elif defined(__FreeBSD__)
# define OS_PCI_HEADER_TYPE		PCIR_HDRTYPE
# define OS_PCI_HEADER_TYPE_BRIDGE	PCIM_HDRTYPE_BRIDGE
# define OS_PCI_PRIMARY_BUS		PCIR_PRIBUS_1
# define OS_PCI_SECONDARY_BUS		PCIR_SECBUS_1
# define OS_PCI_SUBORDINATE_BUS		PCIR_SUBBUS_1
#endif

#if defined(__Linux__)
typedef struct pci_dev*		os_pci_dev_t;
typedef struct pci_bus*		os_pci_bus_t;
#elif defined(__FreeBSD__)
typedef device_t		os_pci_dev_t;
#endif

static __inline int
os_pci_read_config_byte(os_pci_dev_t dev, int reg, uint8_t *value)
{
#if defined(__Linux__)
	pci_read_config_byte(dev, reg, value);
#elif defined(__FreeBSD__)
	*value = pci_read_config(dev, reg, 1);
#else
	return -EINVAL;
#endif
	return 0;
} 

static __inline int
os_pci_write_config_byte(os_pci_dev_t dev, int reg, uint8_t value)
{
#if defined(__Linux__)
	pci_write_config_byte(dev, reg, value);
#elif defined(__FreeBSD__)
	pci_write_config(dev, reg, value, 1);
#else
	return -EINVAL;
#endif
	return 0;
} 

static __inline int
os_pci_read_config_dword(os_pci_dev_t dev, int reg, uint32_t *value)
{
#if defined(__Linux__)
	pci_read_config_dword(dev, reg, value);
#elif defined(__FreeBSD__)
	*value = pci_read_config(dev, reg, 4);
#else
	return -EINVAL;
#endif
	return 0;
} 

static __inline int
os_pci_write_config_dword(os_pci_dev_t dev, int reg, uint32_t value)
{
#if defined(__Linux__)
	pci_write_config_dword(dev, reg, value);
#elif defined(__FreeBSD__)
	pci_write_config(dev, reg, value, 4);
#else
	return -EINVAL;
#endif
	return 0;
} 

static __inline os_pci_dev_t
os_pci_get_device(u16 vendor, u16 device, os_pci_dev_t dev)
{
#if defined(__Linux__)
	unsigned int	vendor1 = vendor;
	unsigned int	device1 = device;
	if (!vendor) vendor1 = PCI_ANY_ID;
	if (!device) device1 = PCI_ANY_ID;
	return pci_get_device(vendor1, device1, dev);
#elif defined(__FreeBSD__)
	struct pci_devinfo	*dinfo;
	int			ready = 0;
	STAILQ_FOREACH(dinfo, &pci_devq, pci_links){
		if (dev == NULL) ready=1;
		else if (dev == dinfo->cfg.dev){
			ready = 1;
			continue;	
		}
		if (ready){
			if ((!vendor && !device) ||
			    (!vendor && dinfo->cfg.device == device) ||
			    (!device && dinfo->cfg.vendor == vendor) ||
			    (dinfo->cfg.vendor == vendor &&
					dinfo->cfg.device == device)){
				return (dinfo->cfg.dev);
			}
		}
	}
#endif
	return NULL;
} 

static __inline int os_pci_dev_vendor(os_pci_dev_t dev, u16 *vendor)
{
#if defined(__Linux__)
	*vendor = dev->vendor;
#elif defined(__FreeBSD__)
	*vendor = pci_get_vendor(dev);
#else
	return -EINVAL;
#endif
	return 0;
}

static __inline int os_pci_dev_device(os_pci_dev_t dev, u16 *device)
{
#if defined(__Linux__)
	*device = dev->device;
#elif defined(__FreeBSD__)
	*device = pci_get_device(dev);
#else
	return -EINVAL;
#endif
	return 0;
}

static __inline int os_pci_dev_slot(os_pci_dev_t dev, int *slot)
{
#if defined(__Linux__)
	*slot = (dev->devfn >> 3) & 0x1F;
#elif defined(__FreeBSD__)
	*slot = pci_get_slot(dev);
#else
	return -EINVAL;
#endif
	return 0;
}

static __inline int os_pci_dev_bus(os_pci_dev_t dev, int *bus)
{
#if defined(__Linux__)
	*bus = dev->bus->number;
#elif defined(__FreeBSD__)
	*bus = pci_get_bus(dev);
#else
	return -EINVAL;
#endif
	return 0;
}

static __inline int os_pci_dev_irq(os_pci_dev_t dev, int *irq)
{
#if defined(__Linux__)
	*irq = dev->irq;
#elif defined(__FreeBSD__)
	*irq = pci_get_irq(dev);
#else
	return -EINVAL;
#endif
	return 0;
}

static __inline int os_pci_dev_hdrtype(os_pci_dev_t dev, uint8_t *hdrtype)
{
#if defined(__Linux__)
	*hdrtype = dev->hdr_type;
#elif defined(__FreeBSD__)
	os_pci_read_config_byte(dev, OS_PCI_HEADER_TYPE, hdrtype);
#else
	return -EINVAL;
#endif
	return 0;
}

static __inline int os_pci_info_print(os_pci_dev_t dev, char *msg)
{
	unsigned short	v, d;
	int		s, b;

	os_pci_dev_vendor(dev, &v);
	os_pci_dev_device(dev, &d);
	os_pci_dev_slot(dev, &s);	
	os_pci_dev_bus(dev, &b);
	DEBUG_EVENT("* %s %04X:%04X:%02X:%02X\n",
			msg, v, d, b, s);
	return 0;
}


#endif	/* __OS_ABSTR_PCI_H */
