QT += xml
QT += sql
INCLUDEPATH += ../../core \
               ../../tools


SOURCES = testXmlHelper.cpp \
          ../../tools/helper.cpp \
          ../../tools/ticktack.cpp \
          ../../tools/settingmetadata.cpp \
          ../../tools/xmlhelper.cpp \
          ../../tools/globalsettings.cpp
# \
#          ../../tools/globalsettings.cpp \
#          ../../tools/helper.cpp \
#          ../../tools/ticktack.cpp \
#          ../../tools/expression.cpp \
#          ../../core/stamp.cpp \
#          ../../core/stampcontainer.cpp \
#          ../../core/species.cpp \
#          ../../core/speciesset.cpp \
#          ../../core/tree.cpp \
#          ../../core/model.cpp

HEADERS =  ../../tools/xmlhelper.h \
          ../../tools/ticktack.h \
          ../../tools/globalsettings.h \
          ../../tools/settingmetadata.h \
          ../../tools/helper.h


# \
#          ../../tools/globalsettings.h \
#          ../../tools/helper.h \
#          ../../tools/expression.h \
#          ../../tools/ticktack.h \
#          ../../core/stamp.h \
#          ../../core/stampcontainer.h \
#          ../../core/species.h \
#          ../../core/speciesset.h \
#          ../../core/tree.h \
#          ../../core/model.h


CONFIG  += qtestlib

