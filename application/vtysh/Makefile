#  Name         : Makefile
#  Version      : V2.01
#  Author       : keith.gong (kizerg@163.com)
#  Time         : 2015.10.16
#  Description  : Makefile for FD1508GS vtysh
#                 Copyright (c) 2009-2015 C-Data Co.,Ltd.
#                 All Rights Reserved.
#  Modification : 2015.10.16, keith.gong, created

MODULE_DIR           ?= $(shell pwd)
MODULE_NAME          := $(notdir $(MODULE_DIR))
PRJ_ROOT_DIR         ?= $(shell cd ../..; pwd)

include $(PRJ_ROOT_DIR)/make/Make.config

EXTRACT_TOOL         := extract.pl
VTYSH_CMD_FILES      := $(wildcard $(GPON_DIR)/*/src/*Vty*.c) \
			$(SWAPP_DIR)/layer2/src/switchVtyMac.c  \
			$(SWAPP_DIR)/layer2/src/switchVtyPort.c  \
			$(SWAPP_DIR)/layer2/src/switchVtyVlan.c  \
			$(SWAPP_DIR)/rstp/src/switchVtyRstp.c   \
			$(SWAPP_DIR)/igmpsn/src/switchVtyIgmp.c \
			$(SWAPP_DIR)/dhcpsn/src/switchVtyDhcp.c \
			$(SWAPP_DIR)/lacp/src/lacpVty.c \
			$(SWAPP_DIR)/hal/src/switchVtyTrunk.c \
			$(SWAPP_DIR)/qosacl/src/switchVtyQosAcl.c\
			$(SYSTEM_DIR)/sysctrl/src/sysCtrlVty.c \
			$(wildcard $(SNMP_DIR)/src/*Vty*.c)


#  Create some dependency directory
MODULE_OBJ_DIR       := $(PRJ_OBJ_DIR)/$(MODULE_NAME)
MODULE_LIB_DIR       := $(PRJ_LIB_DIR)/$(MODULE_NAME)
$(shell $(MKDIR) $(MODULE_OBJ_DIR) $(MODULE_LIB_DIR))

INCLUDE_DIRS         := $(PRJ_ROOT_DIR)/include $(APP_DIR)/include $(COM_DIR)/include $(SYSTEM_DIR)/include  $(GPON_SUB_INC_DIRS) ./include #$(GPON_PONCFG_DIR)/include $(GPON_PROFILE_DIR)/include $(GPON_ONURA_DIR)/include ./include
INCLUDE_LIBS         := $(wildcard $(PRJ_LIB_DIR)/*.a) $(wildcard $(PRJ_LIB_DIR)/*/*.a)
CC_LIBS              := -lrt -lpthread -L$(MODULE_LIB_DIR) -l$(MODULE_NAME)

CFLAGS               += -Wall -DHAVE_CONFIG_H $(addprefix -I,$(INCLUDE_DIRS)) -D$(CDT_PRODUCT)
ifeq ($(CC),gcc)
CFLAGS               += -Wno-unused-but-set-variable
endif
LIBS_FLAGS           := -Wl,--start-group $(INCLUDE_LIBS) -Wl,--end-group

MODULE_APP_OBJS      := $(subst .c,.o, $(wildcard src/*.c))
ifneq (src/vtysh_cmd.c, $(wildcard src/vtysh_cmd.c))
MODULE_APP_OBJS      += src/vtysh_cmd.o
endif
MODULE_APP_DEPS      := $(MODULE_APP_OBJS:src%=$(MODULE_OBJ_DIR)%)
MODULE_LIB_OBJS      := $(subst .c,.o, $(wildcard lib/*.c))
MODULE_LIB_DEPS      := $(MODULE_LIB_OBJS:lib%=$(MODULE_OBJ_DIR)%)

#APP_DEP_LIBS         := $(PRJ_LIB_DIR)/common/libcommon.a $(PRJ_LIB_DIR)/sysctrl/libsysMacCrypt.a $(PRJ_LIB_DIR)/flash/libflash.a $(PRJ_LIB_DIR)/ipc/libipc_if.a
#MODULE_DEP_LIBS      := $(PRJ_LIB_DIR)/common/libcommon.a $(PRJ_LIB_DIR)/sysctrl/libsysMacCrypt.a $(PRJ_LIB_DIR)/flash/libflash.a

APP_DEP_LIBS         := $(PRJ_LIB_DIR)/common/libcommon.a  $(PRJ_LIB_DIR)/ipc/libipc_if.a
MODULE_DEP_LIBS      := $(PRJ_LIB_DIR)/common/libcommon.a 

#  Pattern matching rules:
$(MODULE_OBJ_DIR)/%.o:     $(MODULE_DIR)/src/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(MODULE_OBJ_DIR)/%.o:     $(MODULE_DIR)/lib/%.c
	$(CC) -fPIC -c $(CFLAGS) -o $@ $<

$(shell $(MKDIR) $(PRJ_LIB_DIR)/sysctrl; $(CP) $(SYSTEM_DIR)/libs/libMacCrypt.a $(PRJ_LIB_DIR)/sysctrl/libsysMacCrypt.a)

.PHONY: all lib clean extract
all: lib $(FW_ROOTFS_USR_BIN_DIR)/$(MODULE_NAME)

lib : $(MODULE_LIB_DIR)/lib$(MODULE_NAME).so $(MODULE_LIB_DIR)/lib$(MODULE_NAME).a

$(MODULE_LIB_DIR)/lib$(MODULE_NAME).so : $(MODULE_LIB_DEPS) $(MODULE_DEP_LIBS)
	$(CC) -shared -o $(MODULE_LIB_DIR)/lib$(MODULE_NAME).so $(MODULE_LIB_DEPS) $(MODULE_DEP_LIBS)

$(MODULE_LIB_DIR)/lib$(MODULE_NAME).a: $(MODULE_LIB_DEPS)
	$(AR) $(ARFLAGS) $(MODULE_LIB_DIR)/lib$(MODULE_NAME).a $(MODULE_LIB_DEPS)

extract : $(EXTRACT_TOOL) $(VTYSH_CMD_FILES)
	$(shell export CROSS_COMPILE=$(CROSS_COMPILE); export GPON_INC="$(addprefix -I, $(PRJ_ROOT_DIR)/include $(APP_DIR)/include $(COM_DIR)/include $(GPON_SUB_INC_DIRS) $(SNMP_DIR)/include)"; export SYSTEM_DIR=$(SYSTEM_DIR); ./$(EXTRACT_TOOL) $(VTYSH_CMD_FILES) > $(MODULE_DIR)/src/vtysh_cmd.c)
	$(CC) -c $(CFLAGS) -D$(CPU_ENDIAN) -o $(MODULE_OBJ_DIR)/vtysh_cmd.o $(MODULE_DIR)/src/vtysh_cmd.c


$(FW_ROOTFS_USR_BIN_DIR)/$(MODULE_NAME) : extract $(MODULE_APP_DEPS) $(MODULE_LIB_DIR)/lib$(MODULE_NAME).so
	$(CC) $(CFLAGS) -o $@ $(MODULE_APP_DEPS) -Wl,--start-group $(APP_DEP_LIBS) $(CC_LIBS) -Wl,--end-group
	$(STRIP) $@

clean:
	$(RM) $(MODULE_OBJ_DIR)/*
	$(RM) $(MODULE_LIB_DIR)/*

test:
	@$(ECHO) MODULE_LIB_DEPS = $(MODULE_LIB_DEPS)
	@$(ECHO) MODULE_APP_DEPS = $(MODULE_APP_DEPS)
	@$(ECHO) VTYSH_CMD_FILES = $(VTYSH_CMD_FILES)
	@$(ECHO) $(MODULE_DIR)/src/vtysh_cmd.c : $(VTYSH_CMD_FILES)
