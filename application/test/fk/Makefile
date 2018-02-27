#  Name         : 
#  Version      :
#  Author       : 
#  Time         : 
#  Description  : 
#  Modification : 

MODULE_DIR           ?= $(shell pwd)
PRJ_ROOT_DIR         ?= $(shell cd ../../../; pwd)

-include $(PRJ_ROOT_DIR)/make/Make.config

#  Create some dependency directory 
MODULE_OBJ_DIR       := $(PRJ_OBJ_DIR)/fk
MODULE_LIB_DIR       := $(PRJ_LIB_DIR)/fk
$(shell $(MKDIR) $(MODULE_OBJ_DIR) $(MODULE_LIB_DIR))

MODULE_OBJS          := $(subst .c,.o,$(wildcard src/*.c))
MODULE_DEPS := $(MODULE_OBJS:src%=$(MODULE_OBJ_DIR)%)

INCLUDE_DIRS         := $(APP_INC_DIR) \
						$(MODULE_DIR)/include \
						$(COM_DIR)/include \
						$(PLA_INC_DIR) \
						$(SQLITE_DIR)/include \
						$(LIBEV_DIR)/include \
						$(ZLIB_DIR)/include

LIBS_PATH         :=	$(SQLITE_DIR)/install/lib \
						$(LIBEV_DIR)/install/lib \
						$(ZLIB_DIR)/install/lib \
						$(TF_ADP_DIR)

CC_LIBS              := -lm -lrt -lpthread -lsqlite3 -lev -lz -ltfadp

CFLAGS               += -Wall -Werror -g $(addprefix -I,$(INCLUDE_DIRS)) -DHAVE_CONFIG_H
LIBS_FLAGS           := -Wl,--start-group $(addprefix -L,$(LIBS_PATH)) $(CC_LIBS) -Wl,--end-group

#  Pattern matching rules: 
$(MODULE_OBJ_DIR)/%.o: $(MODULE_DIR)/src/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: all lib clean 
all: $(BIN_DIR)/fk

$(BIN_DIR)/fk : $(MODULE_DEPS)
	$(CC) $(CFLAGS) -DSTT_UNDERLYING_OS=U_LINUX -o $@ $^ $(LIBS_FLAGS) 
	$(CROSS_COMPILE)objdump -S $@ > gpon_dump.txt
	$(STRIP) $@

clean: 
	$(RM) $(MODULE_OBJ_DIR)/*
	$(RM) $(MODULE_LIB_DIR)/*
	$(RM) $(BIN_DIR)/fk
	
test:
	@$(ECHO) $(MODULE_OBJS)
	@$(ECHO) $(MODULE_OBJ_DIR) $(MODULE_DIR)/src/
	@$(ECHO) $(CC) -c $(CFLAGS) -o $(MODULE_OBJ_DIR) $(MODULE_DIR)/src/
	