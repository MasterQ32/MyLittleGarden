TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

PACKAGES=sdl2 SDL2_mixer SDL2_image

QMAKE_CFLAGS   += $$system(pkg-config --cflags $$PACKAGES)
QMAKE_CXXFLAGS += $$system(pkg-config --cflags $$PACKAGES)
QMAKE_LFLAGS   += $$system(pkg-config --libs $$PACKAGES)

SOURCES += \
    engine.cpp \
    game.cpp

HEADERS += \
    engine.h \
    game.hpp \
    palette.h
