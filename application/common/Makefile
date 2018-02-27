#  Name         : Makefile
#  Version      : V2.00
#  Author       : keith.gong (kizerg@163.com)
#  Time         : 2015.01.23
#  Description  : Makefile for FD1508GS Common
#                 Copyright (c) 2009-2015 C-Data Co.,Ltd.
#                 All Rights Reserved.
#  Modification : 2015.01.23, keith.gong, created

COMMON_DIR           ?= $(shell pwd)
PRJ_ROOT_DIR         ?= $(shell cd ../..; pwd)

include $(PRJ_ROOT_DIR)/make/Make.config

#  Create some dependency directory 
COMMON_OBJ_DIR       := $(PRJ_OBJ_DIR)/common
COMMON_LIB_DIR       := $(PRJ_LIB_DIR)/common
$(shell $(MKDIR) $(COMMON_OBJ_DIR) $(COMMON_LIB_DIR)) 

COMMON_OBJS          := $(subst .c,.o,$(wildcard src/*.c))
COMMON_DEPS          := $(COMMON_OBJS:src%=$(COMMON_OBJ_DIR)%)

INCLUDE_DIRS         := $(PRJ_ROOT_DIR)/include ./include $(APP_DIR)/include
INCLUDE_LIBS         := 

CFLAGS               += $(addprefix -I,$(INCLUDE_DIRS)) -D$(CDT_PRODUCT)
LIBS_FLAGS           := -Wl,--start-group $(INCLUDE_LIBS) -Wl,--end-group

#  Pattern matching rules: 
$(COMMON_OBJ_DIR)/%.o:     $(COMMON_DIR)/src/%.c
	$(CC) -fPIC -c $(CFLAGS) -o $@ $<

all: $(COMMON_LIB_DIR)/libcommon.a

$(COMMON_LIB_DIR)/libcommon.a: $(COMMON_DEPS)
	$(AR) $(ARFLAGS) $(COMMON_LIB_DIR)/libcommon.a $(COMMON_DEPS)

clean: 
	$(RM) $(COMMON_OBJ_DIR)/*
	$(RM) $(COMMON_LIB_DIR)/*

test:
