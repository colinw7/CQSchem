TEMPLATE = app

TARGET = CQSchem

QT += widgets svg

DEPENDPATH += .

QMAKE_CXXFLAGS += \
-std=c++14 \

MOC_DIR = .moc

CONFIG += staticlib
CONFIG += c++14

SOURCES += \
CQSchem.cpp \

HEADERS += \
CQSchem.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj

INCLUDEPATH += \
. \
../include \
../../CQUtil/include \
../../CUtil/include \
../../CMath/include \
../../COS/include \

unix:LIBS += \
-L../lib \
-L../../CQUtil/lib \
-lCQUtil \
