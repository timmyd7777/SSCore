QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

###

SSCoreDIR = ../..

#windows{
#DEFINES += \
#M_PI = 3.141592653589793\
#M_2PI = 6.283185307179586\
#M_PI_2 = 1.570796326794897
#}


INCLUDEPATH += \
$$SSCoreDIR/SSCode \
$$SSCoreDIR/SSCode/VSOP2013 \
$$SSCoreDIR/SSTest\
$$SSCoreDIR/SSTest/Android/sstest/src/main/jni\
$$SSCoreDIR/SSTest/Android/sstest/src/main/cpp

HEADERS += \
    $$SSCoreDIR/SSCode/VSOP2013/ELPMPP02.hpp \
    $$SSCoreDIR/SSCode/VSOP2013/VSOP2013.hpp \
    $$SSCoreDIR/SSCode/SSAngle.hpp \
    $$SSCoreDIR/SSCode/SSConstellation.hpp \
    $$SSCoreDIR/SSCode/SSCoordinates.hpp \
    $$SSCoreDIR/SSCode/SSEvent.hpp \
    $$SSCoreDIR/SSCode/SSHTM.hpp \
    $$SSCoreDIR/SSCode/SSIdentifier.hpp \
    $$SSCoreDIR/SSCode/SSImportGJ.hpp \
    $$SSCoreDIR/SSCode/SSImportHIP.hpp \
    $$SSCoreDIR/SSCode/SSImportMPC.hpp \
    $$SSCoreDIR/SSCode/SSImportNGCIC.hpp \
    $$SSCoreDIR/SSCode/SSImportSKY2000.hpp \
    $$SSCoreDIR/SSCode/SSJPLDEphemeris.hpp \
    $$SSCoreDIR/SSCode/SSMatrix.hpp \
    $$SSCoreDIR/SSCode/SSMoonEphemeris.hpp \
    $$SSCoreDIR/SSCode/SSObject.hpp \
    $$SSCoreDIR/SSCode/SSOrbit.hpp \
    $$SSCoreDIR/SSCode/SSPSEphemeris.hpp \
    $$SSCoreDIR/SSCode/SSPlanet.hpp \
    $$SSCoreDIR/SSCode/SSStar.hpp \
    $$SSCoreDIR/SSCode/SSTLE.hpp \
    $$SSCoreDIR/SSCode/SSTime.hpp \
    $$SSCoreDIR/SSCode/SSUtilities.hpp \
    $$SSCoreDIR/SSCode/SSVPEphemeris.hpp \
    $$SSCoreDIR/SSCode/SSVector.hpp \
    $$SSCoreDIR/SSCode/SSView.hpp

android{
HEADERS += \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/JNIUtilities.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSAngle.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSCoordinates.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSDate.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSDegMinSec.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSEvent.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSEventTime.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSHourMinSec.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSIdentifier.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSJPLDEphemeris.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSMatrix.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSObject.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSObjectArray.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSPass.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSRTS.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSSpherical.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSTime.h \
    $$SSCoreDIR/SSTest/Android/sstest/src/main/jni/com_southernstars_sscore_JSSVector.h \
    $$SSCoreDIR/SSTest/SSTest.h
}

SOURCES += \
        $$SSCoreDIR/SSCode/SSAngle.cpp \
        $$SSCoreDIR/SSCode/SSConstellation.cpp \
        $$SSCoreDIR/SSCode/SSCoordinates.cpp \
        $$SSCoreDIR/SSCode/SSEvent.cpp \
        $$SSCoreDIR/SSCode/SSHTM.cpp \
        $$SSCoreDIR/SSCode/SSIdentifier.cpp \
        $$SSCoreDIR/SSCode/SSImportGJ.cpp \
        $$SSCoreDIR/SSCode/SSImportHIP.cpp \
        $$SSCoreDIR/SSCode/SSImportMPC.cpp \
        $$SSCoreDIR/SSCode/SSImportNGCIC.cpp \
        $$SSCoreDIR/SSCode/SSImportSKY2000.cpp \
        $$SSCoreDIR/SSCode/SSJPLDEphemeris.cpp \
        $$SSCoreDIR/SSCode/SSMatrix.cpp \
        $$SSCoreDIR/SSCode/SSMoonEphemeris.cpp \
        $$SSCoreDIR/SSCode/SSObject.cpp \
        $$SSCoreDIR/SSCode/SSOrbit.cpp \
        $$SSCoreDIR/SSCode/SSPSEphemeris.cpp \
        $$SSCoreDIR/SSCode/SSPlanet.cpp \
        $$SSCoreDIR/SSCode/SSStar.cpp \
        $$SSCoreDIR/SSCode/SSTLE.cpp \
        $$SSCoreDIR/SSCode/SSTime.cpp \
        $$SSCoreDIR/SSCode/SSUtilities.cpp \
        $$SSCoreDIR/SSCode/SSVPEphemeris.cpp \
        $$SSCoreDIR/SSCode/SSVector.cpp \
        $$SSCoreDIR/SSCode/SSView.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/ELPMPP02.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/VSOP2013.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/VSOP2013p1.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/VSOP2013p2.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/VSOP2013p3.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/VSOP2013p4.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/VSOP2013p5.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/VSOP2013p6.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/VSOP2013p7.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/VSOP2013p8.cpp \
        $$SSCoreDIR/SSCode/VSOP2013/VSOP2013p9.cpp
