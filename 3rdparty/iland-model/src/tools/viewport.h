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
#ifndef VIEWPORT_H
#define VIEWPORT_H
#include <QtGui>

class Viewport
{
public:
    Viewport(): m_viewAll(true), m_scale_worldtoscreen(1.) {}
    Viewport(const QRectF worldrect, const QRect screenrect) { setWorldRect(worldrect); setScreenRect(screenrect); zoomToAll(); }
    // coordinate transformations
    const QPointF toWorld(const QPoint pixel);
    const QPoint toScreen(const QPointF p);
    const QRect toScreen(const QRectF world) { QPoint p1=toScreen(world.bottomLeft()); QPoint p2=toScreen(world.topRight()); QRect r(p1, QSize(p2.x()-p1.x()+1, p2.y()-p1.y()+1)); return r; }
    // getters
    const QRectF viewRect() const { return m_viewport; }
    bool isVisible(const QPointF &world_coord) const;
    bool isVisible(const QRectF &world_rect) const;
    // zoom
    void zoomToAll();
    void zoomTo(const QPoint &screen_point, const double factor);
    void moveTo(const QPoint &screen_from, const QPoint &screen_to);
    // move
    void setViewPoint(const QPointF &world_center, const double px_per_meter);
    // conversion of length
    double pixelToMeter(const int pixel) { return pixel/m_scale_worldtoscreen; }
    int meterToPixel(const double meter) { return qRound(meter * m_scale_worldtoscreen);}
    // setters...
    void setViewRect(const QRectF &viewrect) { m_viewport = viewrect; }
    void setWorldRect(const QRectF &worldrect) { m_world = worldrect; }
    void setScreenRect(const QRect &viewrect);
private:
    bool m_viewAll;
    QRect m_screen;
    QRectF m_world;
    QRectF m_viewport;
    QPointF m_delta_worldtoscreen;
    double m_scale_worldtoscreen;
};


#endif // VIEWPORT_H
