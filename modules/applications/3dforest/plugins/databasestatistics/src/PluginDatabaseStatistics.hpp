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
#include <QDialog>

class Editor;
class QTextEdit;
class QPushButton;

/** Plugin Database Statistics Window. */
class PluginDatabaseStatisticsWindow : public QDialog
{
    Q_OBJECT

public:
    PluginDatabaseStatisticsWindow(QWidget *parent, Editor *editor);

protected slots:
    void compute();

protected:
    Editor *editor_;
    QTextEdit *textEdit_;
    QPushButton *computeButton_;
};

/** Plugin Database Statistics. */
class PluginDatabaseStatistics : public QObject, public PluginTool
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginTool_iid)
    Q_INTERFACES(PluginTool)

public:
    PluginDatabaseStatistics();

    virtual void initialize(QWidget *parent, Editor *editor);
    virtual void show(QWidget *parent);
    virtual QString windowTitle() const;
    virtual QString toolTip() const;
    virtual QIcon icon() const;

protected:
    PluginDatabaseStatisticsWindow *window_;
    Editor *editor_;
};

#endif /* PLUGIN_DATABASE_STATISTICS_HPP */
