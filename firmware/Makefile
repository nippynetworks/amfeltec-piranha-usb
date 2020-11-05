# ============================================================
# Makefile	Amfeltec USB          
#
# 2013-2014, Amfeltec Corp.	
# ============================================================

PROD	= usb_fw_up
CC      = g++

# Build options
DIR	= $(shell pwd)
KNAME	= $(shell uname -s)

SRCS    = usb_firmware_up.c mem.cpp usb_firmware_update.cpp
OBJS    = usb_up.o

# SRC_H=$(AUSB_DIR)/include/amf_usb.h

#CFLAGS  = -Wall -O2 -D__$(KNAME)__
CFLAGS  = -Wall -g -D__$(KNAME)__

# CFLAGS  += -I$(SPCI_DIR)/include 
### CFLAGS	+= -lstdc++


### 	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
### LDFLAGS	=  -lm -lpthread

################ RULES ######################

usb_fw_up:	$(SRCS)	
	$(CC) $(CFLAGS) -o $@ $^

	@echo "Amfeltec firmware update utility is compiled!"


#all: 	amf_usb_update
#	@echo "$(PROD) is compiled!"



clean:
	rm -f $(PROD) $(OBJS)

