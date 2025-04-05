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

/** @file ViewerOpenGLViewport.cpp */

// Include 3D Forest.
#include <Editor.hpp>
#include <Geometry.hpp>
#include <Time.hpp>
#include <ViewerOpenGL.hpp>
#include <ViewerOpenGLManager.hpp>
#include <ViewerOpenGLViewport.hpp>
#include <ViewerViewports.hpp>

// Include Qt.
#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>

// Include local.
#define LOG_MODULE_NAME "ViewerOpenGLViewport"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

ViewerOpenGLViewport::ViewerOpenGLViewport(QWidget *parent)
    : QOpenGLWidget(parent),
      manager_(nullptr),
      windowViewports_(nullptr),
      viewportId_(0),
      selected_(false),
      resized_(false),
      editor_(nullptr)

{
    setViewDefault();
}

ViewerOpenGLViewport::~ViewerOpenGLViewport()
{
}

void ViewerOpenGLViewport::paintEvent(QPaintEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Paint event.");
    QOpenGLWidget::paintEvent(event);
}

void ViewerOpenGLViewport::resizeEvent(QResizeEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Resize event.");
    resized_ = true;
    QOpenGLWidget::resizeEvent(event);
}

void ViewerOpenGLViewport::showEvent(QShowEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Show event.");
    QOpenGLWidget::showEvent(event);
}

void ViewerOpenGLViewport::hideEvent(QHideEvent *event)
{
    LOG_DEBUG_QT_EVENT(<< "Hide.");
    QOpenGLWidget::hideEvent(event);
}

void ViewerOpenGLViewport::initializeGL()
{
    LOG_DEBUG_RENDER(<< "Initialize OpenGL.");

    initializeOpenGLFunctions();

    setUpdateBehavior(QOpenGLWidget::PartialUpdate);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0F);
}

void ViewerOpenGLViewport::resizeGL(int w, int h)
{
    LOG_DEBUG_RENDER(<< "Resize w <" << w << "> h <" << h << ">.");

    w = static_cast<int>(parentWidget()->devicePixelRatio() * w);
    h = static_cast<int>(parentWidget()->devicePixelRatio() * h);
    camera_.setViewport(0, 0, w, h);
    camera_.updateProjection();
    // cameraChanged();
}

void ViewerOpenGLViewport::mouseReleaseEvent(QMouseEvent *event)
{
    (void)event;
}

void ViewerOpenGLViewport::mousePressEvent(QMouseEvent *event)
{
    camera_.mousePressEvent(event);
    pickObject(event->pos());
    setFocus();
}

void ViewerOpenGLViewport::mouseMoveEvent(QMouseEvent *event)
{
    camera_.mouseMoveEvent(event);
    cameraChanged();
}

void ViewerOpenGLViewport::wheelEvent(QWheelEvent *event)
{
    camera_.wheelEvent(event);
    setFocus();
    cameraChanged();
}

void ViewerOpenGLViewport::setFocus()
{
    if (!selected())
    {
        if (windowViewports_)
        {
            windowViewports_->selectViewport(this);
        }
    }
}

void ViewerOpenGLViewport::cameraChanged()
{
    if (windowViewports_)
    {
        LOG_DEBUG_RENDER(<< "Emit camera changed.");
        emit windowViewports_->cameraChanged(viewportId_);
    }
}

void ViewerOpenGLViewport::setManager(ViewerOpenGLManager *manager)
{
    manager_ = manager;
}

void ViewerOpenGLViewport::setViewports(ViewerViewports *viewer,
                                        size_t viewportId)
{
    windowViewports_ = viewer;
    viewportId_ = viewportId;
    camera_.setViewportId(viewportId_);
}

size_t ViewerOpenGLViewport::viewportId() const
{
    return viewportId_;
}

void ViewerOpenGLViewport::setSelected(bool selected)
{
    selected_ = selected;
}

bool ViewerOpenGLViewport::selected() const
{
    return selected_;
}

void ViewerOpenGLViewport::updateScene(Editor *editor)
{
    LOG_DEBUG_RENDER(<< "Update viewport <" << viewportId_ << ">.");

    editor_ = editor;
}

void ViewerOpenGLViewport::resetScene(Editor *editor, bool resetView)
{
    LOG_DEBUG_RENDER(<< "Start reseting viewport <" << viewportId_
                     << "> reset view <" << (resetView ? "true" : "false")
                     << ">.");

    editor_ = editor;

    aabb_.set(editor->datasets().boundary());
    LOG_DEBUG_RENDER(<< "Set aabb <" << aabb_ << ">.");

    if (resetView)
    {
        setViewResetCenter();
        setViewResetDistance();
    }

    LOG_DEBUG_RENDER(<< "Finished reseting viewport.");
}

