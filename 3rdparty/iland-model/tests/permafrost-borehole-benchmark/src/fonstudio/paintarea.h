#ifndef PAINTAREA_H
#define PAINTAREA_H

#include <QWidget>

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
 private:
     QImage m_bitmap;
     QPoint m_lastDown;
 };


#endif // PAINTAREA_H
