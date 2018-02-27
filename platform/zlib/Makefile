#  Name         : Makefile
#  Version      : 
#  Author       : 
#  Time         : 
#  Description  : 
#                 Copyright (c) 2009-2016 CDATA Co.,Ltd.
#                 All Rights Reserved.
#  Modification : 
#

MODULE_DIR           := $(shell pwd)
MODULE_NAME          := $(notdir $(MODULE_DIR))
PRJ_ROOT_DIR         ?= $(shell cd ../../; pwd)

#TODO
ZLIB_SRC_DIR       := ./zlib1211

-include $(PRJ_ROOT_DIR)/make/Make.config
#CROSS_COMPILE        := 
CROSS_COMPILE        := 
CC                   := $(CROSS_COMPILE)gcc
AR                   := $(CROSS_COMPILE)ar
STRIP                ?= strip

#  Create some dependency directory 
MODULE_OBJ_DIR       ?= $(MODULE_DIR)/install
#MODULE_OBJ_DIR       ?= $(PRJ_ROOT_DIR)/build/objs/$(MODULE_NAME)
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

CONFIG_CMD       := ./configure \
	--prefix="$(MODULE_OBJ_DIR)" \
	--includedir="$(MODULE_INC_DIR)"

.PHONY: config all lib clean 
all: config lib

ifneq ($(ZLIB_SRC_DIR)/Makefile,$(wildcard $(ZLIB_SRC_DIR)/Makefile))
$(ZLIB_SRC_DIR)/Makefile: config
endif

config:
	cd $(ZLIB_SRC_DIR);$(CONFIG_CMD)

lib: $(ZLIB_SRC_DIR)/Makefile
	make -j$(JOBS) -C $(ZLIB_SRC_DIR) 
	make -j$(JOBS) install -C $(ZLIB_SRC_DIR) 

clean:
ifeq ($(ZLIB_SRC_DIR)/Makefile, $(wildcard $(ZLIB_SRC_DIR)/Makefile))
	make -j$(JOBS) clean -C $(ZLIB_SRC_DIR) 
endif
	$(RM) $(MODULE_OBJ_DIR)/*
	$(RM) $(MODULE_LIB_DIR)/*

test:
	@$(ECHO) $(CONFIG_CMD)