Camera ViewerOpenGLViewport::camera() const
{
    return camera_.toCamera();
}

void ViewerOpenGLViewport::setViewOrthographic()
{
    camera_.setOrthographic();
}

void ViewerOpenGLViewport::setViewPerspective()
{
    camera_.setPerspective();
}

void ViewerOpenGLViewport::setViewDirection(const QVector3D &dir,
                                            const QVector3D &up)
{
    camera_.setLookAt(dir, camera_.distance(), camera_.center(), up);

    LOG_DEBUG(<< "Updated view direction in viewport <" << viewportId_
              << "> to camera <" << camera_ << "> from dir <" << dir << "> up <"
              << up << ">.");
}

void ViewerOpenGLViewport::setViewTop()
{
    LOG_DEBUG(<< "Set top view in viewport <" << viewportId_ << ">.");

    QVector3D dir(0.0F, 0.0F, -1.0F);
    QVector3D up(0.0F, -1.0F, 0.0F);
    setViewDirection(dir, up);
}

void ViewerOpenGLViewport::setViewFront()
{
    LOG_DEBUG(<< "Set front view in viewport <" << viewportId_ << ">.");

    QVector3D dir(0.0F, -1.0F, 0.0F);
    QVector3D up(0.0F, 0.0F, 1.0F);
    setViewDirection(dir, up);
}

void ViewerOpenGLViewport::setViewRight()
{
    LOG_DEBUG(<< "Set right view in viewport <" << viewportId_ << ">.");

    QVector3D dir(1.0F, 0.0F, 0.0F);
    QVector3D up(0.0F, 0.0F, 1.0F);
    setViewDirection(dir, up);
}

void ViewerOpenGLViewport::setView3d()
{
    LOG_DEBUG(<< "Set 3D view in viewport <" << viewportId_ << ">.");

    QVector3D dir(-1.0F, -1.0F, -1.0F);
    QVector3D up(-1.0F, -1.0F, 1.0F);
    dir.normalize();
    up.normalize();
    setViewDirection(dir, up);
}

void ViewerOpenGLViewport::setViewDefault()
{
    LOG_DEBUG(<< "Set default view in viewport <" << viewportId_ << ">.");

    QVector3D dir(-1.0F, -1.0F, -1.0F);
    QVector3D up(-1.0F, -1.0F, 1.0F);
    QVector3D center(0.0F, 0.0F, 0.0F);
    float distance = 1.0F;

    dir.normalize();
    up.normalize();

    camera_.setLookAt(dir, distance, center, up);
}

void ViewerOpenGLViewport::setViewResetDistance()
{
    LOG_DEBUG(<< "Reset view distance in viewport <" << viewportId_ << ">.");

    float distance = 1.0F;
    if (aabb_.valid())
    {
        distance = aabb_.radius() * 2.0F;
    }
    if (distance < 1e-6)
    {
        distance = 1.0F;
    }

    camera_.setLookAt(camera_.direction(),
                      distance,
                      camera_.center(),
                      camera_.up());

    LOG_DEBUG(<< "Updated view distance in viewport <" << viewportId_
              << "> to camera <" << camera_ << "> from distance <" << distance
              << "> aabb <" << aabb_ << ">.");
}

void ViewerOpenGLViewport::setViewResetCenter()
{
    LOG_DEBUG(<< "Reset view center in viewport <" << viewportId_ << ">.");

    QVector3D center = camera_.center();
    if (aabb_.valid())
    {
        center = aabb_.center();
        // center[2] = aabb_.min().z();
    }

    camera_.setOffset(QVector3D(0.0F, 0.0F, 0.0F));
    camera_.setLookAt(camera_.direction(),
                      camera_.distance(),
                      center,
                      camera_.up());

    LOG_DEBUG(<< "Updated view center in viewport <" << viewportId_
              << "> to camera <" << camera_ << "> from aabb <" << aabb_
              << ">.");
}

void ViewerOpenGLViewport::clearScreen()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ViewerOpenGLViewport::paintGL()
{
    LOG_DEBUG_RENDER(<< "Paint width <" << camera_.width() << "> height <"
                     << camera_.height() << ">.");

    // Setup camera.
    glViewport(0, 0, camera_.width(), camera_.height());

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera_.projection().data());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera_.modelView().data());

    // Render.
    renderScene();
}

