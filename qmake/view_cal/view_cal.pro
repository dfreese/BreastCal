TEMPLATE = app
QT += core gui
CONFIG += c++11
CONFIG += debug
TARGET = ../../bin/view_cal
QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -pedantic

LIBS += -ljsoncpp

LIBS += -L$$(LIBMIIL_DIR)/lib -lmiil_core
LIBS += -L$$(LIBMIIL_DIR)/lib -lmiil_config
LIBS += -L$$(LIBMIIL_DIR)/lib -lmiil_process
INCLUDEPATH += $$(LIBMIIL_DIR)/include
DEPENDPATH += $$(LIBMIIL_DIR)/include

LIBS += -L$$(ROOTSYS)/lib -lCore -lRIO -lHist -lSpectrum -lGpad -lGQt -lGraf
INCLUDEPATH += $$(ROOTSYS)/include
DEPENDPATH += $$(ROOTSYS)/include


SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui
