QT += charts
QT += core gui charts printsupport
QT += opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets charts

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/classes/qcustomplot.cpp \
    src/pages/hrv1page.cpp \
    src\Classes\butterworth.cpp \
    src\Classes\filter.cpp \
    src\Classes\hilberttransform.cpp \
    src\Classes\hrv1.cpp \
    src\Classes\hrv2.cpp \
    src\Classes\lms_filter.cpp \
    main.cpp \
    src\Classes\ecg_data.cpp \
    mainwindow.cpp \
    src\Classes\movingaverage.cpp \
    src\Classes\pantompkins.cpp \
    src\Classes\savitzkygolay.cpp \
    src\Classes\hrv_dfa.cpp \
    src\Classes\waves.cpp \
    src\Classes\heart_class.cpp

HEADERS += \
    src/classes/qcustomplot.h \
    src/pages/hrv1page.h \
    src\Classes\butterworth.h \
    src\Classes\ecg_data.h \
    src\Classes\filter.h \
    src\Classes\hilberttransform.h \
    src\Classes\hrv1.h \
    src\Classes\hrv2.h \
    src\Classes\lms_filter.h \
    mainwindow.h \
    src\Classes\movingaverage.h \
    src\Classes\pantompkins.h \
    src\Classes\savitzkygolay.h \
    src\Classes\hrv_dfa.h \
    src\Classes\waves.h \
    src\Classes\heart_class.h

FORMS += \
    ecg_data.ui \
    hrv2.ui \
    mainwindow.ui

TRANSLATIONS += \
    DADM_pl_PL.ts
CONFIG += release
CONFIG += embed_translations

DEFINES += QCUSTOMPLOT_USE_OPENGL

INCLUDEPATH += $$PWD\src\Classes

# podmiany dopiero poniżej tej linijki!!

INCLUDEPATH += C:\Users\mateo\Downloads\DADM (3)\DADM\build\Desktop_Qt_6_8_0_MinGW_64_bit-Debug\debug
INCLUDEPATH += C:\Users\mateo\Downloads\boost_1_87_0
INCLUDEPATH += C:\Users\mateo\Downloads\DADM\eigen-3.4.0
INCLUDEPATH += C:/Qt/6.8.0/mingw_64/include
LIBS += -LC:/Qt/6.8.0/mingw_64/lib -lgsl -lgslcblas -lm -lopengl32


qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    style1.qss