void ViewerOpenGLViewport::renderScene()
{
    LOG_DEBUG_RENDER(<< "Start rendering viewport <" << viewportId_ << ">.");

    if (!editor_)
    {
        return;
    }

    std::unique_lock<std::mutex> mutexlock(editor_->editorMutex_);

    updateObjects();

    renderSceneSettingsEnable();

    double t1 = Time::realTime();

    size_t pageIndex = 0;
    size_t pageSize = editor_->viewports().pageSize(viewportId_);

    LOG_DEBUG_RENDER(<< "Render pages n <" << pageSize << ">.");

    if (pageSize == 0)
    {
        renderFirstFrame();
    }

    if (resized_)
    {
        LOG_DEBUG_RENDER(<< "Reset render state after resize event.");

        for (size_t i = 0; i < pageSize; i++)
        {
            Page &page = editor_->viewports().page(viewportId_, i);
            if (page.state() == Page::STATE_RENDERED)
            {
                page.setState(Page::STATE_RENDER);
            }
        }

        resized_ = false;
    }

    while (pageIndex < pageSize)
    {
        Page &page = editor_->viewports().page(viewportId_, pageIndex);

        if (page.state() == Page::STATE_RENDER)
        {
            LOG_DEBUG_RENDER(<< "Render page <" << (pageIndex + 1) << "/"
                             << pageSize << "> page id <" << page.pageId()
                             << ">.");

            if (pageIndex == 0)
            {
                renderFirstFrame();
            }

            if (page.renderColor.empty())
            {
                glColor3f(1.0F, 1.0F, 1.0F);
            }

            if (page.selectionSize > 0)
            {
                ViewerOpenGL::render(ViewerOpenGL::POINTS,
                                     page.renderPosition,
                                     page.size(),
                                     page.renderColor.data(),
                                     page.renderColor.size(),
                                     nullptr,
                                     0,
                                     page.selection.data(),
                                     page.selectionSize);
            }

            glFlush();

            page.setState(Page::STATE_RENDERED);

            double t2 = Time::realTime();
            if (t2 - t1 > 0.02)
            {
                LOG_DEBUG_RENDER(<< "Terminate rendering after <" << (t2 - t1)
                                 << "> seconds.");
                break;
            }
        }
        else
        {
            LOG_DEBUG_RENDER(<< "Skip rendering of page <" << (pageIndex + 1)
                             << "/" << pageSize << "> page id <"
                             << page.pageId() << ">.");
        }

        pageIndex++;
    }

    renderSceneSettingsDisable();

    if (pageIndex == pageSize)
    {
        renderLastFrame();
    }

    LOG_DEBUG_RENDER(<< "Finished rendering viewport <" << viewportId_ << ">.");
}

void ViewerOpenGLViewport::renderFirstFrame()
{
    LOG_DEBUG_RENDER(<< "Rendered first frame in viewport <" << viewportId_
                     << ">.");

    clearScreen();

    // Update manager.
    if (!manager_->isInitialized())
    {
        manager_->init();
    }

    manager_->updateResources();

    // Backup gl states.
    if (editor_->settings().viewSettings().distanceBasedFadingVisible())
    {
        glDisable(GL_FOG);
    }

    GLboolean glLightEnabled = SAFE_GL(glIsEnabled(GL_LIGHTING));
    if (glLightEnabled)
    {
        SAFE_GL(glDisable(GL_LIGHTING));
    }

    const Region &clipFilter = editor_->clipFilter();
    glLineWidth(1.0F);
    ViewerOpenGL::renderClipFilter(clipFilter);
    glLineWidth(1.0F);

    renderAttributes();
    renderSegments();

    // Bounding box.
    if (editor_->settings().viewSettings().sceneBoundingBoxVisible())
    {
        glColor3f(0.25F, 0.25F, 0.25F);
        ViewerOpenGL::renderAabbCorners(aabb_);
    }

    SAFE_GL(glPushMatrix());
    renderGuides();
    SAFE_GL(glPopMatrix());

    // recover gl states
    if (glLightEnabled == GL_TRUE)
    {
        SAFE_GL(glEnable(GL_LIGHTING));
    }
    if (editor_->settings().viewSettings().distanceBasedFadingVisible())
    {
        glEnable(GL_FOG);
    }
}

