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

/** @file ExportFileDialog.hpp */

#ifndef EXPORT_FILE_DIALOG_HPP
#define EXPORT_FILE_DIALOG_HPP

#include <ExportFileProperties.hpp>

#include <QDialog>

class MainWindow;
class QCheckBox;
class QPushButton;
class QComboBox;

/** Export File Dialog. */
class ExportFileDialog : public QDialog
{
    Q_OBJECT

public:
    ExportFileDialog(MainWindow *mainWindow);

    ExportFileProperties properties() const;

public slots:
    void slotAccept();
    void slotReject();

private:
    std::vector<QCheckBox *> attributeCheckBox_;
    QComboBox *scaleComboBox_;

    QPushButton *acceptButton_;
    QPushButton *rejectButton_;
};

#endif /* EXPORT_FILE_DIALOG_HPP */
