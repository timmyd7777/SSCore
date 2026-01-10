cmake_minimum_required(VERSION 3.16)

project(SSCoreTests LANGUAGES C CXX)

# ------------------------------------------------------------------
# Configuration
# ------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
message(status "CMAKE_CURRENT_LIST_DIR is ${CMAKE_CURRENT_LIST_DIR}")
set(SOURCEDIR "${CMAKE_CURRENT_LIST_DIR}/SSCode")
set(SOURCEMAINDIR "${CMAKE_CURRENT_LIST_DIR}/SSTest/")

# ------------------------------------------------------------------
# Compiler options
# ------------------------------------------------------------------

add_compile_options(
    -O2
    -Wno-unused-result
)

# ------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------

include_directories(
    ${SOURCEDIR}
    ${SOURCEDIR}/VSOP2013
    ${SOURCEDIR}/Tetra3
)

# ------------------------------------------------------------------
# SSCore sources
# ------------------------------------------------------------------

set(SSCORE_SOURCES
    ${SOURCEDIR}/SSAngle.cpp
    ${SOURCEDIR}/SSConstellation.cpp
    ${SOURCEDIR}/SSCoordinates.cpp
    ${SOURCEDIR}/SSEvent.cpp
    ${SOURCEDIR}/SSFeature.cpp
    ${SOURCEDIR}/SSHTM.cpp
    ${SOURCEDIR}/SSIdentifier.cpp
    ${SOURCEDIR}/SSImportGCVS.cpp
    ${SOURCEDIR}/SSImportGJ.cpp
    ${SOURCEDIR}/SSImportHIP.cpp
    ${SOURCEDIR}/SSImportMPC.cpp
    ${SOURCEDIR}/SSImportSKY2000.cpp
    ${SOURCEDIR}/SSImportTLE.cpp
    ${SOURCEDIR}/SSImportTYC.cpp
    ${SOURCEDIR}/SSImportWDS.cpp
    ${SOURCEDIR}/SSJPLDEphemeris.cpp
    ${SOURCEDIR}/SSMatrix.cpp
    ${SOURCEDIR}/SSMoonEphemeris.cpp
    ${SOURCEDIR}/SSMount.cpp
    ${SOURCEDIR}/SSMountModel.cpp
    ${SOURCEDIR}/SSObject.cpp
    ${SOURCEDIR}/SSOrbit.cpp
    ${SOURCEDIR}/SSPlanet.cpp
    ${SOURCEDIR}/SSPSEphemeris.cpp
    ${SOURCEDIR}/SSSerial.cpp
    ${SOURCEDIR}/SSSocket.cpp
    ${SOURCEDIR}/SSStar.cpp
    ${SOURCEDIR}/SSTime.cpp
    ${SOURCEDIR}/SSTLE.cpp
    ${SOURCEDIR}/SSUtilities.cpp
    ${SOURCEDIR}/SSVector.cpp
    ${SOURCEDIR}/SSView.cpp

    ${SOURCEDIR}/VSOP2013/ELPMPP02.cpp
    ${SOURCEDIR}/VSOP2013/VSOP2013.cpp
    ${SOURCEDIR}/VSOP2013/VSOP2013p1.cpp
    ${SOURCEDIR}/VSOP2013/VSOP2013p2.cpp
    ${SOURCEDIR}/VSOP2013/VSOP2013p3.cpp
    ${SOURCEDIR}/VSOP2013/VSOP2013p4.cpp
    ${SOURCEDIR}/VSOP2013/VSOP2013p5.cpp
    ${SOURCEDIR}/VSOP2013/VSOP2013p6.cpp
    ${SOURCEDIR}/VSOP2013/VSOP2013p7.cpp
    ${SOURCEDIR}/VSOP2013/VSOP2013p8.cpp
    ${SOURCEDIR}/VSOP2013/VSOP2013p9.cpp

    ${SOURCEDIR}/Tetra3/Tetra3.cpp
    ${SOURCEDIR}/Tetra3/cnpy.cpp
    ${SOURCEDIR}/Tetra3/svdcmp.c
)

# ------------------------------------------------------------------
# Build SSCore as a library
# ------------------------------------------------------------------

add_library(sscore STATIC ${SSCORE_SOURCES})

target_include_directories(sscore PUBLIC
    ${SOURCEDIR}
    ${SOURCEDIR}/VSOP2013
    ${SOURCEDIR}/Tetra3
)

# ------------------------------------------------------------------
# System libraries
# ------------------------------------------------------------------

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

target_link_libraries(sscore
    PUBLIC
        Threads::Threads
        ZLIB::ZLIB
        m
)

# ------------------------------------------------------------------
# Executables
# ------------------------------------------------------------------

add_executable(sstest
    ${SOURCEMAINDIR}/SSTest.cpp
)

add_executable(ssmounttest
    ${SOURCEMAINDIR}/SSMountTest.cpp
)

add_executable(sstetratest
    ${SOURCEMAINDIR}SSTetraTest.cpp
)

add_executable(sstletest
    ${SOURCEMAINDIR}SSTLETest.cpp
)

# ------------------------------------------------------------------
# Link executables to SSCore
# ------------------------------------------------------------------

target_link_libraries(sstest PRIVATE sscore)
target_link_libraries(ssmounttest PRIVATE sscore)
target_link_libraries(sstetratest PRIVATE sscore)
target_link_libraries(sstletest PRIVATE sscore)

# ------------------------------------------------------------------
# Convenience targets (equivalent to Makefile run targets)
# ------------------------------------------------------------------

add_custom_target(run
    COMMAND sstest ../../SSData .
    DEPENDS sstest
)

add_custom_target(runmount
    COMMAND sudo ssmounttest
    DEPENDS ssmounttest
)

add_custom_target(runtetra
    COMMAND sstetratest ../../SSData/Stars/Tetra3.npz
    DEPENDS sstetratest
)

add_custom_target(runtle
    COMMAND sstletest ../../SSData/SolarSystem/Satellites/brightest.txt
    DEPENDS sstletest
)

