#  Copyright (C)
#  文件名:Makefile
#  作者:
#  版本:1.0.0          
#  日期:
#  描述: 

#MODULE_DIR           ?= $(shell pwd)
#PRJ_ROOT_DIR         ?= $(shell cd ../..; pwd)

#include $(PRJ_ROOT_DIR)/make/Make.config

#CFLAGS+=-D_GNU_SOURCE -I$(PRJ_ROOT_DIR)/include -I../../application/include -I../../application/common/include -I$(ZLIB_DIR)/include
#all:pon_adaptor_net_if.o pon_adaptor_errno.o pon_adaptor_common_if.o libpon_adaptor.a 

#pon_adaptor_net_if.o:pon_adaptor_net_if.c
#	$(CC) -o pon_adaptor_net_if.o -c $(CFLAGS) pon_adaptor_net_if.c 
	
#pon_adaptor_errno.o:pon_adaptor_errno.c
#	$(CC) -o pon_adaptor_errno.o -c $(CFLAGS) pon_adaptor_errno.c 	
	
#pon_adaptor_common_if.o:pon_adaptor_common_if.c
#	$(CC) -o pon_adaptor_common_if.o -c $(CFLAGS) pon_adaptor_common_if.c
	
#libpon_adaptor.a:pon_adaptor_net_if.o pon_adaptor_errno.o pon_adaptor_common_if.o
#	$(AR) $(ARFLAGS) libpon_adaptor.a pon_adaptor_net_if.o pon_adaptor_errno.o pon_adaptor_common_if.o
#	$(MKDIR) $(PRJ_LIB_DIR)/ponadaptor
#	$(CP) libpon_adaptor.a $(PRJ_LIB_DIR)/ponadaptor/
	
#clean:
#	-rm -f *.o libpon_adaptor.a

#持续集成
MODULE_DIR           ?= $(shell pwd)
#PRJ_ROOT_DIR         ?= $(shell cd ../../../..; pwd)

#include $(PRJ_ROOT_DIR)/make/Make.config

MODULE_OBJS          := $(subst .c,.o,$(wildcard src/*.c)) 
MODULE_DEPS          := $(MODULE_OBJS:src%=$(MODULE_OBJ_DIR)%)

INCLUDE_DIRS         := $(MODULE_DIR)/include \
						$(MODULE_DIR)/sqlite/include \
						$(MODULE_DIR)/libev/include \
						$(MODULE_DIR)/zlib/include


APP_DEP_LIBS         :=	-L$(MODULE_DIR)/libev/install/lib \
						-L$(MODULE_DIR)/zlib/install/lib \
						-L$(MODULE_DIR)/ponadp

CFLAGS               += -Wall -Werror -g $(addprefix -I,$(INCLUDE_DIRS)) -DHAVE_CONFIG_H
LIBS_FLAGS           := -Wl,--start-group $(APP_DEP_LIBS) -Wl,--end-group
CC_LIBS              := -lm -lrt -lpthread -L$(MODULE_DIR)/zlib/install/lib -lz -L$(MODULE_DIR)/libev/install/lib -lev -lpon_adaptor

$(MODULE_OBJ_DIR)/%.o: pon_drv_init.c pon_drv_db.c  pon_drv_net.c pon_thread_pool.c
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: all lib clean 
all: $(FW_ROOTFS_USR_BIN_DIR)/pondrvt

clean: 
	$(RM) $(MODULE_OBJ_DIR)/*
	$(RM) $(MODULE_LIB_DIR)/*
