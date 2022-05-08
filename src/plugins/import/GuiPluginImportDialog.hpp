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

/** @file GuiPluginImportDialog.hpp */

#ifndef GUI_PLUGIN_IMPORT_DIALOG_HPP
#define GUI_PLUGIN_IMPORT_DIALOG_HPP

#include <EditorSettingsImport.hpp>

#include <QDialog>

class GuiWindowMain;
class QCheckBox;
class QPushButton;

/** Gui Plugin Import Dialog. */
class GuiPluginImportDialog : public QDialog
{
    Q_OBJECT

public:
    GuiPluginImportDialog(GuiWindowMain *window);

    EditorSettingsImport getSettings() const;

public slots:
    void slotAccept();
    void slotReject();

protected:
    QCheckBox *centerCheckBox_;
    QCheckBox *convertCheckBox_;
    QPushButton *acceptButton_;
    QPushButton *rejectButton_;
};

#endif /* GUI_PLUGIN_IMPORT_DIALOG_HPP */
