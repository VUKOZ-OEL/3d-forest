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

/** @file InfoDialog.hpp */

#ifndef INFO_DIALOG_HPP
#define INFO_DIALOG_HPP

// Include Qt.
#include <QDialog>
class QTextEdit;
class QPushButton;

// Include local.
#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Info Dialog. */
class EXPORT_GUI InfoDialog : public QDialog
{
    Q_OBJECT

public:
    InfoDialog(QWidget *parent = nullptr, int w = 0, int h = 0);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setText(const QString &text);

public slots:
    void slotClose();

private:
    int defaultWidth_;
    int defaultHeight_;
    QTextEdit *textEdit_;
    QPushButton *closeButton_;
};

#include <WarningsEnable.hpp>

#endif /* INFO_DIALOG_HPP */
