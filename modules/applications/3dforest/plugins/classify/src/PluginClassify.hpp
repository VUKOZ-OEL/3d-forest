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

/** @file PluginClassify.hpp */

#ifndef PLUGIN_CLASSIFY_HPP
#define PLUGIN_CLASSIFY_HPP

#include <EditorFilter.hpp>
#include <PluginTool.hpp>
#include <QMutex>
#include <Vector3.hpp>
#include <WindowDock.hpp>
#include <vector>

class Editor;
class EditorPage;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;
class QCloseEvent;

/** Plugin Classify Window. */
class PluginClassifyWindow : public WindowDock
{
    Q_OBJECT

public:
    PluginClassifyWindow(QMainWindow *parent, Editor *editor);
    ~PluginClassifyWindow() = default;

protected slots:
    void compute();

protected:
    Editor *editor_;
    QWidget *widget_;
    QPushButton *computeButton_;
};

/** Plugin Classify. */
class PluginClassify : public QObject, public PluginTool
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginTool_iid)
    Q_INTERFACES(PluginTool)

public:
    PluginClassify();

    virtual void initialize(QMainWindow *parent, Editor *editor);
    virtual void show(QMainWindow *parent);
    virtual QAction *toggleViewAction() const;
    virtual QString windowTitle() const;
    virtual QString buttonText() const;
    virtual QString toolTip() const;
    virtual QPixmap icon() const;

protected:
    PluginClassifyWindow *window_;
    Editor *editor_;
};

#endif /* PLUGIN_CLASSIFY_HPP */
