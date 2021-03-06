#-------------------------------------------------
#
# Project created by QtCreator 2017-07-31T11:52:51
#
#-------------------------------------------------

QT += core gui
CONFIG += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RtRt
TEMPLATE = app

DESTDIR = $$PWD

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
QMAKE_LFLAGS += -no-pie

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
    gl_objects/gl_buffer.cpp \
    gl_objects/gl_plane.cpp \
    gl_objects/gl_triangulated_shape.cpp \
    main_window.cpp \
    my_opengl_widget.cpp  \
    util.cpp

HEADERS  += \
    gl_objects/gl_buffer.h \
    gl_objects/gl_plane.h \
    gl_objects/gl_shape.h \
    gl_objects/gl_triangulated_shape.h \
    main_window.h \
    my_opengl_widget.h  \
    objects/light_source.h \
    objects/material.h \
    objects/scene.h \
    objects/sphere.h \
    util.h

FORMS    += \
    main_window.ui 

DISTFILES += \
    shaders/raytrace.frag \ \
    shaders/raytrace.vert

RESOURCES +=
