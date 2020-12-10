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

/**
    @file MainWindow.cpp
*/

#include <MainWindow.hpp>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>

const QString MainWindow::APPLICATION_NAME = "3DForest";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    initializeWindow();
}

MainWindow::~MainWindow()
{
    killTimer(timerNewData_);
}

QSize MainWindow::minimumSizeHint() const
{
    return QSize(320, 200);
}

QSize MainWindow::sizeHint() const
{
    return QSize(640, 480);
}

void MainWindow::initializeWindow()
{
    // Menu
    createMenus();

    // 3D Viewer
    viewer_ = new Viewer(this);
    setCentralWidget(viewer_);

    // Docks
    createDockWindows();

    // Initial project
    //openProject("project.json");
    explorer_->updateProject(project_);

    // Timers
    timerNewData_ = startTimer(1000);
}

void MainWindow::createMenus()
{
    // File
    QMenu *menuFile = menuBar()->addMenu(tr("File"));
    menuFile->addAction(tr("Open Project..."),
                        this,
                        &MainWindow::slotOpenProject);
    menuFile->addAction(tr("Exit"), this, &MainWindow::close);

    // View
    menuView = menuBar()->addMenu(tr("View"));
    QMenu *menuViewportLayout = menuView->addMenu(tr("Viewport Layout"));
    menuViewportLayout->addAction(tr("Single"),
                                  this,
                                  &MainWindow::slotViewportLayoutSingle);
    menuViewportLayout->addAction(tr("Two Columns"),
                                  this,
                                  &MainWindow::slotViewportLayoutTwoColumns);
}

void MainWindow::createDockWindows()
{
    // Explorer
    explorer_ = new Explorer(this);
    connect(explorer_,
            SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            this,
            SLOT(slotExplorerItemDoubleClicked(QTreeWidgetItem *, int)));

    QDockWidget *dockWidget = new QDockWidget(tr("Explorer"), this);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea);
    dockWidget->setMinimumWidth(50);
    dockWidget->setMaximumWidth(500);
    dockWidget->setWidget(explorer_);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    menuView->addAction(dockWidget->toggleViewAction());
}

void MainWindow::slotOpenProject()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open Project"),
                                            "",
                                            tr("3DForest Project (*.json)"));
    if (fileName.isEmpty())
    {
        return;
    }

    openProject(fileName);
}

void MainWindow::slotViewportLayoutSingle()
{
    viewer_->setViewportLayout(Viewer::VIEWPORT_LAYOUT_SINGLE);
}

void MainWindow::slotViewportLayoutTwoColumns()
{
    viewer_->setViewportLayout(Viewer::VIEWPORT_LAYOUT_TWO_COLUMNS);
    const std::vector<std::shared_ptr<Node>> &scene = editor_.getScene();
    viewer_->update(scene);
}

void MainWindow::slotExplorerItemDoubleClicked(QTreeWidgetItem *item,
                                               int column)
{
    (void)column;
    openFile(item->text(Explorer::COLUMN_PATH));
}

void MainWindow::openProject(const QString &path)
{
    closeProject();
    project_.open(path.toStdString());
    explorer_->updateProject(project_);
}

void MainWindow::closeProject()
{
    explorer_->clear();
    project_.close();
}

void MainWindow::openFile(const QString &path)
{
    closeFile();

    try
    {
        editor_.open(path.toStdString());
        // project_.load();
    }
    catch (std::exception &e)
    {
        showError(e.what());
        return;
    }

    updateWindowTitle(path);

    const std::vector<std::shared_ptr<Node>> &scene = editor_.getScene();
    viewer_->update(scene);
}

void MainWindow::closeFile()
{
    updateWindowTitle("");
    editor_.close();
}

void MainWindow::showError(const char *message)
{
    (void)QMessageBox::critical(this, tr("Error"), message);
}

void MainWindow::updateWindowTitle(const QString &path)
{
    if (path == "")
    {
        setWindowTitle(APPLICATION_NAME);
    }
    else
    {
        QString newtitle = APPLICATION_NAME + " - " + path;
        setWindowTitle(newtitle);
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    (void)event;
}
