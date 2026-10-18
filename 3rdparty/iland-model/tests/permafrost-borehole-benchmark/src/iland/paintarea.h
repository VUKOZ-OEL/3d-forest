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

#ifndef PAINTAREA_H
#define PAINTAREA_H

#include <QWidget>
#include "mapgrid.h"
#include "layeredgrid.h"
/**
  */
struct PaintObject {
    PaintObject(): what(PaintNothing), map_grid(0), float_grid(0), dbl_grid(0), layered(0), layer_id(0), min_value(0.), max_value(1.), cur_min_value(0.), cur_max_value(1.), auto_range(false), handler(0) {}
    enum { PaintNothing, PaintMapGrid, PaintFloatGrid, PaintLayers, PaintLIF, PaintTrees, PaintHeightGrid, PaintResourceUnits, PaintRegeneration, PaintHandledObject } what;
    MapGrid *map_grid;
    QString name;
    QString description;
    const FloatGrid *float_grid;
    const Grid<double> *dbl_grid;
    const LayeredGridBase *layered;
    int layer_id;
    GridViewType view_type;
    double min_value;
    double max_value;
    double cur_min_value, cur_max_value;
    bool auto_range;
    static QColor background_color;
    // other options
    bool clip_to_stands;
    bool species_colors;
    QString expression;
    QObject *handler;
};

/**PaintArea
  */
class PaintArea : public QWidget
 {
     Q_OBJECT

 public:
     PaintArea(QWidget *parent = 0);
     QImage &drawImage() { return m_bitmap; }

signals:
     void needsPainting(QPainter &painter);
     void mouseClick(const QPoint &pos);
     void mouseDrag(const QPoint &from, const QPoint &to, Qt::MouseButton mouseButton);
     void mouseMove(const QPoint &pos);
     void mouseWheel(const QPoint &pos, int wheel_steps);

 protected:
     void paintEvent(QPaintEvent *event);
     void mousePressEvent ( QMouseEvent * event );
     void mouseReleaseEvent ( QMouseEvent * event );
     void resizeEvent ( QResizeEvent * event );
     void mouseMoveEvent(QMouseEvent *event);
     void wheelEvent ( QWheelEvent * event );
     void keyPressEvent(QKeyEvent *event);
 private:
     QImage m_bitmap;
     QPoint m_lastDown;
 };


#endif // PAINTAREA_H
