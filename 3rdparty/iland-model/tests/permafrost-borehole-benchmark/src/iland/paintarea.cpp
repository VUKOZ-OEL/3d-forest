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
#include <QtGui>

#include "paintarea.h"

// static
QColor PaintObject::background_color = Qt::white;

/** @class PaintArea
  @ingroup GUI
  A small compoenent that acts as the "window" for special drawing.
  The PaintArea is embedded in the MainWindow. The PaintArea receives UI events from Qt and,
  after some processing emits signals.
  */

PaintArea::PaintArea(QWidget *parent)
     : QWidget(parent)
 {
     m_bitmap = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
     // enable mouse tracking
     this->setMouseTracking(true);
     // enable keyboard focus
     setFocusPolicy(Qt::StrongFocus);

 }

void PaintArea::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_bitmap = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
    //qDebug() << "paintarea resize" << this->size();
}

QMutex _paint_mtx;
void PaintArea::paintEvent(QPaintEvent *)
{
    QMutexLocker lock(&_paint_mtx);

    if (QThread::currentThread() != QCoreApplication::instance()->thread())
        qDebug() << "Paint in wrong thread???";

    QPainter pxpainter(&m_bitmap);
    emit needsPainting(pxpainter); // code draws on m_bitmap


    QPainter painter(this);
    painter.drawImage(rect(), m_bitmap);
}

void PaintArea::mousePressEvent ( QMouseEvent * event )
{
    m_lastDown = event->pos();
    //emit mouseClick(event->pos());
}

void PaintArea::mouseMoveEvent( QMouseEvent * event )
{
    if (event->buttons() == Qt::LeftButton)
        setCursor(Qt::ClosedHandCursor);
    emit mouseMove(event->pos());
}

void PaintArea::wheelEvent ( QWheelEvent * event )
{
    emit mouseWheel(event->pos(), event->delta() / 120);
}

void PaintArea::mouseReleaseEvent ( QMouseEvent * event )
{
    setCursor(Qt::CrossCursor);

    if ( (event->pos()-m_lastDown).manhattanLength() > 2) {
        emit mouseDrag(m_lastDown, event->pos(), event->button());
    } else {
        // a click event...
        emit mouseClick(event->pos());
    }
}

void PaintArea::keyPressEvent(QKeyEvent *event)
{
    // emulate mouse actions with the keyboard
    QPoint mousepos = this->rect().center();
    switch (event->key()) {
        case Qt::Key_Plus:
        case Qt::Key_PageUp:  // zoom in
            emit mouseWheel(mousepos, 1);
            break;

        case Qt::Key_PageDown: // zoom out
        case Qt::Key_Minus:
            emit mouseWheel(mousepos, -1);
            break;
        // pan with cursor keys
        case Qt::Key_Left:
            emit mouseDrag(mousepos, mousepos + QPoint(20, 0), Qt::MidButton); break;
        case Qt::Key_Right:
            emit mouseDrag(mousepos, mousepos + QPoint(-20, 0), Qt::MidButton); break;

        case Qt::Key_Up:
            emit mouseDrag(mousepos, mousepos + QPoint(0, 20), Qt::MidButton); break;
        case Qt::Key_Down:
            emit mouseDrag(mousepos, mousepos + QPoint(0, -20), Qt::MidButton); break;
        }
}
