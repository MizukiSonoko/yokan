
CC=clang++
TARGET=yokan
OPTION=-std=c++0x -Wall

all: yokan.o
		$(CC) yokan.o -o $(TARGET)

yokan.o: yokan.cpp
		$(CC) $(OPTION) -c yokan.cpp

clean:
		rm yokan.o yokan