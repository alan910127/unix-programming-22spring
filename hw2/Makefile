CC = gcc
CFLAGS = -Wall
SOFLAGS = ${CFLAGS} -ldl -shared
OBJFLAGS = ${CFLAGS} -ldl -fPIC

LIBOBJ = permission.o standards.o syscalls.o helper.o
TARGET = logger.so hw2

.PHONY: all clean

all: ${TARGET}
	chmod +x ./logger

logger.so: ${LIBOBJ}
	${CC} $^ ${SOFLAGS} -o $@
hw2: hw2.c
	${CC} $^ ${CFLAGS} -o $@

%.o: %.c
	${CC} $^ ${OBJFLAGS} -c -o $@

clean:
	-rm ${LIBOBJ}
	-rm ${TARGET}
