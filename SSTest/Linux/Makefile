SOURCEDIR=../../SSCode

# All SSCode source files needed to compile executable

SSCORE_SOURCES=\
$(SOURCEDIR)/SSAngle.cpp \
$(SOURCEDIR)/SSConstellation.cpp \
$(SOURCEDIR)/SSCoordinates.cpp \
$(SOURCEDIR)/SSEvent.cpp \
$(SOURCEDIR)/SSFeature.cpp \
$(SOURCEDIR)/SSHTM.cpp \
$(SOURCEDIR)/SSIdentifier.cpp \
$(SOURCEDIR)/SSImportGCVS.cpp \
$(SOURCEDIR)/SSImportGJ.cpp \
$(SOURCEDIR)/SSImportHIP.cpp \
$(SOURCEDIR)/SSImportMPC.cpp \
$(SOURCEDIR)/SSImportSKY2000.cpp \
$(SOURCEDIR)/SSImportTLE.cpp \
$(SOURCEDIR)/SSImportTYC.cpp \
$(SOURCEDIR)/SSImportWDS.cpp \
$(SOURCEDIR)/SSJPLDEphemeris.cpp \
$(SOURCEDIR)/SSMatrix.cpp \
$(SOURCEDIR)/SSMoonEphemeris.cpp \
$(SOURCEDIR)/SSMount.cpp \
$(SOURCEDIR)/SSMountModel.cpp \
$(SOURCEDIR)/SSObject.cpp \
$(SOURCEDIR)/SSOrbit.cpp \
$(SOURCEDIR)/SSPlanet.cpp \
$(SOURCEDIR)/SSPSEphemeris.cpp \
$(SOURCEDIR)/SSSerial.cpp \
$(SOURCEDIR)/SSSocket.cpp \
$(SOURCEDIR)/SSStar.cpp \
$(SOURCEDIR)/SSTime.cpp \
$(SOURCEDIR)/SSTLE.cpp \
$(SOURCEDIR)/SSUtilities.cpp \
$(SOURCEDIR)/SSVector.cpp \
$(SOURCEDIR)/SSView.cpp \
$(SOURCEDIR)/VSOP2013/ELPMPP02.cpp \
$(SOURCEDIR)/VSOP2013/VSOP2013.cpp \
$(SOURCEDIR)/VSOP2013/VSOP2013p1.cpp \
$(SOURCEDIR)/VSOP2013/VSOP2013p2.cpp \
$(SOURCEDIR)/VSOP2013/VSOP2013p3.cpp \
$(SOURCEDIR)/VSOP2013/VSOP2013p4.cpp \
$(SOURCEDIR)/VSOP2013/VSOP2013p5.cpp \
$(SOURCEDIR)/VSOP2013/VSOP2013p6.cpp \
$(SOURCEDIR)/VSOP2013/VSOP2013p7.cpp \
$(SOURCEDIR)/VSOP2013/VSOP2013p8.cpp \
$(SOURCEDIR)/VSOP2013/VSOP2013p9.cpp \
$(SOURCEDIR)/Tetra3/Tetra3.cpp \
$(SOURCEDIR)/Tetra3/cnpy.cpp \
$(SOURCEDIR)/Tetra3/svdcmp.c \

# All SSCore headers needed to compile executable

