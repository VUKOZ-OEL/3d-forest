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

/** @file PluginDatabaseStatistics.hpp */

#ifndef PLUGIN_DATABASE_STATISTICS_HPP
#define PLUGIN_DATABASE_STATISTICS_HPP

#include <PluginTool.hpp>
#include <WindowDock.hpp>

class Editor;
class EditorTile;
class QTextEdit;
class QPushButton;

/** Plugin Database Statistics Window. */
class PluginDatabaseStatisticsWindow : public WindowDock
{
    Q_OBJECT

public:
    PluginDatabaseStatisticsWindow(QMainWindow *parent, Editor *editor);

protected slots:
    void compute();

protected:
    Editor *editor_;
    QWidget *widget_;
    QTextEdit *textEdit_;
    QPushButton *computeButton_;

    // Stats
    uint64_t numberOfPoints_;
    uint64_t classificationPoints_;
    uint32_t classificationMaximum_;

    void computeReset();
    void computeStep(EditorTile *tile);
    void computeOutput();
};

/** Plugin Database Statistics. */
class PluginDatabaseStatistics : public QObject, public PluginTool
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginTool_iid)
    Q_INTERFACES(PluginTool)

public:
    PluginDatabaseStatistics();

    virtual void initialize(QMainWindow *parent, Editor *editor);
    virtual void show(QMainWindow *parent);
    virtual QAction *toggleViewAction() const;
    virtual QString windowTitle() const;
    virtual QString buttonText() const;
    virtual QString toolTip() const;
    virtual QIcon icon() const;

protected:
    PluginDatabaseStatisticsWindow *window_;
    Editor *editor_;
};

#endif /* PLUGIN_DATABASE_STATISTICS_HPP */
