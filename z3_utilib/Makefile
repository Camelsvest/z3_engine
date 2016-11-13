# 
# Makefile for library z3_util
#

C++	= g++
CC 	= gcc
AR 	= ar
LD	= ld
MAKE	= make

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
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"
