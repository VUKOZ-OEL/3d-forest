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

/** @file FilterManagementStatusTreeWidget.hpp */

#ifndef FILTER_MANAGEMENT_STATUS_TREE_WIDGET_HPP
#define FILTER_MANAGEMENT_STATUS_TREE_WIDGET_HPP

// Include std.
#include <map>
#include <vector>

// Include 3D Forest.
#include <Segment.hpp>
class MainWindow;

// Include Qt.
#include <QWidget>
class QCheckBox;
class QVBoxLayout;

/** Management Status Tree Widget. */
class FilterManagementStatusTreeWidget : public QWidget
{
    Q_OBJECT

public:
    FilterManagementStatusTreeWidget(MainWindow *mainWindow);

    void setSegment(const Segment &segment);
    void clear();

public slots:
    void slotSetCheckbox(bool b);

protected:
    MainWindow *mainWindow_;
    std::vector<QCheckBox *> checkboxList_;
    QVBoxLayout *mainLayout_ = nullptr;

    Segment segment_;

    class Status
    {
    public:
        size_t statusId;
        QString label;

        bool operator==(const Status &other) const
        {
            return statusId == other.statusId && label == other.label;
        }
    };
    std::map<size_t, Status> statusMap_;

    std::map<size_t, Status> createMap();
    void createCheckBoxList();
    void updateCheckBoxList();

    void setCheckbox(size_t idx);
};

#endif /* FILTER_MANAGEMENT_STATUS_TREE_WIDGET_HPP */
