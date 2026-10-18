# -------------------------------------------------
# Project created by QtCreator 2009-02-10T13:47:01
# -------------------------------------------------
QT += xml
QT += script
QT += sql
QT += widgets
QT += concurrent
TARGET = fonstudio
TEMPLATE = app

# includepath: adds directories to the standard include (no directory needed when #include a file).
INCLUDEPATH += ../core \
    ../tools \
    ../output

CONFIG += exceptions

# Use Precompiled headers (PCH)
PRECOMPILED_HEADER = stable.h
SOURCES += main.cpp \
    mainwindow.cpp \
    ../core/grid.cpp \
    solarradiation.cpp \
    hemigrid.cpp \
    imagestamp.cpp \
    paintarea.cpp \
    ../tools/expression.cpp \
    ../tools/helper.cpp \
    lightroom.cpp \
    ../core/stamp.cpp \
    ../core/stampcontainer.cpp \
    ../tools/ticktack.cpp \
    ../tools/settingmetadata.cpp \
    ../tools/xmlhelper.cpp \
    ../core/threadrunner.cpp \
    version.cpp \
    ../tools/randomgenerator.cpp \
    ../tools/debugtimer.cpp


HEADERS += mainwindow.h \
    ../stable.h \
    ../core/grid.h \
    solarradiation.h \
    hemigrid.h \
    imagestamp.h \
    paintarea.h \
    ../tools/expression.h \
    ../tools/helper.h \
    lightroom.h \
    ../core/exception.h \
    ../core/global.h \
    ../core/stamp.h \
    ../core/stampcontainer.h \
    ../tools/ticktack.h \
    ../tools/settingmetadata.h \
    ../core/speciesset.h \
    ../tools/xmlhelper.h \
    ../core/threadrunner.h \
    ../3rdparty/MersenneTwister.h \
    version.h \
    ../tools/randomgenerator.h \
    ../tools/debugtimer.h

FORMS += mainwindow.ui
RESOURCES += res/fonstudio.qrc
DEFINES += FONSTUDIO
