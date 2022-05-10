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

/** @file GuiWindowMain.cpp */

#include <Log.hpp>

#include <GuiPluginImport.hpp>
#include <GuiPluginInterface.hpp>
#include <GuiPluginProjectFile.hpp>
#include <GuiPluginViewer.hpp>
#include <GuiViewports.hpp>
#include <GuiWindowMain.hpp>

#include <QCloseEvent>
#include <QCoreApplication>
#include <QDir>
#include <QMenuBar>
#include <QMessageBox>
#include <QPluginLoader>
#include <QStatusBar>
#include <QToolBar>

#define LOG_LOCAL(msg)
//#define LOG_LOCAL(msg) LOG_MODULE("GuiWindowMain", msg)

const char *GuiWindowMain::APPLICATION_NAME = "3DForest";
const char *GuiWindowMain::APPLICATION_VERSION = "1.0";

GuiWindowMain::GuiWindowMain(QWidget *parent)
    : QMainWindow(parent),
      threadRender_(&editor_)
{
    LOG_LOCAL("");

    // Status bar
    statusBar()->showMessage(tr("Ready"));

    // Menu
    guiPluginProjectFile_ = new GuiPluginProjectFile(this);
    createSeparator("File");
    guiPluginImport_ = new GuiPluginImport(this);
    guiPluginViewer_ = new GuiPluginViewer(this);
    loadPlugins();

    // Exit
    createSeparator("File");
    createAction(&actionExit_,
                 "File",
                 tr("E&xit"),
                 tr("Exit the application"),
                 QIcon(),
                 this,
                 SLOT(close()),
                 false);
    actionExit_->setShortcuts(QKeySequence::Quit);

    hideToolBar("File");

    // Rendering
    connect(guiPluginViewer_->viewports(),
            SIGNAL(cameraChanged(size_t)),
            this,
            SLOT(slotRenderViewport(size_t)));

    connect(this,
            SIGNAL(signalRender()),
            this,
            SLOT(slotRender()),
            Qt::QueuedConnection);

    threadRender_.setCallback(this);
    threadRender_.create();
}

GuiWindowMain::~GuiWindowMain()
{
    LOG_LOCAL("");
    threadRender_.stop();
}

QSize GuiWindowMain::minimumSizeHint() const
{
    return QSize(320, 200);
}

QSize GuiWindowMain::sizeHint() const
{
    return QSize(1024, 768);
}

void GuiWindowMain::showError(const char *message)
{
    (void)QMessageBox::critical(this, tr("Error"), message);
}

void GuiWindowMain::setWindowTitle(const QString &path)
{
    QString newtitle = APPLICATION_NAME;

    if (path != "")
    {
        newtitle = newtitle + " - " + path;
    }

    QMainWindow::setWindowTitle(newtitle + " [*]");
}

void GuiWindowMain::createAction(QAction **result,
                                 const QString &menu,
                                 const QString &text,
                                 const QString &toolTip,
                                 const QIcon &icon,
                                 const QObject *receiver,
                                 const char *member,
                                 bool useToolBar)
{
    QAction *action;

    // Create action
    action = new QAction(text, this);

    if (!toolTip.isEmpty())
    {
        action->setToolTip(toolTip);
        action->setStatusTip(toolTip);
    }

    if (!icon.isNull())
    {
        action->setIcon(icon);
    }

    // Connect action
    if (receiver && member)
    {
        connect(action, SIGNAL(triggered()), receiver, member);
    }

    // Add action to menu
    if (!menu_.contains(menu))
    {
        menu_[menu] = menuBar()->addMenu(menu);
    }
    menu_[menu]->addAction(action);

    if (useToolBar && !icon.isNull())
    {
        if (!toolBar_.contains(menu))
        {
            toolBar_[menu] = addToolBar(menu);
            toolBar_[menu]->setIconSize(
                QSize(GUI_ICON_SIZE_TOOL_BAR, GUI_ICON_SIZE_TOOL_BAR));
        }
        toolBar_[menu]->addAction(action);
    }

    // Optional return value for further customization of new action
    if (result)
    {
        *result = action;
    }
}

void GuiWindowMain::createSeparator(const QString &menu)
{
    menu_[menu]->addSeparator();
}

void GuiWindowMain::hideToolBar(const QString &menu)
{
    if (toolBar_.contains(menu))
    {
        toolBar_[menu]->close();
    }
}

void GuiWindowMain::loadPlugins()
{
    // Process all files in the application "exe" directory
    QDir pluginsDir(QCoreApplication::applicationDirPath() + "/plugins/");
    const QStringList entries = pluginsDir.entryList(QDir::Files);

    for (const QString &fileName : entries)
    {
        // Try to load the file as a plugin
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();

        loadPlugin(plugin);
    }
}

void GuiWindowMain::loadPlugin(QObject *plugin)
{
    if (!plugin)
    {
        return;
    }

    // Detect and register various plugins

    GuiPluginInterface *guiPluginInterface;
    guiPluginInterface = qobject_cast<GuiPluginInterface *>(plugin);
    if (guiPluginInterface)
    {
        guiPluginInterface->initialize(this);
        plugins_.push_back(guiPluginInterface);

        // EditorProcessorInterface *processor;
        // processor = dynamic_cast<EditorProcessorInterface
        // *>(guiPluginInterface); if (processor)
        // {
        //     editor_.addFilter(processor);
        // }

        return;
    }
}

void GuiWindowMain::cancelThreads()
{
    LOG_LOCAL("");
    threadRender_.cancel();
}

void GuiWindowMain::resumeThreads()
{
    LOG_LOCAL("");
    slotRenderViewport();
}

void GuiWindowMain::threadProgress(bool finished)
{
    (void)finished;
    LOG_LOCAL("finished=" << finished);
    emit signalRender();
}

void GuiWindowMain::slotRender()
{
    LOG_LOCAL("");
    editor_.lock();
    guiPluginViewer_->viewports()->updateScene(&editor_);
    editor_.unlock();
}

void GuiWindowMain::slotRenderViewport()
{
    LOG_LOCAL("");
    slotRenderViewport(guiPluginViewer_->viewports()->selectedViewportId());
}

void GuiWindowMain::slotRenderViewport(size_t viewportId)
{
    LOG_LOCAL("");
    GuiViewports *viewports = guiPluginViewer_->viewports();
    threadRender_.render(viewportId, viewports->camera(viewportId));
}

void GuiWindowMain::updateEverything()
{
    LOG_LOCAL("");

    cancelThreads();

    GuiViewports *viewports = guiPluginViewer_->viewports();

    editor_.lock();
    viewports->resetScene(&editor_, true);
    editor_.unlock();

    size_t viewportId = viewports->selectedViewportId();
    threadRender_.render(viewportId, viewports->camera(viewportId));

    setWindowTitle(QString::fromStdString(editor_.projectPath()));
}

void GuiWindowMain::closeEvent(QCloseEvent *event)
{
    if (guiPluginProjectFile_->projectClose())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}
