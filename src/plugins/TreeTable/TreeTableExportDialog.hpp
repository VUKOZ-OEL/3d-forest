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
#include <Dialog.hpp>
#include <FileFormatInterface.hpp>
class Application;
class CheckBox;
class LineEdit;
class PushButton;

/** Tree Table Export Dialog. */
class TreeTableExportDialog : public Dialog
{
public:
    TreeTableExportDialog(Application *app, const std::string &fileName);

    std::shared_ptr<FileFormatInterface> writer() const;

    void slotBrowse();
    void slotAccept();
    void slotReject();

private:
    Application *app_;

    LineEdit *fileNameLineEdit_;
    PushButton *browseButton_;

    PushButton *acceptButton_;
    PushButton *rejectButton_;
};

#endif /* TREE_TABLE_EXPORT_DIALOG_HPP */
