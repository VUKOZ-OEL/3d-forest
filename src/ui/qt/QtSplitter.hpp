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

/** @file QtSplitter.hpp */

#ifndef QT_SPLITTER_HPP
#define QT_SPLITTER_HPP

// Include 3D Forest.
#include <Splitter.hpp>
class QtApplication;

// Include Qt.
#include <QSplitter>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtSplitter. */
class EXPORT_UI_QT QtSplitter : public QSplitter
{
public:
    QtSplitter(Splitter *splitter,
               QtApplication *app,
               QWidget *parent = nullptr);

    ~QtSplitter() override;

private:
    void addCommonWidget(Widget *widget);
    void updateSettings();
    void applySizes(const std::vector<int> &sizes);

    Splitter *splitter_;
    QtApplication *app_;
};

#include <WarningsEnable.hpp>

#endif /* QT_SPLITTER_HPP */
