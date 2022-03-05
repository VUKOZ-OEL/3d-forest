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

/** @file PluginClassification.hpp */

#ifndef PLUGIN_CLASSIFICATION_HPP
#define PLUGIN_CLASSIFICATION_HPP

#include <PluginTool.hpp>

class PluginClassificationWindow;

/** Plugin Classification. */
class PluginClassification : public QObject, public PluginTool
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginTool_iid)
    Q_INTERFACES(PluginTool)

public:
    PluginClassification();

    virtual void initialize(QMainWindow *parent, Editor *editor);
    virtual void show(QMainWindow *parent);
    virtual QAction *toggleViewAction() const;
    virtual QString windowTitle() const;
    virtual QString buttonText() const;
    virtual QString toolTip() const;
    virtual QPixmap icon() const;

protected:
    PluginClassificationWindow *window_;
    Editor *editor_;
};

#endif /* PLUGIN_CLASSIFICATION_HPP */
