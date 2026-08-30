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

/** @file QtGroupBox.hpp */

#ifndef QT_GROUP_BOX_HPP
#define QT_GROUP_BOX_HPP

// Include 3D Forest.
#include <GroupBox.hpp>
class QtApplication;

// Include Qt.
#include <QGroupBox>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtGroupBox. */
class EXPORT_UI_QT QtGroupBox : public QGroupBox
{
public:
    explicit QtGroupBox(GroupBox *groupBox,
                        QtApplication *app,
                        QWidget *parent = nullptr);
    virtual ~QtGroupBox();

private:
    GroupBox *groupBox_;
};

#include <WarningsEnable.hpp>

#endif /* QT_GROUP_BOX_HPP */
