# Makefile File
# for Amfeltec Device Driver

include Makefile.in

DEBUG=n
DEBUG_USB=n
DEBUG_TX=n
DEBUG_RX=n
TEST_MODE=n
PWD=$(shell pwd)
RM      = @rm -rf
JUNK	= *~ *.bak DEADJOE

KVER	?= $(shell uname -r)
KNAME	?= $(shell uname -s)
KDIR	=/lib/modules/$(shell uname -r)/build
KVERBOSE=0
MODTYPE=ko

VERBOSE		=0
AUSB_DIR	=$(PWD)
SRC_DIR		=$(PWD)/firmware
MOD_DIR		=$(PWD)/driver
INCLUDE_DIR =$(PWD)/include 
OS_DIR		=$(PWD)/include/os

TARGETS=compile_amf_mod compile_amf_firmware
CLEANTARGETS=clean_amf_mod clean_amf_firmware  
INSTALLTARGETS=install_amf_mod

EXTRA_CFLAGS=$(GLOBAL_CFLAGS) -I$(KDIR)/include/linux -I$(INCLUDE_DIR) -I$(SRC_DIR) -I$(OS_DIR) -I$(DAHDI_DIR)/include/

DEBFLAGS = -O
ifeq ($(DEBUG),y)
	DEBFLAGS += -g -DAMF_DEBUG 
	ifeq ($(DEBUG_USB),y)
		DEBFLAGS += -DAMF_DEBUG_USB 
	endif
	ifeq ($(DEBUG_TX),y)
		DEBFLAGS += -DAMF_DEBUG_TX 
	endif
	ifeq ($(DEBUG_RX),y)
		DEBFLAGS += -DAMF_DEBUG_RX 
	endif
endif

EXTRA_CFLAGS += $(DEBFLAGS)
EXTRA_CFLAGS += -w

TESTFLAGS = -O
ifeq ($(TEST_MODE),y)
	TESTFLAGS += -g -DAMF_TEST_MODE
endif 
EXTRA_CFLAGS += $(TESTFLAGS)


EXTRA_UTIL_FLAGS=$(GLOBAL_CFLAGS)

all: check-dahdi check-kernel $(TARGETS)

compile_amf_mod:
	$(MAKE) KBUILD_VERBOSE=$(KVERBOSE) -C $(KDIR) SUBDIRS=$(MOD_DIR) EXTRA_FLAGS="$(EXTRA_CFLAGS)" modules

compile_amf_firmware:
	$(MAKE) -C $(SRC_DIR) EXTRA_FLAGS="EXTRA_UTIL_FLAGS"  

clean_amf_mod:
	$(MAKE) -C $(KDIR) SUBDIRS=$(MOD_DIR) clean

clean_amf_firmware:
	$(MAKE) -C $(SRC_DIR) clean

clean: 	$(CLEANTARGETS)
	@find . -name '*symvers*' | xargs rm -f

install: $(INSTALLTARGETS)
	$(shell depmod -a)

install_amf_mod:
	$(MAKE) -C $(KDIR) SUBDIRS=$(MOD_DIR) modules_install

#Check for linux headers
check-kernel: 
	@if [ ! -e $(KDIR) ]; then \
		echo "   Error linux headers/source not found: $(KDIR) !"; \
		echo ; \
		exit 1; \
	fi 
	@if [ ! -e $(KDIR)/.config ]; then \
		echo "   Error linux headers/source not configured: missing $(KDIR)/.config !"; \
		echo ; \
		exit 1; \
	fi 
	@if [ ! -e $(KDIR)/include ]; then \
		echo "   Error linux headers/source incomplete: missing $(KDIR)/include dir !"; \
		echo ; \
		exit 1; \
	fi

#Check for dahdi
check-dahdi:

	@echo
	@echo " +----------------- Checking Dahdi Sources -----------------+"  
	@echo 
	@if [ -e $(DAHDI_DIR)/linux/include/dahdi/kernel.h ]; then \
		echo "   Compiling with OLD DAHDI Support!"; \
		echo "   Dahdi Dir: $(DAHDI_DIR)"; \
		echo; \
	elif [ -e $(DAHDI_DIR)/include/dahdi/kernel.h ]; then \
		echo "   Compiling with NEW DAHDI Support!"; \
		echo "   Dahdi Dir: $(DAHDI_DIR)"; \
		echo; \
		if [ -f $(DAHDI_DIR)/drivers/dahdi/Module.symvers ]; then \
		  	cp -f $(DAHDI_DIR)/drivers/dahdi/Module.symvers $(MOD_DIR)/; \
		elif [ -f $(DAHDI_DIR)/src/dahdi-headers/drivers/dahdi/Module.symvers ]; then \
		  	cp -f $(DAHDI_DIR)/src/dahdi-headers/drivers/dahdi/Module.symvers $(MOD_DIR)/; \
		else \
			echo "Error: Dahdi source not compiled, missing Module.symvers file"; \
			echo "	     Please recompile Dahdi directory first"; \
			exit 1; \
		fi; \
	else \
		echo "Error: Dahdi source not found"; \
		echo "       Please check Dahdi Dir ($(DAHDI_DIR)) "; \
		echo ; \
		exit 1; \
	fi; 
	@echo 
	@echo " +----------------------------------------------------------+" 
	@echo 
	@sleep 1;              

boot:
	@./mkboot.sh


