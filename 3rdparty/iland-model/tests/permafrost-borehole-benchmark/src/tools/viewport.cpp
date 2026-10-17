/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/
#include "global.h"
#include "viewport.h"


/** @class Viewport
  Handles coordinaive transforation between grids (based on real-world metric coordinates).
  The visible part of the grid is defined by the "viewport" (defaults to 100% of the grid).
  The result coordinates are mapped into a "ScreenRect", which is a pixel-based viewing window.
*/

/// toWorld() converts the pixel-information (e.g. by an mouse event) to the corresponding real world coordinates (defined by viewport).
const QPointF Viewport::toWorld(const QPoint pixel)
{
    QPointF p;
    p.setX( pixel.x()/m_scale_worldtoscreen +  m_delta_worldtoscreen.x());
    p.setY( (m_screen.height() - pixel.y()  )/m_scale_worldtoscreen + m_delta_worldtoscreen.y() );
    return p;

}

/// toScreen() converts world coordinates in screen coordinates using the defined viewport.
const QPoint Viewport::toScreen(const QPointF p)
{
    QPoint pixel;
    pixel.setX( qRound( (p.x()-m_delta_worldtoscreen.x())* m_scale_worldtoscreen ) );
    pixel.setY( m_screen.height() - 1 -  qRound( (p.y()-m_delta_worldtoscreen.y() ) * m_scale_worldtoscreen ));
    return pixel;
}

/// sets the screen rect; this also modifies the viewport.
void Viewport::setScreenRect(const QRect &viewrect)
{
    if (m_screen!=viewrect) {
        m_screen = viewrect;
        m_viewport = viewrect;
        zoomToAll();
    }
}

/// show the full extent of the world.
void Viewport::zoomToAll()
{
    // calculate move/scale so that world-rect maps entirely onto screen
    double scale_x = m_screen.width() /  m_world.width(); // pixel per meter in x
    double scale_y = m_screen.height() / m_world.height(); // pixel per meter in y
    double scale = qMin(scale_x, scale_y);
    QPointF d;
    if (scale_x < scale_y) {
        // x-axis fills the screen; center in y-axis
        d.setX(m_world.left());
        int py_mid = m_screen.height()/2;
        double world_mid = m_world.center().y();
        d.setY( world_mid - py_mid/scale );
    } else {
        d.setY(m_world.top());
        int px_mid = m_screen.width()/2;
        double world_mid = m_world.center().x();
        d.setX( world_mid - px_mid/scale );
    }
    m_delta_worldtoscreen = d;
    m_scale_worldtoscreen = scale;
    m_viewport.setBottomLeft(toWorld(m_screen.topLeft()));
    m_viewport.setTopRight(toWorld(m_screen.bottomRight()));
}

/// zoom using a factor of @p factor. Values > 1 means zoom out, < 1 zoom in. (factor=1 would have no effect).
/// after zooming, the world-point under the mouse @p screen_point is still under the mouse.
void Viewport::zoomTo(const QPoint &screen_point, const double factor)
{
    QPointF focus_point = toWorld(screen_point); // point under the mouse

    m_viewport.setWidth(m_viewport.width() * factor);
    m_viewport.setHeight(m_viewport.height() * factor);

    m_scale_worldtoscreen /= factor;

    // get scale/delta
    QPointF new_focus = toWorld(screen_point);
    m_delta_worldtoscreen -= (new_focus - focus_point);

    m_viewport.setBottomLeft(toWorld(m_screen.topLeft()));
    m_viewport.setTopRight(toWorld(m_screen.bottomRight()));

    //qDebug() <<"oldf"<< new_focus << "newf" << focus_point << "m_delta" << m_delta_worldtoscreen << "m_scale:" << m_scale_worldtoscreen << "viewport:"<<m_viewport;
}

/// move the viewport. @p screen_from and @p screen_to give mouse positions (in pixel) from dragging the mouse.
void Viewport::moveTo(const QPoint &screen_from, const QPoint &screen_to)
{
    QPointF p1 = toWorld(screen_from);
    QPointF p2 = toWorld(screen_to);
    m_delta_worldtoscreen -= (p2-p1);
    // correct the viewport
    m_viewport.setBottomLeft(toWorld(m_screen.topLeft()));
    m_viewport.setTopRight(toWorld(m_screen.bottomRight()));
}

/// set 'world_center' as the new center point of the viewport
void Viewport::setViewPoint(const QPointF &world_center, const double px_per_meter)
{
    QPoint p = toScreen(world_center); // point where world_center would be
    QPoint target = m_screen.center();
    moveTo(p,target);
    double px_p_m = qMax(px_per_meter, 0.001);
    double factor =  m_scale_worldtoscreen / px_p_m;
    zoomTo(target, factor);
}

bool Viewport::isVisible(const QPointF &world_coord) const
{
    return m_viewport.contains(world_coord);
}
bool Viewport::isVisible(const QRectF &world_rect) const
{
    return m_viewport.contains(world_rect)
            || m_viewport.intersects(world_rect);
}