void ViewerOpenGLViewport::renderLastFrame()
{
    glLineWidth(2.0F);
    glDisable(GL_DEPTH_TEST);
    renderLabels();
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1.0F);
}

void ViewerOpenGLViewport::renderSegments()
{
    const Segments &segments = editor_->segments();
    const QueryFilterSet &filter = editor_->segmentsFilter();

    for (size_t i = 0; i < segments.size(); i++)
    {
        const Segment &segment = segments[i];

        // Ignore hidden segments.
        if (!filter.enabled(segment.id))
        {
            continue;
        }

        float r = static_cast<float>(segment.color[0]);
        float g = static_cast<float>(segment.color[1]);
        float b = static_cast<float>(segment.color[2]);

        // Render boundary.
        if (segment.selected)
        {
            glColor3f(r, g, b);
            ViewerAabb boundary;
            boundary.set(segment.boundary);
            ViewerOpenGL::renderAabb(boundary);
        }

        // Ignore "unsegmented".
        if (segment.id == 0)
        {
            continue;
        }

        if (editor_->settings().treeSettings().useOnlyForSelectedTrees() &&
            !segment.selected)
        {
            continue;
        }

        if (editor_->settings().treeSettings().convexHullProjectionVisible())
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            glColor4f(r, g, b, 0.25F);

            // Render meshes.
            for (const auto &it : segment.meshList)
            {
                if (it.first == "convexHullProjection")
                {
                    ViewerOpenGL::render(it.second);
                }
            }

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }

        if (editor_->settings().treeSettings().convexHullVisible())
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);
            glColor4f(r, g, b, 0.25F);

            // Render meshes.
            for (const auto &it : segment.meshList)
            {
                if (it.first == "convexHull")
                {
                    ViewerOpenGL::render(it.second);
                }
            }

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }
    }
}

void ViewerOpenGLViewport::renderAttributes()
{
    const Segments &segments = editor_->segments();

    for (size_t i = 0; i < segments.size(); i++)
    {
        const Segment &segment = segments[i];

        if (skipSegmentRendering(segment))
        {
            continue;
        }

        // Render attributes.
        const TreeAttributes &attributes = segment.treeAttributes;

        glColor3f(1.0F, 1.0F, 0.0F);

        if (attributes.isDbhValid())
        {
            Vector3<float> treeDbhPosition(attributes.dbhPosition);
            float treeDbhRadius = static_cast<float>(attributes.dbh) * 0.5F;
            ViewerOpenGL::renderCircle(treeDbhPosition, treeDbhRadius);
        }

        if (attributes.isPositionValid())
        {
            Vector3<float> treePosition = attributes.position;

            if (attributes.isHeightValid())
            {
                Vector3<float> treeTip(
                    treePosition[0],
                    treePosition[1],
                    treePosition[2] + static_cast<float>(attributes.height));

                ViewerOpenGL::renderLine(treePosition, treeTip);

                if (editor_->settings().treeSettings().treePosition() ==
                    TreeSettings::Position::TOP)
                {
                    treePosition[2] += static_cast<float>(attributes.height);
                }
            }

            ViewerOpenGL::renderCross(
                treePosition,
                static_cast<float>(segment.boundary.length(0)),
                static_cast<float>(segment.boundary.length(1)));
        }
    }
}

void ViewerOpenGLViewport::renderLabels()
{
    const Segments &segments = editor_->segments();

    for (size_t i = 0; i < segments.size(); i++)
    {
        const Segment &segment = segments[i];

        if (skipSegmentRendering(segment))
        {
            continue;
        }

        glColor3f(1.0F, 1.0F, 0.0F);

        ViewerAabb boundary;
        boundary.set(segment.boundary);
        Vector3<float> treePosition;
        treePosition[0] = boundary.center().x();
        treePosition[1] = boundary.center().y();
        treePosition[2] = boundary.max().z();

        ViewerOpenGL::renderText(manager_,
                                 camera_,
                                 treePosition,
                                 segment.label,
                                 0.05F * camera_.distance());
    }
}

bool ViewerOpenGLViewport::skipSegmentRendering(const Segment &segment)
{
    // Do not render any attributes.
    if (!editor_->settings().treeSettings().treeAttributesVisible())
    {
        return true;
    }

    // Ignore "unsegmented".
    if (segment.id == 0)
    {
        return true;
    }

    // Render only selected trees.
    if (editor_->settings().treeSettings().useOnlyForSelectedTrees() &&
        !segment.selected)
    {
        return true;
    }

    // Ignore hidden segments.
    const QueryFilterSet &filter = editor_->segmentsFilter();
    if (!filter.enabled(segment.id))
    {
        return true;
    }

    return false;
}

