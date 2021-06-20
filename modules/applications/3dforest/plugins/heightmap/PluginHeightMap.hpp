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

/** @file PluginHeightMap.hpp */

#ifndef PLUGIN_HEIGHT_MAP_HPP
#define PLUGIN_HEIGHT_MAP_HPP

#include <EditorFilter.hpp>
#include <PluginTool.hpp>
#include <QDialog>
#include <QMutex>
#include <Vector3.hpp>
#include <vector>

class Editor;
class EditorTile;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QPushButton;
class QCloseEvent;

/** Plugin Height Map Filter.

    This class represents Height Map model.

    An instance of this class is shared memory resource between main window GUI
    thread and editor thread. Most of the code must not be run by multiple
    threads at once. A mutex is used to protect concurent read/write access to
    shared memory.
*/
class PluginHeightMapFilter
{
public:
    PluginHeightMapFilter();
    ~PluginHeightMapFilter() = default;

    void initialize(Editor *editor);
    void setColormap(const QString &name, int colorCount);
    void setPreviewEnabled(bool enabled);
    bool isPreviewEnabled();
    void filterTile(EditorTile *tile);
    void applyToTiles(QWidget *widget);

protected:
    Editor *editor_;
    bool previewEnabled_;
    std::vector<Vector3<float>> colormap_;
    QMutex mutex_;

    std::vector<Vector3<float>> createColormap(const QString &name,
                                               int colorCount);
};

/** Plugin Height Map Window.

    This class represents Height Map GUI as view-conroller for
    PluginHeightMapFilter.

    GUI could be provided directly by the top PluginHeightMap if it was derived
    from QDialog instead of QObject.
*/
class PluginHeightMapWindow : public QDialog
{
    Q_OBJECT

public:
    PluginHeightMapWindow(QWidget *parent, PluginHeightMapFilter *filter);
    ~PluginHeightMapWindow() = default;

protected slots:
    void colorCountChanged(int i);
    void colormapChanged(const QString &name);
    void previewChanged(int index);
    void apply();

protected:
    PluginHeightMapFilter *filter_;
    QSpinBox *colorCountSpinBox_;
    QComboBox *colormapComboBox_;
    QCheckBox *previewCheckBox_;
    QPushButton *applyButton_;

    void closeEvent(QCloseEvent *event) override;
};

/** Plugin Height Map.

    This class represents Height Map plugin as it is visible to the application.

    This plugin provides on-the-fly interactive preview functionality.
    Plugins with interactive preview inherit EditorFilter.
    Plugins with interactive preview feature have more complex design
    compared to solutions which use simple modal progress bar. The reason is
    that interactive preview creates concurent access to memory from multiple
    threads.

    Height Map plugin uses delayed lazy initialization of GUI widgets from
    show() instead of from initialize() to save plugin loading time and memory.
*/
class PluginHeightMap : public QObject, public PluginTool, public EditorFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginTool_iid)
    Q_INTERFACES(PluginTool)

public:
    PluginHeightMap();
    virtual ~PluginHeightMap() = default;

    virtual void initialize(QWidget *parent, Editor *editor);
    virtual void show(QWidget *parent);
    virtual QString windowTitle() const;

    virtual bool isFilterEnabled();
    virtual void filterTile(EditorTile *tile);

protected:
    PluginHeightMapWindow *window_; /**< First time use creates GUI. */
    PluginHeightMapFilter filter_;  /**< Must be created from the constructor.*/
};

#endif /* PLUGIN_HEIGHT_MAP_HPP */
