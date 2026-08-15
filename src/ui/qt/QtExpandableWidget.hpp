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

/** @file QtExpandableWidget.hpp */

#ifndef QT_EXPANDABLE_WIDGET_HPP
#define QT_EXPANDABLE_WIDGET_HPP

// Include std.
#include <string>

// Include 3D Forest.

// Include Qt.
#include <QWidget>

class QToolButton;
class QVBoxLayout;

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtExpandableWidget. */
class EXPORT_UI_QT QtExpandableWidget : public QWidget
{
public:
    QtExpandableWidget(const std::string &title,
                       QWidget *content,
                       QWidget *parent = nullptr);

    QWidget *content() const { return content_; }

private:
    QToolButton *header_;
    QWidget *content_;
};

#include <WarningsEnable.hpp>

#endif /* QT_EXPANDABLE_WIDGET_HPP */
