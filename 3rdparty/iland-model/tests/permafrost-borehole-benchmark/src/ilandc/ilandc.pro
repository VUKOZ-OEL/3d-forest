#-------------------------------------------------
#
# iLand console version project file. See iland-model.org
#
#-------------------------------------------------

## iLand console application
QT       += core
QT       -= gui ### we include the GUI library for QColor, QImage
QT       += xml
QT       += sql
QT       += qml
QT       += concurrent

INCLUDEPATH += ../core \
    ../tools \
    ../output \
    ../abe \
    ../abe/output \
    ../bite \
    ../ilandc


DEPENDPATH += plugins
CONFIG += exceptions
CONFIG += rtti

# LIBS += -lQt5Concurrent

CONFIG(debug, debug|release) {
win32-msvc*:contains(QMAKE_TARGET.arch, x86_64):{
#debug msvc
PRE_TARGETDEPS += ../plugins/iland_fired.lib
PRE_TARGETDEPS += ../plugins/iland_windd.lib
PRE_TARGETDEPS += ../plugins/iland_barkbeetled.lib
LIBS += -L../plugins -liland_fired -liland_windd -liland_barkbeetled
message(windows debug)
}
win32:*gcc*: {
PRE_TARGETDEPS += ../plugins/libiland_fired.a
PRE_TARGETDEPS += ../plugins/libiland_windd.a
PRE_TARGETDEPS += ../plugins/libiland_barkbeetled.a
LIBS += -L../plugins -liland_fired -liland_windd -liland_barkbeetled
message(gcc debug)
}
linux-g++: {
 ## debug on linux
message("linux g++ debug")
# QMAKE_CXXFLAGS += -g -O2
PRE_TARGETDEPS += ../plugins/libiland_fire.a
PRE_TARGETDEPS += ../plugins/libiland_wind.a
PRE_TARGETDEPS += ../plugins/libiland_barkbeetle.a
LIBS += -L../plugins -liland_fire -liland_wind -liland_barkbeetle
}

}


CONFIG(release, debug|release) {
win32:*gcc*: {
# release stuff
PRE_TARGETDEPS += ../plugins/libiland_fire.a
PRE_TARGETDEPS += ../plugins/libiland_wind.a
PRE_TARGETDEPS += ../plugins/libiland_barkbeetle.a
LIBS += -L../plugins -liland_fire -liland_wind -liland_barkbeetle
message(gcc release)
}
win32-msvc*:contains(QMAKE_TARGET.arch, x86_64):{
#debug msvc
PRE_TARGETDEPS += ../plugins/iland_fire.lib
PRE_TARGETDEPS += ../plugins/iland_wind.lib
PRE_TARGETDEPS += ../plugins/iland_barkbeetle.lib
### debug symbols...
#QMAKE_CXXFLAGS_RELEASE += /Zi
#QMAKE_LFLAGS_RELEASE += /DEBUG
LIBS += -L../plugins -liland_fire -liland_wind -liland_barkbeetle
message(windows release x)
}
linux-g++*: {
 ## release on linux
message("linux g++ release")
PRE_TARGETDEPS += ../plugins/libiland_fire.a
PRE_TARGETDEPS += ../plugins/libiland_wind.a
PRE_TARGETDEPS += ../plugins/libiland_barkbeetle.a
LIBS += -L../plugins -liland_fire -liland_wind -liland_barkbeetle
#QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
#QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
#message($$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO)

}
linux-icc*: {
 ## release on linux
message("linux intel icc release")
QMAKE_CXXFLAGS -= -O2
QMAKE_CXXFLAGS += -O3
PRE_TARGETDEPS += ../plugins/libiland_fire.a
PRE_TARGETDEPS += ../plugins/libiland_wind.a
PRE_TARGETDEPS += ../plugins/libiland_barkbeetle.a
LIBS += -L../plugins -liland_fire -liland_wind -liland_barkbeetle
#QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
#QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

}
}

DEFINES += NO_DEBUG_MSGS

CONFIG += precompile_header

TARGET = ilandc
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# to enable debug symbols in release code
# debug information in release-mode executable
#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_LFLAGS_RELEASE -= -Wl,-s


### Flag to allow 3GB on Win 32
### you also need to modify boot.ini ...
#QMAKE_LFLAGS_CONSOLE += -Wl,--large-address-aware

SOURCES += main.cpp \
    consoleshell.cpp \
    ../core/version.cpp \
    ../core/model.cpp \
    ../core/modelcontroller.cpp \
    ../core/grid.cpp \
    ../core/tree.cpp \
    ../tools/expression.cpp \
    ../tools/helper.cpp \
    ../core/stamp.cpp \
    ../core/stampcontainer.cpp \
    ../core/species.cpp \
    ../tools/ticktack.cpp \
    ../tools/settingmetadata.cpp \
    ../tools/globalsettings.cpp \
    ../core/speciesset.cpp \
    ../core/resourceunit.cpp \
    ../tools/xmlhelper.cpp \
    ../core/standloader.cpp \
    ../core/resourceunitspecies.cpp \
    ../core/production3pg.cpp \
    ../core/threadrunner.cpp \
    ../tools/expressionwrapper.cpp \
    ../output/output.cpp \
    ../output/treeout.cpp \
    ../output/outputmanager.cpp \
    ../output/standout.cpp \
    ../output/landscapeout.cpp \
    ../core/standstatistics.cpp \
    ../output/dynamicstandout.cpp \
    ../core/management.cpp \
    ../core/speciesresponse.cpp \
    ../core/climate.cpp \
    ../core/modelsettings.cpp \
    ../core/phenology.cpp \
    ../tools/floatingaverage.cpp \
    ../output/productionout.cpp \
    ../core/watercycle.cpp \
    ../tools/climateconverter.cpp \
    ../tools/csvfile.cpp \
    ../tools/scriptglobal.cpp \
    ../output/standdeadout.cpp \
    ../core/environment.cpp \
    ../output/managementout.cpp \
    ../tools/sqlhelper.cpp \
    ../tools/random.cpp \
    ../core/timeevents.cpp \
    ../core/seeddispersal.cpp \
    ../core/establishment.cpp \
    ../core/soil.cpp \
    ../core/snag.cpp \
    ../core/saplings.cpp \
    ../output/saplingout.cpp \
    ../tools/gisgrid.cpp \
    ../tools/mapgrid.cpp \
    ../tools/randomgenerator.cpp \
    ../output/carbonout.cpp \
    ../output/carbonflowout.cpp \
    ../tools/modules.cpp \
    ../tools/dem.cpp \
    ../3rdparty/SimpleRNG.cpp \
    ../output/snapshot.cpp \
    ../tools/spatialanalysis.cpp \
    ../tools/statdata.cpp \
    ../tools/debugtimer.cpp \
    ../abe/fomewrapper.cpp \
    ../abe/fmstand.cpp \
    ../abe/agent.cpp \
    ../abe/fmunit.cpp \
    ../abe/agenttype.cpp \
    ../abe/fomescript.cpp \
    ../abe/activity.cpp \
    ../abe/forestmanagementengine.cpp \
    ../abe/fmstp.cpp \
    ../abe/thinning.cpp \
    ../abe/actgeneral.cpp \
    ../abe/abegrid.cpp \
    ../abe/scheduler.cpp \
    ../abe/fmtreelist.cpp \
    ../abe/actscheduled.cpp \
    ../abe/actplanting.cpp \
    ../abe/actsalvage.cpp \
    ../abe/output/unitout.cpp \
    ../abe/output/abestandout.cpp \
    ../abe/output/abestandremovalout.cpp \
    ../abe/actthinning.cpp \
    ../core/grasscover.cpp \
    ../tools/scriptgrid.cpp \
    ../output/waterout.cpp \
    ../core/dbhdistribution.cpp \
    ../output/svdout.cpp \
    ../core/svdstate.cpp \
    ../output/soilinputout.cpp \
    ../output/devstageout.cpp \
    ../output/ecovizout.cpp \
    ../tools/scripttree.cpp \
    ../tools/scriptresourceunit.cpp \
    ../bite/bitescript.cpp \
    ../bite/biteengine.cpp \
    ../bite/biteagent.cpp \
    ../bite/bitecell.cpp \
    ../bite/biteitem.cpp \
    ../bite/bitedispersal.cpp \
    ../bite/bitecolonization.cpp \
    ../bite/bitecellscript.cpp \
    ../bite/bitewrapper.cpp \
    ../bite/bitebiomass.cpp \
    ../bite/bitelifecycle.cpp \
    ../bite/biteimpact.cpp \
    ../bite/biteclimate.cpp \
    ../bite/biteoutput.cpp \
    ../abe/fmsaplinglist.cpp \
    ../bite/biteoutputitem.cpp \
    ../core/permafrost.cpp


HEADERS += \
    consoleshell.h \
    stable.h \
    ../core/version.h \
    ../core/model.h \
    ../core/modelcontroller.h \
    ../core/grid.h \
    ../core/tree.h \
    ../tools/expression.h \
    ../tools/helper.h \
    ../core/exception.h \
    ../core/global.h \
    ../core/stamp.h \
    ../core/stampcontainer.h \
    ../core/species.h \
    ../tools/ticktack.h \
    ../tools/settingmetadata.h \
    ../tools/globalsettings.h \
    ../core/speciesset.h \
    ../core/model.h \
    ../core/resourceunit.h \
    ../tools/xmlhelper.h \
    ../core/standloader.h \
    ../core/resourceunitspecies.h \
    ../core/production3pg.h \
    ../core/threadrunner.h \
    ../tools/expressionwrapper.h \
    ../output/output.h \
    ../output/treeout.h \
    ../output/outputmanager.h \
    ../output/standout.h \
    ../output/landscapeout.h \
    ../output/devstageout.h \
    ../output/ecovizout.h \
    ../core/standstatistics.h \
    ../output/dynamicstandout.h \
    ../core/management.h \
    ../core/speciesresponse.h \
    ../core/climate.h \
    ../core/modelsettings.h \
    ../core/phenology.h \
    ../tools/floatingaverage.h \
    ../output/productionout.h \
    ../core/watercycle.h \
    ../tools/climateconverter.h \
    ../tools/csvfile.h \
    ../tools/scriptglobal.h \
    ../output/standdeadout.h \
    ../core/environment.h \
    ../output/managementout.h \
    ../tools/sqlhelper.h \
    ../tools/random.h \
    ../tools/randomgenerator.h \
    ../3rdparty/MersenneTwister.h \
    ../core/timeevents.h \
    ../core/seeddispersal.h \
    ../core/establishment.h \
    ../core/soil.h \
    ../core/snag.h \
    ../core/saplings.h \
    ../output/saplingout.h \
    ../tools/gisgrid.h \
    ../tools/mapgrid.h \
    ../output/carbonout.h \
    ../output/carbonflowout.h \
    ../core/plugin_interface.h \
    ../tools/modules.h \
    ../tools/dem.h \
    ../core/layeredgrid.h \
    ../3rdparty/SimpleRNG.h \
    ../output/snapshot.h \
    ../tools/spatialanalysis.h \
    ../tools/statdata.h \
    ../tools/debugtimer.h \
    ../abe/activity.h \
    ../abe/forestmanagementengine.h \
    ../abe/abe_global.h \
    ../abe/fomewrapper.h \
    ../abe/fmstand.h \
    ../abe/agent.h \
    ../abe/fmunit.h \
    ../abe/agenttype.h \
    ../abe/fomescript.h \
    ../output/landscapeout.h \
    ../abe/fmstp.h \
    ../abe/thinning.h \
    ../abe/actgeneral.h \
    ../abe/abegrid.h \
    ../abe/scheduler.h \
    ../abe/fmtreelist.h \
    ../abe/actscheduled.h \
    ../abe/actplanting.h \
    ../abe/actsalvage.h \
    ../abe/output/unitout.h \
    ../abe/output/abestandout.h \
    ../abe/output/abestandremovalout.h \
    ../abe/actthinning.h \
    ../core/grasscover.h \
    ../tools/scriptgrid.h \
    ../output/waterout.h \
    ../core/dbhdistribution.h \
    ../output/svdout.h \
    ../core/svdstate.h \
    ../output/soilinputout.h \
    ../tools/scripttree.h \
    ../tools/scriptresourceunit.h \
    ../bite/bitescript.h \
    ../bite/biteengine.h \
    ../bite/bite_global.h \
    ../bite/biteagent.h \
    ../bite/bitecell.h \
    ../bite/biteitem.h \
    ../bite/bitedispersal.h \
    ../bite/bitecolonization.h \
    ../bite/bitecellscript.h \
    ../bite/bitewrapper.h \
    ../bite/bitebiomass.h \
    ../bite/bitelifecycle.h \
    ../bite/biteimpact.h \
    ../bite/biteclimate.h \
    ../bite/biteoutput.h \
    ../abe/fmsaplinglist.h \
    ../bite/biteoutputitem.h \
    ../core/permafrost.h








