# Amfeltec Corp.
#  
# Makefile for Amfeltec USB driver
#

EXTRA_CFLAGS=$(EXTRA_FLAGS) -DOS_KERNEL 

amf_usb-objs := amf_main.o 
amf_usb-objs += amf_usb_driver.o 
amf_usb-objs += amf_usb_core.o 
amf_usb-objs += amf_usb_cpu.o 
amf_usb-objs += amf_usb_dahdi.o 
amf_usb-objs += amf_usb_cp210x.o 
amf_usb-objs += amf_usb_daa.o
amf_usb-objs += amf_usb_proslic.o
amf_usb-objs += amf_usb_cpu_regs.o  

obj-m += amf_usb.o 


