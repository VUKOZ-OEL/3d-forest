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

/** @file AlgorithmWidgetInterface.hpp */

#ifndef ALGORITHM_WIDGET_INTERFACE_HPP
#define ALGORITHM_WIDGET_INTERFACE_HPP

class MainWindow;

#include <QWidget>
class QVBoxLayout;

/** Algorithm Widget Interface. */
class AlgorithmWidgetInterface : public QWidget
{
    Q_OBJECT

public:
    AlgorithmWidgetInterface(MainWindow *mainWindow,
                             const QIcon &icon,
                             const QString &text);
    virtual ~AlgorithmWidgetInterface() = default;

    const QIcon &icon() { return icon_; }
    const QString &text() { return text_; }

    virtual bool applyParameters() = 0;
    virtual bool next() = 0;
    virtual void progress(size_t &nTasks,
                          size_t &iTask,
                          double &percent) const = 0;
    virtual void updateData() = 0;

signals:
    void signalParametersChanged();

protected:
    MainWindow *mainWindow_;

    QIcon icon_;
    QString text_;
    QVBoxLayout *mainLayout_;
};

#endif /* ALGORITHM_WIDGET_INTERFACE_HPP */
