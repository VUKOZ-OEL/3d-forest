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

/** @file ImportFileDialog.hpp */

#ifndef IMPORT_FILE_DIALOG_HPP
#define IMPORT_FILE_DIALOG_HPP

// Include 3D Forest.
#include <ImportSettings.hpp>
class MainWindow;
class InfoDialog;

// Include Qt.
#include <QDialog>
class QCheckBox;
class QPushButton;

/** Import File Dialog. */
class ImportFileDialog : public QDialog
{
    Q_OBJECT

public:
    ImportFileDialog(MainWindow *mainWindow);

    ImportSettings settings() const;

public slots:
    void slotHelp();
    void slotAccept();
    void slotReject();

protected:
    InfoDialog *helpDialog_;

    QCheckBox *importFilesAsSeparateTreesCheckBox_;
    QCheckBox *translateToOriginCheckBox_;

    QCheckBox *convertToVersion1Dot4CheckBox_;
    QCheckBox *randomizePointsCheckBox_;
    QCheckBox *copyExtraBytesCheckBox_;

    QPushButton *helpButton_;
    QPushButton *acceptButton_;
    QPushButton *rejectButton_;
};

#endif /* IMPORT_FILE_DIALOG_HPP */