void ViewerOpenGLViewport::renderGuides()
{
    // Overlay.
    QMatrix4x4 m;
    float w = static_cast<float>(camera_.width());
    float h = static_cast<float>(camera_.height());

    m.ortho(-w * 0.5F, w * 0.5F, -h * 0.5F, h * 0.5F, -50.0F, 50.0F);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m.data());

    // Overlay: rotated.
    float axesSize = 30.0F;
    m.setToIdentity();
    m.translate(-w * 0.5F + axesSize + 2.0F, -h * 0.5F + axesSize + 2.0F);
    m.scale(axesSize);
    m = m * camera_.rotation();

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m.data());

    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0F);
    ViewerOpenGL::renderAxis();
    glLineWidth(1.0F);
    glEnable(GL_DEPTH_TEST);

    // Restore matrix.
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera_.projection().data());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera_.modelView().data());
}

void ViewerOpenGLViewport::renderSceneSettingsEnable()
{
    const ViewSettings &opt = editor_->settings().viewSettings();

    // Background.
    const Vector3<double> &rgb = opt.backgroundColor();
    glClearColor(static_cast<float>(rgb[0]),
                 static_cast<float>(rgb[1]),
                 static_cast<float>(rgb[2]),
                 1.0F);

    // Point size.
    glPointSize(static_cast<float>(opt.pointSize()));

    // Fog.
    if (opt.distanceBasedFadingVisible())
    {
        QVector3D eye = camera_.eye();
        QVector3D direction = -camera_.direction();
        direction.normalize();

        float min;
        float max;
        aabb_.range(eye, direction, &min, &max);
        float d = max - min;

        GLfloat colorFog[4]{0.0F, 0.0F, 0.0F, 0.0F};
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogfv(GL_FOG_COLOR, colorFog);
        glHint(GL_FOG_HINT, GL_DONT_CARE);
        glFogf(GL_FOG_START, min);
        glFogf(GL_FOG_END, max + d * 0.1F);
        glEnable(GL_FOG);
    }
}

void ViewerOpenGLViewport::renderSceneSettingsDisable()
{
    glPointSize(1.0F);

    if (editor_->settings().viewSettings().distanceBasedFadingVisible())
    {
        glDisable(GL_FOG);
    }
}

void ViewerOpenGLViewport::updateObjects()
{
    objects_.clear();

    const Segments &segments = editor_->segments();
    const QueryFilterSet &filter = editor_->segmentsFilter();

    for (size_t i = 0; i < segments.size(); i++)
    {
        const Segment &segment = segments[i];

        if (segment.id == 0)
        {
            continue;
        }

        if (!filter.enabled(segment.id))
        {
            continue;
        }

        Object obj;
        obj.id = segment.id;
        obj.aabb.set(segment.boundary);

        objects_.push_back(std::move(obj));
    }
}

void ViewerOpenGLViewport::pickObject(const QPoint &p)
{
    LOG_DEBUG(<< "Pick x <" << p.x() << "> y <" << p.y() << ">.");

    selectedId = 0;

    size_t nearId = 0;
    float dist = Numeric::max<float>();

    QVector3D p1;
    QVector3D p2;

    camera_.ray(p.x(), p.y(), &p1, &p2);

    for (size_t i = 0; i < objects_.size(); i++)
    {
        const QVector3D &min = objects_[i].aabb.min();
        const QVector3D &max = objects_[i].aabb.max();
        float x;
        float y;
        float z;
        bool b = intersectSegmentAABB(p1.x(),
                                      p1.y(),
                                      p1.z(),
                                      p2.x(),
                                      p2.y(),
                                      p2.z(),
                                      min.x(),
                                      min.y(),
                                      min.z(),
                                      max.x(),
                                      max.y(),
                                      max.z(),
                                      x,
                                      y,
                                      z);
        if (b)
        {
            float d = distance(x, y, z, p1.x(), p1.y(), p1.z());
            if (d < dist)
            {
                LOG_DEBUG(<< "d <" << d << "> < dist <" << dist << "> ID <"
                          << objects_[i].id << ">.");
                dist = d;
                nearId = objects_[i].id;
            }
        }
    }

    selectedId = nearId;
    LOG_DEBUG(<< "Selected ID <" << selectedId << ">.");
}
