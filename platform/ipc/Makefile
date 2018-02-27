
#  Copyright (C), 2014-2014, C-Data Tech. Co., Ltd.
#  文件名:Makefile
#  作者:jh.liang
#  版本:1.0.0          
#  日期:2014/12
#  描述: 此文件定义IPC模块主程序

PRJ_ROOT_DIR         ?= $(shell cd ../..; pwd)

-include $(PRJ_ROOT_DIR)/make/Make.config

CFLAGS+=-D_GNU_SOURCE -I$(shell pwd)/../include -I$(PRJ_ROOT_DIR)/include -I$(COM_DIR)/include
all:ipc_ker libipc_if.a

ipc_ker:ipc_ker.o
	$(CC) -o ipc_ker $(CFLAGS) ipc_ker.c 
	
libipc_if.a:ipc_if.o
	$(AR) $(ARFLAGS) libipc_if.a ipc_if.o #$(PRJ_ROOT_DIR)/build/objs/sysMacCrypt/cdtMacCrypt.o $(PRJ_ROOT_DIR)/build/objs/flash/cdtNvramParam.o
	$(MKDIR) $(PRJ_LIB_DIR)/ipc
	$(CP) libipc_if.a $(PRJ_LIB_DIR)/ipc/
	$(CP) ipc_ker $(PRJ_LIB_DIR)/ipc/
clean:
	-rm -f *.o ipc_ker libipc_if.a
