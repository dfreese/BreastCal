TEMPLATE = app
QT += core gui
#CONFIG -= qt
#CONFIG += console
CONFIG += c++11
CONFIG += debug
TARGET = ../../bin/view_cal
QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -pedantic

LIBS += -L$$(LIBMIIL_DIR)/lib -lmiil_core
LIBS += -L$$(LIBMIIL_DIR)/lib -lmiil_config
LIBS += -L$$(LIBMIIL_DIR)/lib -lmiil_process
INCLUDEPATH += $$(LIBMIIL_DIR)/include
DEPENDPATH += $$(LIBMIIL_DIR)/include

LIBS += -L$$(ROOTSYS)/lib -lHist -lSpectrum -lCore -lRIO -lGpad -lGQt
INCLUDEPATH += $$(ROOTSYS)/include
DEPENDPATH += $$(ROOTSYS)/include


SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui
