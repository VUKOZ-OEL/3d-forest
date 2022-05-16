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
#include <GuiProjectPlugin.hpp>
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
#include <QToolButton>

#define LOG_LOCAL(msg)
//#define LOG_LOCAL(msg) LOG_MODULE("GuiWindowMain", msg)
#define GUI_ICON_SIZE_TOOL_BAR 24

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
    guiPluginImport_ = new GuiPluginImport(this);
    guiPluginViewer_ = new GuiPluginViewer(this);
    guiPluginProject_ = new GuiProjectPlugin(this);
    loadPlugins();

    // Exit
    createAction(&actionExit_,
                 "File",
                 "",
                 tr("E&xit"),
                 tr("Exit the application"),
                 QIcon(),
                 this,
                 SLOT(close()));
    actionExit_->setShortcuts(QKeySequence::Quit);

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

    updateEverything();
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
                                 const QString &toolBar,
                                 const QString &text,
                                 const QString &toolTip,
                                 const QIcon &icon,
                                 const QObject *receiver,
                                 const char *member)
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
    else if (!toolBar_.contains(toolBar))
    {
        createMenuSeparator(menu);
    }
    menu_[menu]->addAction(action);

    if (!toolBar.isEmpty() && !icon.isNull())
    {
        if (!toolBar_.contains(toolBar))
        {
            toolBar_[toolBar] = addToolBar(toolBar);
            toolBar_[toolBar]->setIconSize(
                QSize(GUI_ICON_SIZE_TOOL_BAR, GUI_ICON_SIZE_TOOL_BAR));
        }
        toolBar_[toolBar]->addAction(action);
    }

    // Optional return value for further customization of new action
    if (result)
    {
        *result = action;
    }
}

void GuiWindowMain::createToolButton(QToolButton **result,
                                     const QString &text,
                                     const QString &toolTip,
                                     const QIcon &icon,
                                     const QObject *receiver,
                                     const char *member)
{
    QToolButton *button;

    // Create button
    button = new QToolButton;
    button->setText(text);
    button->setToolTip(toolTip);
    button->setStatusTip(toolTip);
    button->setIcon(icon);
    button->setEnabled(true);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // Connect button
    if (receiver && member)
    {
        connect(button, SIGNAL(clicked()), receiver, member);
    }

    // Return value
    *result = button;
}

void GuiWindowMain::createMenuSeparator(const QString &menu)
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

void GuiWindowMain::suspendThreads()
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

    suspendThreads();

    GuiViewports *viewports = guiPluginViewer_->viewports();

    editor_.lock();
    viewports->resetScene(&editor_, true);
    editor_.unlock();

    emit signalUpdate();

    size_t viewportId = viewports->selectedViewportId();
    threadRender_.render(viewportId, viewports->camera(viewportId));

    setWindowTitle(QString::fromStdString(editor_.projectPath()));
}

void GuiWindowMain::updateData()
{
    suspendThreads();

    GuiViewports *viewports = guiPluginViewer_->viewports();
    viewports->resetScene(&editor_, false);
    editor_.viewports().clearContent();

    resumeThreads();
}

void GuiWindowMain::updateSelection()
{
    suspendThreads();

    GuiViewports *viewports = guiPluginViewer_->viewports();
    viewports->resetScene(&editor_, false);
    editor_.viewports().setState(EditorPage::STATE_SELECT);

    resumeThreads();
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
