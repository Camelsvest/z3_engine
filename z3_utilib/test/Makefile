# Makefile for test

C++	= g++
CC 	= gcc
AR 	= ar
LD	= ld
MAKE	= make

PRJ_CFLAGS	+= ${CFLAGS}
PRJ_CFLAGS	+= -g -pthread -Wall -fno-inline -fstack-protector -fPIC -DZ3_MEM_DEBUG

PRJ_LDFLAGS	+= -g -pthread -fPIC

export PRJ_ROOT	  := ${shell pwd}

vpath %.h ${PRJ_ROOT}
vpath %.h ${PRJ_ROOT}/..

PRJ_INCLUDES	+= -I..

PRJ_LIBS	+= -L.. -lz3util

C_SRCS	:= ${shell find . -maxdepth 1 -name "*.c"}
C_OBJS	:= ${patsubst %.c, %.o, ${C_SRCS}}

TARGET		= test
DEPS		= Makefile.deps

.PHONY: clean install uninstall

${TARGET}: ${C_OBJS} ../libz3util.a
	$(CC) -o $@ ${C_OBJS} ${PRJ_LDFLAGS} ${PRJ_LIBS}

%.o: %.c ${DEPS}
	${CC} ${PRJ_CFLAGS} -o $@ -c $< ${PRJ_INCLUDES}

$(DEPS): $(C_SRCS)
	${CC} -MM ${PRJ_CFLAGS} $^ > $@ ${PRJ_INCLUDES}

-include $(DEPS)

../libz3util.a:
	make -C ../

clean:
	@rm ${TARGET}
	@rm ${DEPS}
	@rm ${C_OBJS}

install:
	@echo "Sorry, install command will be enable in the future.\n"

uninstall:
	@echo "Sorry, uninstall command will be enable in the future.\n"

