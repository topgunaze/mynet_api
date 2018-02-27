#  Name         : Makefile
#  Version      : V2.01
#  Author       : keith.gong (kizerg@163.com)
#  Time         : 2016.12.19
#  Description  : Makefile for SQLite
#                 Copyright (c) 2009-2016 CDATA Co.,Ltd.
#                 All Rights Reserved.
#  Modification : 2016.12.19, keith.gong, created
#

MODULE_DIR           := $(shell pwd)
MODULE_NAME          := $(notdir $(MODULE_DIR))
PRJ_ROOT_DIR         ?= $(shell cd ../../; pwd)

#TODO
LIBEV_SRC_DIR       := ./libev37

-include $(PRJ_ROOT_DIR)/make/Make.config
#CROSS_COMPILE        := 
CC                   := $(CROSS_COMPILE)gcc
AR                   := $(CROSS_COMPILE)ar
STRIP                ?= strip

#  Create some dependency directory 
MODULE_OBJ_DIR       ?= $(MODULE_DIR)/install
MODULE_OBJ_DIR       ?= $(PRJ_ROOT_DIR)/build/objs/$(MODULE_NAME)
MODULE_LIB_DIR       ?= $(PRJ_ROOT_DIR)/build/libs/$(MODULE_NAME)
MODULE_INC_DIR       ?= $(MODULE_DIR)/include

MKDIR                ?= mkdir -p 
$(shell $(MKDIR) $(MODULE_OBJ_DIR) $(MODULE_LIB_DIR) $(MODULE_INC_DIR))

INCLUDE_DIRS         := 
MODULE_CFLAGS        := 
CFLAGS               += 
CC_LIBS              := 

APP_DEP_LIBS         := 

MODULE_APP_OBJS      := 
MODULE_APP_DEPS      := 
MODULE_LIB_OBJS      := 
MODULE_LIB_DEPS      := 

#  Pattern matching rules:
$(MODULE_OBJ_DIR)/%.o:     $(MODULE_DIR)/src/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(MODULE_OBJ_DIR)/%.o:     $(MODULE_DIR)/lib/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

JOBS := $(shell grep -c ^processor /proc/cpuinfo 2>/dev/null)
ifeq ($(JOBS),)
JOBS := 1
endif

#CONFIG_CMD       := ./configure \
#	--build=i386-linux \
#	--host=arm-linux \
#	--target=arm-linux \
#	CC=$(CROSS_COMPILE)gcc \
#	AR=$(CROSS_COMPILE)ar \
#	RANLIB=$(CROSS_COMPILE)ranlib \
#	STRIP=$(CROSS_COMPILE)strip \
#	--prefix="$(MODULE_OBJ_DIR)" \
#	--includedir="$(MODULE_INC_DIR)"

CONFIG_CMD       := ./configure \
	--build=i386-linux \
	CC=$(CROSS_COMPILE)gcc \
	AR=$(CROSS_COMPILE)ar \
	RANLIB=$(CROSS_COMPILE)ranlib \
	STRIP=$(CROSS_COMPILE)strip \
	--prefix="$(MODULE_OBJ_DIR)" \
	--includedir="$(MODULE_INC_DIR)"
	
.PHONY: all lib clean config
all: lib

ifneq ($(LIBEV_SRC_DIR)/Makefile,$(wildcard $(LIBEV_SRC_DIR)/Makefile))
$(LIBEV_SRC_DIR)/Makefile: config
endif

lib: $(LIBEV_SRC_DIR)/Makefile
	make -j$(JOBS) -C $(LIBEV_SRC_DIR) 
	make -j$(JOBS) install -C $(LIBEV_SRC_DIR) 

config:
	cd $(LIBEV_SRC_DIR);$(CONFIG_CMD)

clean:
ifeq ($(LIBEV_SRC_DIR)/Makefile, $(wildcard $(LIBEV_SRC_DIR)/Makefile))
	make -j$(JOBS) clean -C $(LIBEV_SRC_DIR) 
endif
	$(RM) $(MODULE_OBJ_DIR)/*
	$(RM) $(MODULE_LIB_DIR)/*

test:
	@$(ECHO) $(CONFIG_CMD)

