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

/** @file ImportFilePlugin.cpp */

// Include 3D Forest.
#include <ImportFileDialog.hpp>
#include <ImportFilePlugin.hpp>
#include <IndexFileBuilder.hpp>
#include <MainWindow.hpp>
#include <ThemeIcon.hpp>

// Include Qt.
#include <QCoreApplication>
#include <QFileDialog>
#include <QProgressBar>
#include <QProgressDialog>

// Include 3rd party.
#include <pcdio.h>

// Include local.
#define LOG_MODULE_NAME "ImportFilePlugin"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

#define IMPORT_PLUGIN_FILTER "LAS (LASer) File (*.las);;PCL (*.pcd)"
#define ICON(name) (ThemeIcon(":/ImportFileResources/", name))

static void importPluginPCDLogMessageHandler(pcl::VERBOSITY_LEVEL level,
                                             const std::string &message)
{
    switch (level)
    {
        case pcl::L_WARN:
            LOG_WARNING(<< message);
            break;
        case pcl::L_ERROR:
            LOG_ERROR(<< message);
            break;
        case pcl::L_INFO:
            LOG_INFO(<< message);
            break;
        case pcl::L_DEBUG:
        default:
            LOG_DEBUG(<< message);
            break;
    }
}

ImportFilePlugin::ImportFilePlugin() : mainWindow_(nullptr)
{
    pcl::logMessageHandler = importPluginPCDLogMessageHandler;
}

void ImportFilePlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(&importFileAction_,
                              "File",
                              "File Import/Export",
                              tr("Import..."),
                              tr("Import new point cloud dataset"),
                              ICON("import-file"),
                              this,
                              SLOT(slotImportFile()),
                              MAIN_WINDOW_MENU_FILE_PRIORITY,
                              50);

    mainWindow_->hideToolBar("File Import/Export");
}

void ImportFilePlugin::slotImportFile()
{
    importFile();
}

static void importPluginDialog(MainWindow *mainWindow);

static void importPluginFile(const std::string &pathIn,
                             const std::string &pathOut,
                             const ImportSettings &settings,
                             MainWindow *mainWindow);

static bool importPluginCreateIndex(const std::string &pathIn,
                                    const std::string &pathOut,
                                    const ImportSettings &settings,
                                    MainWindow *mainWindow);

static void importPluginAddAsNewTree(const std::string &path,
                                     MainWindow *mainWindow);

static bool importPluginPcd2Las(const std::string &pathIn,
                                const std::string &pathOut);

void ImportFilePlugin::importFile()
{
    try
    {
        importPluginDialog(mainWindow_);
    }
    catch (std::exception &e)
    {
        mainWindow_->showError(e.what());
        return;
    }
}

static void importPluginDialog(MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Start importing files.");

    QFileDialog fileDialog(mainWindow, QObject::tr("Import File"));
    fileDialog.setNameFilter(QObject::tr(IMPORT_PLUGIN_FILTER));
    fileDialog.setFileMode(QFileDialog::ExistingFiles);

    if (fileDialog.exec() == QDialog::Rejected)
    {
        LOG_DEBUG(<< "Canceled importing files from the dialog.");
        return;
    }

    QStringList files = fileDialog.selectedFiles();
    QStringList selectedFiles;
    for (auto const &file : files)
    {
        if (file.length() > 0)
        {
            selectedFiles.append(file);
        }
    }

    LOG_DEBUG(<< "Selected <" << selectedFiles.count() << "> files.");
    if (selectedFiles.count() < 1)
    {
        LOG_DEBUG(<< "Canceled importing files. No files selected.");
        return;
    }

    // Stop rendering.
    mainWindow->suspendThreads();

    // Import settings.
    ImportFileDialog settingsDialog(mainWindow);

    if (settingsDialog.exec() == QDialog::Rejected)
    {
        return;
    }

    ImportSettings settings = settingsDialog.settings();

    // Import.
    for (auto const &file : selectedFiles)
    {
        std::string pathIn = file.toStdString();
        std::string pathOut = File::replaceExtension(pathIn, ".las");
        importPluginFile(pathIn, pathOut, settings, mainWindow);
    }

    // Update.
    mainWindow->updateNewProject();
    mainWindow->slotRenderViewports();

    LOG_DEBUG(<< "Finished importing files.");
}

static void importPluginFile(const std::string &pathIn,
                             const std::string &pathOut,
                             const ImportSettings &settings,
                             MainWindow *mainWindow)
{
    LOG_DEBUG(<< "Import file <" << pathIn << ">.");

    if (!importPluginCreateIndex(pathIn, pathOut, settings, mainWindow))
    {
        return;
    }

    mainWindow->editor().open(pathOut, settings);

    if (settings.importFilesAsSeparateTrees)
    {
        importPluginAddAsNewTree(pathIn, mainWindow);
    }
}

static bool importPluginCreateIndex(const std::string &pathIn,
                                    const std::string &pathOut,
                                    const ImportSettings &settings,
                                    MainWindow *mainWindow)
{
    std::string ext = toLower(File::fileExtension(pathIn));
    if (ext == "pcd")
    {
        LOG_DEBUG(<< "Import PCD file <" << pathIn << ">.");
        if (!importPluginPcd2Las(pathIn, pathOut))
        {
            return false;
        }
    }
    else if (ext == "las")
    {
        LOG_DEBUG(<< "Import LAS file <" << pathIn << ">.");
    }
    else
    {
        THROW("Unknown file format <" + ext + "> in <" + pathIn + ">.");
    }

    // If the index already exists, then return success.
    std::string pathFile;
    std::string pathIndex;

    pathFile = File::resolvePath(pathOut, mainWindow->editor().projectPath());
    pathIndex = IndexFileBuilder::extension(pathFile);

    if (File::exists(pathIndex))
    {
        return true;
    }

    // Create modal progress dialog with custom progress bar.
    // Custom progress bar allows to display percentage with fractional part.
    QProgressDialog progressDialog(mainWindow);
    progressDialog.setWindowTitle(QObject::tr("Create Index"));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setMinimumDuration(0);

    QProgressBar *progressBar = new QProgressBar(&progressDialog);
    progressBar->setTextVisible(false);
    progressBar->setRange(0, 100);
    progressBar->setValue(progressBar->minimum());
    progressDialog.setBar(progressBar);

    // Initialize index builder.
    IndexFileBuilder builder;
    builder.start(pathOut, pathOut, settings);

    char buffer[80];

    progressDialog.show();

    // Do import operation in progress loop.
    while (!builder.end())
    {
        // Update progress.
        double value = builder.percent();

        std::snprintf(buffer,
                      sizeof(buffer),
                      "Overall progress: %6.2f %% complete",
                      value);

        progressDialog.setValue(static_cast<int>(value));
        progressDialog.setLabelText(buffer);

        QCoreApplication::processEvents();

        if (progressDialog.wasCanceled())
        {
            return false;
        }

        // Process several bytes of the operation.
        builder.next();
    }

    progressDialog.setValue(progressDialog.maximum());

    return true;
}

static bool importPluginPcd2Las(const std::string &pathIn,
                                const std::string &pathOut)
{
    pcl::PCLPointCloud2 cloud2;
    if (pcl::io::loadPCDFile(pathIn, cloud2) != 0)
    {
        THROW("Can't read file '" + pathIn + "'");
    }

    pcl::PointCloud<pcl::PointXYZI> cloud;
    if (!pcl::fromPCLPointCloud2(cloud2, cloud))
    {
        THROW("Can't convert pcd file '" + pathIn + "'");
    }

    /** @todo Progress bar. */
    std::vector<LasFile::Point> points;
    points.resize(cloud.size());
    std::memset(points.data(), 0, sizeof(LasFile::Point) * points.size());

    const float s = 1000.0F;

    for (size_t i = 0; i < cloud.size(); i++)
    {
        points[i].x = static_cast<int32_t>(cloud[i].x * s);
        points[i].y = static_cast<int32_t>(cloud[i].y * s);
        points[i].z = static_cast<int32_t>(cloud[i].z * s);
        points[i].format = 0;
    }

    LasFile::create(pathOut, points, {0.001, 0.001, 0.001});

    return true;
}

static void importPluginAddAsNewTree(const std::string &path,
                                     MainWindow *mainWindow)
{
    Editor &editor = mainWindow->editor();

    Segments segments = editor.segments();
    QueryFilterSet segmentsFilter = editor.segmentsFilter();
    const Datasets datasets = editor.datasets();

    if (datasets.size() < 1)
    {
        return;
    }

    const Dataset &dataset = datasets.at(datasets.size() - 1);

    size_t segmentId = segments.unusedId();

    QueryFilterSet filter;
    filter.setFilter({dataset.id()});
    filter.setEnabled(true);

    QueryWhere where;
    where.setDataset(filter);

    Query query(&editor);
    query.setWhere(where);
    query.exec();

    bool canceled = false;
    int i = 0;
    int maximum = static_cast<int>(dataset.nPoints());
    int j = 0;
    int n = 10 * 1000;

    QProgressDialog progressDialog(mainWindow);
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, maximum);
    progressDialog.setWindowTitle(QObject::tr("Add new tree"));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(0);
    progressDialog.show();

    while (query.next())
    {
        query.segment() = segmentId;
        query.setModified();

        i++;
        if (i > maximum)
        {
            i = maximum;
        }

        j++;
        if (j >= n)
        {
            j = 0;
            progressDialog.setValue(i);

            QCoreApplication::processEvents();
            if (progressDialog.wasCanceled())
            {
                canceled = true;
                break;
            }
        }
    }

    progressDialog.setValue(progressDialog.maximum());

    query.flush();

    if (canceled)
    {
        return;
    }

    segments.addTree(segmentId, File::baseName(path), dataset.boundary());
    segmentsFilter.setEnabled(segmentId, true);

    editor.setSegments(segments);
    editor.setSegmentsFilter(segmentsFilter);
}
