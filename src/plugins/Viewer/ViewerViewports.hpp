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

/** @file ViewerViewports.hpp */

#ifndef VIEWER_VIEWPORTS_HPP
#define VIEWER_VIEWPORTS_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <Camera.hpp>
class Editor;
class ViewerOpenGLViewport;
class ViewerOpenGLManager;

// Include Qt.
#include <QWidget>

// Include local.
#include <ExportGui.hpp>

/** Viewer Viewports. */
class ViewerViewports : public QWidget
{
    Q_OBJECT

public:
    enum ViewLayout
    {
        VIEW_LAYOUT_SINGLE,
        VIEW_LAYOUT_TWO_COLUMNS,
        VIEW_LAYOUT_GRID,
        VIEW_LAYOUT_THREE_ROWS_RIGHT
    };

    explicit ViewerViewports(QWidget *parent = nullptr);
    ~ViewerViewports();

    void setLayout(ViewLayout viewLayout);

    void setViewOrthographic();
    void setViewPerspective();
    void setViewTop();
    void setViewFront();
    void setViewRight();
    void setView3d();
    void setViewResetDistance();
    void setViewResetCenter();

    void selectViewport(ViewerOpenGLViewport *viewport);
    size_t selectedViewportId() const;
    std::vector<Camera> camera(size_t viewportId) const;
    std::vector<Camera> camera() const;

    void updateScene(Editor *editor);
    void resetScene(Editor *editor, bool resetView);
    void resetViewport(Editor *editor, size_t viewportId, bool resetView);

signals:
    void cameraChanged(size_t viewportId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    std::vector<ViewerOpenGLViewport *> viewports_;
    std::shared_ptr<ViewerOpenGLManager> manager_;

    void initializeViewports();
    ViewerOpenGLViewport *createViewport(size_t viewportId);
    ViewerOpenGLViewport *selectedViewport();
    const ViewerOpenGLViewport *selectedViewport() const;
};

inline std::ostream &operator<<(std::ostream &out,
                                const ViewerViewports::ViewLayout &in)
{
    switch (in)
    {
        case ViewerViewports::VIEW_LAYOUT_SINGLE:
            out << "SINGLE";
            break;
        case ViewerViewports::VIEW_LAYOUT_TWO_COLUMNS:
            out << "TWO_COLUMNS";
            break;
        case ViewerViewports::VIEW_LAYOUT_GRID:
            out << "GRID";
            break;
        case ViewerViewports::VIEW_LAYOUT_THREE_ROWS_RIGHT:
            out << "THREE_ROWS_RIGHT";
            break;
        default:
            out << "UNKNOWN";
            break;
    }

    return out;
}

#endif /* VIEWER_VIEWPORTS_HPP */
