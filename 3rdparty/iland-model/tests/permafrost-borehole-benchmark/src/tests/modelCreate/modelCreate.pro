QT += xml
QT += sql
INCLUDEPATH += ../../core \
    ../../tools

# PRECOMPILED_HEADER = ../../stable.h
SOURCES = testModelCreate.cpp \
    ../../tools/globalsettings.cpp \
    ../../tools/settingmetadata.cpp \
    ../../tools/helper.cpp \
    ../../tools/xmlhelper.cpp \
    ../../tools/ticktack.cpp \
    ../../tools/expression.cpp \
    ../../core/grid.cpp \
    ../../core/stamp.cpp \
    ../../core/stampcontainer.cpp \
    ../../core/species.cpp \
    ../../core/speciesset.cpp \
    ../../core/tree.cpp \
    ../../core/model.cpp \
    ../../core/ressourceunit.cpp
HEADERS = ../../core/global.h \
    ../../tools/globalsettings.h \
    ../../tools/settingmetadata.h \
    ../../tools/helper.h \
    ../../tools/xmlhelper.h \
    ../../tools/expression.h \
    ../../tools/ticktack.h \
    ../../core/grid.h \
    ../../core/stamp.h \
    ../../core/stampcontainer.h \
    ../../core/species.h \
    ../../core/speciesset.h \
    ../../core/tree.h \
    ../../core/model.h \
    ../../core/ressourceunit.h \
    ../../core/exception.h
CONFIG += qtestlib
