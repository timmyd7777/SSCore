SOURCEDIR=../../SSCode

SOURCES=../SSTest.cpp \
$(SOURCEDIR)/SSAngle.cpp \
$(SOURCEDIR)/SSConstellation.cpp \
$(SOURCEDIR)/SSCoordinates.cpp \
$(SOURCEDIR)/SSEvent.cpp \
$(SOURCEDIR)/SSIdentifier.cpp \
$(SOURCEDIR)/SSImportMPC.cpp \
$(SOURCEDIR)/SSJPLDEphemeris.cpp \
$(SOURCEDIR)/SSMatrix.cpp \
$(SOURCEDIR)/SSMoonEphemeris.cpp \
$(SOURCEDIR)/SSObject.cpp \
$(SOURCEDIR)/SSOrbit.cpp \
$(SOURCEDIR)/SSPlanet.cpp \
$(SOURCEDIR)/SSPSEphemeris.cpp \
$(SOURCEDIR)/SSStar.cpp \
$(SOURCEDIR)/SSTime.cpp \
$(SOURCEDIR)/SSTLE.cpp \
$(SOURCEDIR)/SSUtilities.cpp \
$(SOURCEDIR)/SSVector.cpp \

HEADERS=\
$(SOURCEDIR)/SSAngle.hpp \
$(SOURCEDIR)/SSConstellation.cpp \
$(SOURCEDIR)/SSCoordinates.hpp \
$(SOURCEDIR)/SSEvent.hpp \
$(SOURCEDIR)/SSIdentifier.hpp \
$(SOURCEDIR)/SSImportMPC.hpp \
$(SOURCEDIR)/SSJPLDEphemeris.hpp \
$(SOURCEDIR)/SSMatrix.hpp \
$(SOURCEDIR)/SSMoonEphemeris.hpp \
$(SOURCEDIR)/SSObject.hpp \
$(SOURCEDIR)/SSOrbit.hpp \
$(SOURCEDIR)/SSPlanet.hpp \
$(SOURCEDIR)/SSPSEphemeris.hpp \
$(SOURCEDIR)/SSStar.hpp \
$(SOURCEDIR)/SSTime.hpp \
$(SOURCEDIR)/SSTLE.hpp \
$(SOURCEDIR)/SSUtilities.hpp \
$(SOURCEDIR)/SSVector.hpp \

CC=gcc
CFLAGS=-std=c++11 -I$(SOURCEDIR)
LDFLAGS=-lstdc++

EXECUTABLE=sstest

all:	test

test:	$(SOURCES) $(HEADERS)
	$(CC) -o $(EXECUTABLE) $(CFLAGS) $(LDFLAGS) $(SOURCES) 

clean:
	/bin/rm -f *.o $(EXECUTABLE)
