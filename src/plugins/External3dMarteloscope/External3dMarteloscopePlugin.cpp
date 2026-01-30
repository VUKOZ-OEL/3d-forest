/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file External3dMarteloscopePlugin.cpp */

// Include 3D Forest.
#include <External3dMarteloscopePlugin.hpp>
#include <External3dMarteloscopeRunner.hpp>
#include <File.hpp>
#include <FileFormatCsv.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCoreApplication>

// Include local.
#define LOG_MODULE_NAME "External3dMarteloscopePlugin"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define ICON(name) (ThemeIcon(":/External3dMarteloscopeResources/", name))

static std::string toStdString(const QString &str)
{
    return str.toUtf8().constData();
}

External3dMarteloscopePlugin::External3dMarteloscopePlugin()
    : mainWindow_(nullptr),
      runner_(nullptr)
{
}

External3dMarteloscopePlugin::~External3dMarteloscopePlugin()
{
    if (runner_ && runner_->isRunning())
    {
        runner_->stop();
    }
}

void External3dMarteloscopePlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "External",
                              "External",
                              tr("3d-Marteloscope"),
                              tr("Start 3d-Marteloscope"),
                              ICON("external-3d-marteloscope"),
                              this,
                              SLOT(slotPlugin()),
                              MAIN_WINDOW_MENU_EXTERNAL_PRIORITY);
}

void External3dMarteloscopePlugin::slotPlugin()
{
    std::string errorMessage;

    if (!runner_)
    {
        runner_ = new External3dMarteloscopeRunner(mainWindow_);
    }

    try
    {
        run();
    }
    catch (std::exception &e)
    {
        errorMessage = e.what();
    }
    catch (...)
    {
        errorMessage = "unknown";
    }

    if (!errorMessage.empty())
    {
        mainWindow_->showError(errorMessage.c_str());
    }
}

void External3dMarteloscopePlugin::run()
{
    std::string projectPath = mainWindow_->editor().projectPath();

    exportTrees(projectPath);
    runPythonApp(projectPath);
}

void External3dMarteloscopePlugin::runPythonApp(const std::string &projectPath)
{
    LOG_DEBUG(<< "Start python app with project <" << projectPath << ">.");

    QString pythonPath = "python";
    QString appPath =
        QCoreApplication::applicationDirPath() +
        "/plugins/3DForestExternal3dMarteloscopePlugin/python/app.py";

    runner_->start(pythonPath, appPath, QString::fromStdString(projectPath));

    LOG_DEBUG(<< "Finished starting python app.");
}

void External3dMarteloscopePlugin::exportTrees(const std::string &projectPath)
{
    LOG_DEBUG(<< "Export trees from project <" << projectPath << ">.");

    // Create table with tree data.
    FileFormatTable table;
    fillTreeTable(&table);

    // Export table to CSV formatted file.
    FileFormatCsv csv;
    const std::string csvDirectory = File::directory(projectPath);
    const std::string csvPath = File::join(csvDirectory, "trees.csv");
    LOG_DEBUG(<< "Export trees to file <" << csvPath << ">.");
    csv.setFileName(csvPath);
    csv.create(table);
}

void External3dMarteloscopePlugin::fillTreeTable(FileFormatTable *table)
{
    mainWindow_->suspendThreads();

    auto &editor = mainWindow_->editor();

    const Segments &segments = editor.segments();
    const SpeciesList &speciesList = editor.speciesList();
    const ManagementStatusList &management = editor.managementStatusList();
    double ppm = editor.settings().unitsSettings().pointsPerMeter()[0];

    // Header "id;species;x;y;dbh;height".
    auto &columns = table->columns;
    columns.resize(6);
    columns[0].header = "id";
    columns[1].header = "species";
    columns[2].header = "x";
    columns[3].header = "y";
    columns[4].header = "dbh";
    columns[5].header = "height";

    // Data.
    for (size_t r = 0; r < segments.size(); r++)
    {
        const Segment &segment = segments[r];
        const auto &attributes = segment.treeAttributes;

        if (segment.id == 0)
        {
            continue;
        }

        std::string species;
        size_t speciesIdx = speciesList.index(segment.speciesId, false);
        if (speciesIdx != SIZE_MAX)
        {
            species = speciesList[speciesIdx].abbreviation;
        }

        size_t managementIdx =
            management.index(segment.managementStatusId, false);
        if (managementIdx != SIZE_MAX &&
            management[managementIdx].label != "Untouched" &&
            management[managementIdx].label != "Target tree")
        {
            // Skip trees with all other management status.
            continue;
        }

        columns[0].cells.push_back(toString(segment.id));
        columns[1].cells.push_back(species);
        columns[2].cells.push_back(toString(attributes.crownCenter[0] / ppm));
        columns[3].cells.push_back(toString(attributes.crownCenter[1] / ppm));
        columns[4].cells.push_back(toString(attributes.dbh / ppm));
        columns[5].cells.push_back(toString(attributes.height / ppm));
    }
}
