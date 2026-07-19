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

/** @file Dialog.hpp */

#ifndef DIALOG_HPP
#define DIALOG_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
#include <CloseEvent.hpp>
#include <Size.hpp>
#include <ThemeIcon.hpp>
#include <Ui.hpp>
class Application;
class Layout;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Dialog. */
class EXPORT_UI_COMMON Dialog
{
public:
    enum DialogCode
    {
        Rejected,
        Accepted
    };

    Dialog();
    Dialog(Application *app);
    virtual ~Dialog();

    void setWindowTitle(const std::string &str);
    void setWindowIcon(const ThemeIcon &icon);

    void setLayout(Layout *layout);

    void setFixedHeight(int h);
    void setMaximumWidth(int w);
    void setMaximumHeight(int h);
    int width() const;
    int height() const;

    void setModal(bool b);
    void setWindowModality(int modality);

    int exec();
    void show() {};
    void hide() {};
    void raise() {};
    void activateWindow() {};

    void close();
    void setResult(int result);

    virtual Size sizeHint() const { return Size(); }
    virtual Size minimumSizeHint() const { return Size(); }

    virtual void accept() {}
    virtual void reject() {}

    virtual void closeEvent(CloseEvent *event);

protected:
    Application *app_;
    std::string windowTitle_;
};

#include <WarningsEnable.hpp>

#endif /* DIALOG_HPP */
