
#  Copyright (C), 2014-2014, C-Data Tech. Co., Ltd.
#  文件名:Makefile
#  作者:jh.liang
#  版本:1.0.0          
#  日期:2014/12
#  描述: 此文件定义IPC模块主程序

SUBDIRS  := vtysh common ipc switch sysctrl monitor
.PHONY : subdir
subdir :
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir || exit "$$?";done
clean:
	@for dir in $(SUBDIRS); do $(MAKE) clean -C $$dir || exit 1;done
