ASM = yasm
CC  = gcc

ASMFLAGS = -f elf64 -DYASM -D__x86_64__ -DPIC 
CFLAGS   = -c -g -Wall -fno-stack-protector -fPIC -nostdlib

TARGET = libmini.so libmini.a
OBJECT = libmini64.o libmini.o

.PHONY: clean all

all: ${TARGET}

libmini.so: ${OBJECT}
	ld -shared -o $@ $^

libmini.a: ${OBJECT}
	ar rv $@ $^

libmini64.o: libmini64.asm
	${ASM} ${ASMFLAGS} $^ -o $@

libmini.o: libmini.c
	${CC} ${CFLAGS} $^

clean:
	-rm ${TARGET}
	-rm ${OBJECT}
