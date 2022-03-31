CC = gcc
CFLAGS = -Wall -Wno-unknown-pragmas -ldl
SOFLAGS = ${CFLAGS} -shared
OBJFLAGS = ${CFLAGS} -fPIC

SOURCE = hw2.c
OBJECT = $(patsubst %.c, %.o, $(SOURCE))
TARGET = logger.so

.PHONY: all clean

${TARGET}: ${OBJECT}
	${CC} $^ ${SOFLAGS} -o $@

%.o: %.c
	${CC} $^ ${OBJFLAGS} -c -o $@

clean:
	-rm ${OBJECT}
	-rm ${TARGET}
