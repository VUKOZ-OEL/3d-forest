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

/** @file QtDialog.hpp */

#ifndef QT_DIALOG_HPP
#define QT_DIALOG_HPP

// Include 3D Forest.
class Dialog;
class QtApplication;

// Include Qt.
#include <QDialog>
class QCloseEvent;
class QDialogButtonBox;
class QResizeEvent;

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtDialog. */
class EXPORT_UI_QT QtDialog : public QDialog
{
public:
    QtDialog(Dialog *dialog, QtApplication *app, QWidget *parent = nullptr);

    void accept() override;
    void reject() override;
    void done(int result) override;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateProperties();

    Dialog *dialog_;
    QDialogButtonBox *buttonBox_;
};

#include <WarningsEnable.hpp>

#endif /* QT_DIALOG_HPP */
