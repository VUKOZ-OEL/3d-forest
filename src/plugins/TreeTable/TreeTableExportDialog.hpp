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

/** @file TreeTableExportDialog.hpp */

#ifndef TREE_TABLE_EXPORT_DIALOG_HPP
#define TREE_TABLE_EXPORT_DIALOG_HPP

// Include 3D Forest.
#include <TreeTableExportInterface.hpp>
class MainWindow;

// Include Qt.
#include <QDialog>
class QCheckBox;
class QLineEdit;
class QPushButton;

/** Tree Table Export Dialog. */
class TreeTableExportDialog : public QDialog
{
    Q_OBJECT

public:
    TreeTableExportDialog(MainWindow *mainWindow, const QString &fileName);

    std::shared_ptr<TreeTableExportInterface> writer() const;
    TreeTableExportProperties properties() const;

public slots:
    void slotBrowse();
    void slotAccept();
    void slotReject();

private:
    MainWindow *mainWindow_;

    QLineEdit *fileNameLineEdit_;
    QPushButton *browseButton_;
    QCheckBox *exportValidValuesOnlyCheckBox_;

    QPushButton *acceptButton_;
    QPushButton *rejectButton_;
};

#endif /* TREE_TABLE_EXPORT_DIALOG_HPP */
