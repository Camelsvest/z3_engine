# 
# Makefile for library z3_util
#

export CROSS_COMPILE?= arm-hisiv100nptl-linux-

C++	= $(CROSS_COMPILE)g++
CC 	= $(CROSS_COMPILE)gcc
AR 	= $(CROSS_COMPILE)ar
LD	= $(CROSS_COMPILE)ld
MAKE	= make
RANLIB  = $(CROSS_COMPILE)ranlib

PRJ_CFLAGS	+= ${CFLAGS}
PRJ_CFLAGS	+= -g -pthread -Wall -fno-inline -fstack-protector -fPIC -DZ3_MEM_DEBUG

PRJ_LDFLAGS	+= -g -pthread -fPIC

PRJ_INCLUDES	+= -I..

export PRJ_ROOT	  := ${shell pwd}

vpath %.h ${PRJ_ROOT}

C_SRCS	:= ${shell find ./ -maxdepth 1 -name "*.c"}
C_OBJS	:= ${patsubst %.c, %.o, ${C_SRCS}}

TARGET	:= libz3util.a
DEPS	:= Makefile.deps

.PHONY: ${CLEAN_DIRS} clean install uninstall

${TARGET}: ${C_OBJS} ${DEPS}
	$(AR) -rs $@ ${C_OBJS}

.c.o:
	${CC} ${PRJ_CFLAGS} -o $@ -c $< ${PRJ_INCLUDES}

${DEPS}: ${C_SRCS}
	${CC} -MM ${PRJ_CFLAGS} $^ > $@ ${PRJ_INCLUDES}

-include ${DEPS}

clean:
	@rm ${TARGET}
	@rm ${DEPS}
	@rm ${C_OBJS}

install:
	test -d $(INSTALL_DIR)/include/z3_util || mkdir -p $(INSTALL_DIR)/include/z3_util
	cp -f ./z3_alloc.h $(INSTALL_DIR)/include/z3_util
	cp -f ./z3_trace.h $(INSTALL_DIR)/include/z3_util
	cp -f ./z3_list.h $(INSTALL_DIR)/include/z3_util

	test -d $(INSTALL_DIR)/lib || mkdir -p $(INSTALL_DIR)/lib
	cp -f $(TARGET) $(INSTALL_DIR)/lib

uninstall:
	rm -rf $(INSTALL_DIR)/include/z3_util
	rm -rf $(INSTALL_DIR)/lib/$(TARGET)
