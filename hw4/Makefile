CXX = g++
CPPFLAGS = -Wall -std=c++17
OBJECTS = utilities.o debugger.o hw4.o
TARGET = hw4

.PHONY: all clean

all: ${TARGET}

${TARGET}: ${OBJECTS}
	${CXX} ${OBJECTS} ${CPPFLAGS} -o ${TARGET}

%.o: %.cpp
	${CXX} $^ ${CPPFLAGS} -c 

clean: 
	-rm ${OBJECTS}
	-rm ${TARGET}