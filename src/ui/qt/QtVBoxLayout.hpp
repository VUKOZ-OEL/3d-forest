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

/** @file QtVBoxLayout.hpp */

#ifndef QT_V_BOX_LAYOUT_HPP
#define QT_V_BOX_LAYOUT_HPP

// Include 3D Forest.
#include <VBoxLayout.hpp>
class QtApplication;

// Include Qt.
#include <QVBoxLayout>
class QWidget;

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtVBoxLayout. */
class EXPORT_UI_QT QtVBoxLayout : public QVBoxLayout
{
public:
    explicit QtVBoxLayout(VBoxLayout *layout,
                          QtApplication *app,
                          QWidget *parent = nullptr);
    virtual ~QtVBoxLayout();

private:
    VBoxLayout *layout_;
};

#include <WarningsEnable.hpp>

#endif /* QT_V_BOX_LAYOUT_HPP */
