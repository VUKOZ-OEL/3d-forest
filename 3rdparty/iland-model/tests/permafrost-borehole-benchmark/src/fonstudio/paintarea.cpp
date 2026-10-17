
#include <QtGui>

#include "paintarea.h"

PaintArea::PaintArea(QWidget *parent)
     : QWidget(parent)
 {
     //QTimer *timer = new QTimer(this);
     //connect(timer, SIGNAL(timeout()), this, SLOT(update()));
     //timer->start(1000);

     //setWindowTitle(tr("Analog Clock"));
     //resize(200, 200);
     m_bitmap = QImage(this->size(), QImage::Format_ARGB32);
     // enable mouse tracking
     this->setMouseTracking(true);

 }

void PaintArea::resizeEvent(QResizeEvent *event)
{
    m_bitmap = QImage(this->size(), QImage::Format_ARGB32);
    qDebug() << "paintarea resize" << this->size();
}
void PaintArea::paintEvent(QPaintEvent *)
{

    QPainter painter(this);
    QPainter pxpainter(&m_bitmap);

    //QPainter pxpainter(m_bitmap);
    //painter.drawRect(0, 0, width()-1, height()-1);

     emit needsPainting(pxpainter);
     painter.drawImage(rect(), m_bitmap);
     //painter.drawPixmap(rect(), m_bitmap);
     //painter.drawPixmap(0, 0,
//     QStylePainter spainter(this);
//     QStyleOption opt;
//     spainter.drawPrimitive(QStyle::PE_PanelButtonCommand, opt);

}

void PaintArea::mousePressEvent ( QMouseEvent * event )
{
    m_lastDown = event->pos();
    emit mouseClick(event->pos());
}

void PaintArea::mouseMoveEvent( QMouseEvent * event )
{
    emit mouseMove(event->pos());
}

void PaintArea::wheelEvent ( QWheelEvent * event )
{
    emit mouseWheel(event->pos(), event->delta() / 120);
}

void PaintArea::mouseReleaseEvent ( QMouseEvent * event )
{
    setCursor(Qt::CrossCursor);

    if ( (event->pos()-m_lastDown).manhattanLength() > 3) {
        emit mouseDrag(m_lastDown, event->pos(), event->button());
    }
    //emit mouseClick(event->pos());
     //emit needsPainting(painter);
}