SSCORE_HEADERS=\
$(SOURCEDIR)/SSAngle.hpp \
$(SOURCEDIR)/SSConstellation.cpp \
$(SOURCEDIR)/SSCoordinates.hpp \
$(SOURCEDIR)/SSEvent.hpp \
$(SOURCEDIR)/SSFeature.hpp \
$(SOURCEDIR)/SSHTM.hpp \
$(SOURCEDIR)/SSIdentifier.hpp \
$(SOURCEDIR)/SSImportGCVS.hpp \
$(SOURCEDIR)/SSImportGJ.hpp \
$(SOURCEDIR)/SSImportHIP.hpp \
$(SOURCEDIR)/SSImportMPC.hpp \
$(SOURCEDIR)/SSImportSKY2000.hpp \
$(SOURCEDIR)/SSImportTLE.hpp \
$(SOURCEDIR)/SSImportTYC.hpp \
$(SOURCEDIR)/SSImportWDS.hpp \
$(SOURCEDIR)/SSJPLDEphemeris.hpp \
$(SOURCEDIR)/SSMatrix.hpp \
$(SOURCEDIR)/SSMoonEphemeris.hpp \
$(SOURCEDIR)/SSMount.hpp \
$(SOURCEDIR)/SSMountModel.hpp \
$(SOURCEDIR)/SSObject.hpp \
$(SOURCEDIR)/SSOrbit.hpp \
$(SOURCEDIR)/SSPlanet.hpp \
$(SOURCEDIR)/SSPSEphemeris.hpp \
$(SOURCEDIR)/SSSerial.hpp\
$(SOURCEDIR)/SSSocket.hpp \
$(SOURCEDIR)/SSStar.hpp \
$(SOURCEDIR)/SSTime.hpp \
$(SOURCEDIR)/SSTLE.hpp \
$(SOURCEDIR)/SSUtilities.hpp \
$(SOURCEDIR)/SSVector.hpp \
$(SOURCEDIR)/SSView.hpp \
$(SOURCEDIR)/VSOP2013/ELPMPP02.hpp \
$(SOURCEDIR)/VSOP2013/VSOP2013.hpp \
$(SOURCEDIR)/Tetra3/Tetra3.hpp \
$(SOURCEDIR)/Tetra3/cnpy.h \
$(SOURCEDIR)/Tetra3/svdcmp.h \

# Name of C/C++ compiler

CC=gcc

# Command-line options passed to C/C++ compiler, including:
# -I = relative paths to directories containing header files
# -D = pre-processor macro definitions
# -O = optimization
# -W = compiler warnings

CFLAGS=-O2 \
-Wno-unused-result \
-I$(SOURCEDIR) \
-I$(SOURCEDIR)/VSOP2013 \
-I$(SOURCEDIR)/Tetra3 \
-std=c++20

# Command-line options passed to linker, including:
# -l = names of libraries to link with
# -L = relative paths to directories containing libraries

LDFLAGS=-lstdc++ -lm -lz -pthread

# Generate list of object files from names of C and C++ source files

CPPOBJS=$(SSCORE_SOURCES:.cpp=.o)
OBJECTS=$(CPPOBJS)
OBJECTS=$(CPPOBJS:.c=.o)

# Default target is test executable

all:	test mounttest tetratest tletest

# This target runs the sstest executable, with default commend-line arguments

run:	mounttest
	./sstest ../../SSData .

# This target runs the ssmounttest executable as super-user

runmount: mounttest
	sudo ./ssmounttest

# This target runs the sstetratest executable with the Tetra3 pattern database in the SSData/Stars directory.

runtetra: tetratest
	./sstetratest ../../SSData/Stars/Tetra3.npz
	
# This target runs the sstletest executable with TLE data in the SSData/SolarSystem/Satellites directory.

runtle: tletest
	./sstletest ../../SSData/SolarSystem/Satellites/brightest.txt

# These targets build object files from C and C++ source files

.c.o:
	$(CC) $(CFLAGS) -c -std=c17 $< -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

# This target builds the test executable from object files

test:	$(OBJECTS) $(SSCORE_HEADERS) ../SSTest.cpp
	$(CC) -o sstest $(CFLAGS) ../SSTest.cpp $(OBJECTS) $(LDFLAGS)

# This target builds the telescope mount test executable from object files

mounttest:	$(OBJECTS) $(SSCORE_HEADERS) ../SSMountTest.cpp
	$(CC) -o ssmounttest $(CFLAGS) ../SSMountTest.cpp $(OBJECTS) $(LDFLAGS)

# This target builds the tetra3 plate solver test executable from object files
 
tetratest:	$(OBJECTS) $(SSCORE_HEADERS) ../SSTetraTest.cpp
	$(CC) -o sstetratest $(CFLAGS) ../SSTetraTest.cpp $(OBJECTS) $(LDFLAGS)

# This target bullds the TLE and SGP4/SDP4 test executable from object files
tletest:	$(OBJECTS) $(SSCORE_HEADERS) ../SSTLETest.cpp
	$(CC) -o sstletest $(CFLAGS) ../SSTLETest.cpp $(OBJECTS) $(LDFLAGS)
	
# This target removes all object files, the executables,
# and CSV files generated by running the executable

clean:
	rm -f $(OBJECTS) sstest ssmounttest sstetratest sstletest *.csv *.tle
