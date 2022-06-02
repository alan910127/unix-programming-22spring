CXX = g++
LIBNAME = capstone
CPPFLAGS = -O3 -Wall -std=c++17 -l${LIBNAME}
OBJECTS = utilities.o debugger.o hw4.o disassemble.o
TARGET = hw4

.PHONY: all clean

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CXX} ${OBJECTS} ${CPPFLAGS} -o ${TARGET}

%.o: %.cpp
	${CXX} $< ${CPPFLAGS} -c -o $@

clean: 
	-rm ${OBJECTS}
	-rm ${TARGET}