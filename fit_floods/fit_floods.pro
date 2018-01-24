TEMPLATE = app
CONFIG -= qt
CONFIG += console
CONFIG += c++11
CONFIG += debug
TARGET = ../bin/fit_floods
QMAKE_CXXFLAGS += -std=c++0x -Wall -Wextra -pedantic

LIBS += -L$$(LIBMIIL_DIR)/lib -lmiil_core
LIBS += -L$$(LIBMIIL_DIR)/lib -lmiil_config
LIBS += -L$$(LIBMIIL_DIR)/lib -lmiil_process
INCLUDEPATH += $$(LIBMIIL_DIR)/include
DEPENDPATH += $$(LIBMIIL_DIR)/include

LIBS += -L$$(ROOTSYS)/lib -lHist -lSpectrum -lCore -lRIO
INCLUDEPATH += $$(ROOTSYS)/include
DEPENDPATH += $$(ROOTSYS)/include


SOURCES += fit_floods.cpp \
    segmentation.cpp

HEADERS += \
    segmentation.h
