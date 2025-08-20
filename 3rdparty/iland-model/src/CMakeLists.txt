# Copyright 2020 VUKOZ
#
# This file is part of 3D Forest.
#
# 3D Forest is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# 3D Forest is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.

set(SUB_PROJECT_NAME "ILandModel")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(
    ${SUB_PROJECT_NAME}
    SHARED
    3rdparty/SimpleRNG.cpp
    abe/abegrid.cpp
    abe/actgeneral.cpp
    abe/activity.cpp
    abe/actplanting.cpp
    abe/actsalvage.cpp
    abe/actscheduled.cpp
    abe/actthinning.cpp
    abe/agent.cpp
    abe/agenttype.cpp
    abe/fmdeadtreelist.cpp
    abe/fmsaplinglist.cpp
    abe/fmstand.cpp
    abe/fmstp.cpp
    abe/fmtreelist.cpp
    abe/fmunit.cpp
    abe/fomescript.cpp
    abe/fomewrapper.cpp
    abe/forestmanagementengine.cpp
    abe/output/abestandout.cpp
    abe/output/abestandremovalout.cpp
    abe/output/unitout.cpp
    abe/patch.cpp
    abe/patches.cpp
    abe/scheduler.cpp
    abe/thinning.cpp
    bite/biteagent.cpp
    bite/bitebiomass.cpp
    bite/bitecell.cpp
    bite/bitecellscript.cpp
    bite/biteclimate.cpp
    bite/bitecolonization.cpp
    bite/bitedispersal.cpp
    bite/biteengine.cpp
    bite/biteimpact.cpp
    bite/biteitem.cpp
    bite/bitelifecycle.cpp
    bite/biteoutput.cpp
    bite/biteoutputitem.cpp
    bite/bitescript.cpp
    bite/bitewrapper.cpp
    core/climate.cpp
    core/dbhdistribution.cpp
    core/deadtree.cpp
    core/environment.cpp
    core/establishment.cpp
    core/grasscover.cpp
    core/grid.cpp
    core/management.cpp
    core/microclimate.cpp
    core/model.cpp
    core/modelcontroller.cpp
    core/modelsettings.cpp
    core/permafrost.cpp
    core/phenology.cpp
    core/production3pg.cpp
    core/resourceunit.cpp
    core/resourceunitspecies.cpp
    core/saplings.cpp
    core/seeddispersal.cpp
    core/snag.cpp
    core/soil.cpp
    core/species.cpp
    core/speciesresponse.cpp
    core/speciesset.cpp
    core/stamp.cpp
    core/stampcontainer.cpp
    core/standloader.cpp
    core/standstatistics.cpp
    core/svdstate.cpp
    core/threadrunner.cpp
    core/timeevents.cpp
    core/tree.cpp
    core/version.cpp
    core/watercycle.cpp
    output/carbonflowout.cpp
    output/carbonout.cpp
    output/customaggout.cpp
    output/devstageout.cpp
    output/dynamicstandout.cpp
    output/ecovizout.cpp
    output/landscapeout.cpp
    output/managementout.cpp
    output/output.cpp
    output/outputmanager.cpp
    output/productionout.cpp
    output/saplingout.cpp
    output/snapshot.cpp
    output/soilinputout.cpp
    output/standdeadout.cpp
    output/standout.cpp
    output/svdindicatorout.cpp
    output/svdout.cpp
    output/treeout.cpp
    output/waterout.cpp
    tools/climateconverter.cpp
    tools/colors.cpp
    tools/csvfile.cpp
    tools/debugtimer.cpp
    tools/dem.cpp
    tools/expression.cpp
    tools/expressionwrapper.cpp
    tools/floatingaverage.cpp
    tools/geotiff.cpp
    tools/gisgrid.cpp
    tools/globalsettings.cpp
    tools/helper.cpp
    tools/mapgrid.cpp
    tools/modules.cpp
    tools/random.cpp
    tools/randomgenerator.cpp
    tools/scriptglobal.cpp
    tools/scriptgrid.cpp
    tools/scriptresourceunit.cpp
    tools/scripttree.cpp
    tools/settingmetadata.cpp
    tools/spatialanalysis.cpp
    tools/sqlhelper.cpp
    tools/statdata.cpp
    tools/ticktack.cpp
    tools/viewport.cpp
    tools/xmlhelper.cpp
    plugins/barkbeetle/barkbeetleplugin.cpp
    plugins/barkbeetle/barkbeetlemodule.cpp
    plugins/barkbeetle/barkbeetlescript.cpp
    plugins/barkbeetle/bbgenerations.cpp
    plugins/barkbeetle/barkbeetleout.cpp
    plugins/fire/fireplugin.cpp
    plugins/fire/firemodule.cpp
    plugins/fire/fireout.cpp
    plugins/fire/firescript.cpp
    plugins/wind/windplugin.cpp
    plugins/wind/windmodule.cpp
    plugins/wind/windscript.cpp
    plugins/wind/windout.cpp
)

if (WIN32)
    set_target_properties(
        ${SUB_PROJECT_NAME}
        PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

target_include_directories(
    ${SUB_PROJECT_NAME}
    PUBLIC
    3rdparty
    abe
    abe/output
    bite
    core
    output
    plugins
    plugins/barkbeetle
    plugins/fire
    plugins/wind
    tools
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(
    ${SUB_PROJECT_NAME}
    PRIVATE
    EXPORT_${SUB_PROJECT_NAME}=1
)

target_link_libraries(
    ${SUB_PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Xml
)

install(TARGETS ${SUB_PROJECT_NAME} DESTINATION bin)