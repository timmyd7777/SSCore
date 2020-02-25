CC=gcc
CFLAGS=-std=c++11
LDFLAGS=-lstdc++
SOURCES=SSTime.cpp SSAngle.cpp SSVector.cpp SSMatrix.cpp test.cpp
HEADERS=SSTime.hpp SSAngle.hpp SSVector.hpp SSMatrix.cpp

all:	test

test:	$(SOURCES) $(HEADERS)
	$(CC) -o test $(CFLAGS) $(LDFLAGS) $(SOURCES) 

clean:
	/bin/rm -f *.o test
