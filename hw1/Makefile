CXX = g++
CXXFLAGS = -std=c++17 -Wall
OBJECTS = hw1.o utils.o record.o wrapped.o
TARGET = hw1

.PHONY: all clean

all: ${OBJECTS}
	${CXX} ${CXXFLAGS} $^ -o ${TARGET}

%.o: %.cpp
	${CXX} ${CXXFLAGS} $^ -c

clean:
	-rm ${OBJECTS}
	-rm ${TARGET}
