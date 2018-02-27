#  Name         : Makefile
#  Version      : V2.00
#  Author       : steven.tian
#  Time         : 2015.05.18
#  Description  : Makefile for FD1508GS soc driver user space interface
#                 Copyright (c) 2009-2015 C-Data Co.,Ltd.
#                 All Rights Reserved.
#  Modification : 2015.05.18, steven.tian, created

MONITOR_DIR          ?= $(shell pwd)
PRJ_ROOT_DIR         ?= $(shell cd ../..; pwd)

#  Include GPON project configure file
include $(PRJ_ROOT_DIR)/make/Make.config

include $(PRJ_ROOT_DIR)/make/Make.monitor-config

#  Create some depend directory 
$(shell $(MKDIR) $(MONITOR_OBJ_DIR))

#CC_COMPILER         += -ansi -pedantic
### if -pipe not is supported on this host
#CC_COMPILER         += -ansi -pedantic -pipe -nostdinc
CC_FLAGS             += -g -O2 -Wall -D__LINUX__
CFLAGS               += $(MONITOR_INCLUDE) $(CC_COMPILER) $(CC_FLAGS)
CC_LIBS              := -lpthread -lrt -lm

#  Pattern matching rules: 
$(MONITOR_OBJ_DIR)/%.o:     $(MONITOR_DIR)/src/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: all clean 
all: $(FW_ROOTFS_USR_BIN_DIR)/monitor

$(FW_ROOTFS_USR_BIN_DIR)/monitor : $(MONITOR_DEPS_OBJS) $(APP_DEP_LIBS)
	$(CC) $(CFLAGS) -D__MAKE_MONITOR_APP__ -DSTT_UNDERLYING_OS=U_LINUX -o $(FW_ROOTFS_USR_BIN_DIR)/monitor $(MONITOR_DEPS_OBJS) -Wl,--start-group $(APP_DEP_LIBS) -Wl,--end-group $(CC_LIBS)
	$(STRIP) $(FW_ROOTFS_USR_BIN_DIR)/monitor

clean:
	$(RM) $(MONITOR_DEPS_OBJS) $(FW_ROOTFS_USR_BIN_DIR)/monitor

test:
	@$(ECHO) MONITOR_LIB_DIR=$(MONITOR_LIB_DIR)
	@$(ECHO) $(FW_ROOTFS_USR_BIN_DIR)/monitor : $(APP_DEP_LIBS)
	
