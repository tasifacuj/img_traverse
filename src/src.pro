QMAKE_CXXFLAGS += -std=c++17

QT -= gui

TARGET = img_traverse
TEMPLATE = lib
CONFIG += staticlib

SOURCES += $$PWD/ImgTraverse.cpp

HEADERS += $$PWD/ImgTraverse.hpp \
    Serialize.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += $$PWD


