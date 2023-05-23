CONFIG+=debug \
    c++17

SOURCES +=  $$PWD/testrunner.cpp

INCLUDEPATH += $$PWD/../src

LIBS += -L$$OUT_PWD/../src -limg_traverse


