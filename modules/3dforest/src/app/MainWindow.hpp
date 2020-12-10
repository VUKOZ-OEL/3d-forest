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
    @file MainWindow.hpp
*/

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <Editor.hpp>
#include <Explorer.hpp>
#include <Project.hpp>
#include <QMainWindow>
#include <Viewer.hpp>
#include <vector>

/** Main Window. */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const QString APPLICATION_NAME;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

public slots:
    void slotOpenProject();
    void slotViewportLayoutSingle();
    void slotViewportLayoutTwoColumns();
    void slotExplorerItemDoubleClicked(QTreeWidgetItem *item, int column);

protected:
    Project project_;
    Editor editor_;
    Explorer *explorer_;
    Viewer *viewer_;

    int timerNewData_;
    QMenu *menuView;

    // Initialization
    void initializeWindow();
    void createMenus();
    void createDockWindows();

    // Project
    void openProject(const QString &path);
    void closeProject();
    void openFile(const QString &path);
    void closeFile();

    // Utilities
    void showError(const char *message);
    void updateWindowTitle(const QString &path);

    // Events
    void timerEvent(QTimerEvent *e) override;
};

#endif /* MAIN_WINDOW_HPP */
