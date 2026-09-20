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

/** @file MessageBox.hpp */

#ifndef MESSAGE_BOX_HPP
#define MESSAGE_BOX_HPP

// Include 3D Forest.
#include <Dialog.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Message Box. */
class EXPORT_UI_COMMON MessageBox : public Dialog
{
public:
    explicit MessageBox(Application *app);

    void setText(const std::string &text);
    void setInformativeText(const std::string &text);

    const std::string &text() const { return text_; }
    const std::string &informativeText() const { return informativeText_; }

    static int information(Application *app,
                           const std::string &title,
                           const std::string &text,
                           int buttons = Ok,
                           int defaultButton = NoButton);

    static int about(Application *app,
                     const std::string &title,
                     const std::string &text,
                     int buttons = Ok,
                     int defaultButton = NoButton);

    static int question(Application *app,
                        const std::string &title,
                        const std::string &text,
                        int buttons = Yes | No,
                        int defaultButton = NoButton);

    static int warning(Application *app,
                       const std::string &title,
                       const std::string &text,
                       int buttons = Ok,
                       int defaultButton = NoButton);

    static int critical(Application *app,
                        const std::string &title,
                        const std::string &text,
                        int buttons = Ok,
                        int defaultButton = NoButton);

private:
    std::string text_;
    std::string informativeText_;
};

#include <WarningsEnable.hpp>

#endif /* MESSAGE_BOX_HPP */
