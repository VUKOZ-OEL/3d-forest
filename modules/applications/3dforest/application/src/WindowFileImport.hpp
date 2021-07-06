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

/** @file WindowFileImport.hpp */

#ifndef WINDOW_FILE_IMPORT_HPP
#define WINDOW_FILE_IMPORT_HPP

#include <QDialog>

class QCheckBox;
class QPushButton;

/** Window File Import. */
class WindowFileImport : public QDialog
{
    Q_OBJECT

public:
    WindowFileImport(QWidget *parent = nullptr);

    bool center() const;

public slots:
    void setResultAccept();
    void setResultReject();

protected:
    QCheckBox *centerCheckBox_;
    QCheckBox *convertCheckBox_;
    QPushButton *acceptButton_;
    QPushButton *rejectButton_;
};

#endif /* WINDOW_FILE_IMPORT_HPP */
