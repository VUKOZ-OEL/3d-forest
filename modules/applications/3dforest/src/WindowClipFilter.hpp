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

/**
    @file WindowClipFilter.hpp
*/

#ifndef WINDOW_CLIP_FILTER_HPP
#define WINDOW_CLIP_FILTER_HPP

#include <Editor.hpp>
#include <QWidget>

class QDoubleSpinBox;
class QCheckBox;
class QPushButton;

/** Window Clip Filter. */
class WindowClipFilter : public QWidget
{
    Q_OBJECT

public:
    explicit WindowClipFilter(QWidget *parent = nullptr);
    ~WindowClipFilter();

    void updateEditor(const Editor &editor);

public slots:
    void valueChanged(double d);
    void stateChanged(int state);

signals:
    void filterChanged(const ClipFilter &clipFilter);
    void filterReset();

protected:
    QDoubleSpinBox *xMinSpinBox_;
    QDoubleSpinBox *xMaxSpinBox_;
    QDoubleSpinBox *yMinSpinBox_;
    QDoubleSpinBox *yMaxSpinBox_;
    QDoubleSpinBox *zMinSpinBox_;
    QDoubleSpinBox *zMaxSpinBox_;
    QCheckBox *enabledCheckBox_;
    QPushButton *resetButton_;

    void changed();
    void reset();
};

#endif /* WINDOW_CLIP_FILTER_HPP */
