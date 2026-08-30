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

/** @file TextEdit.hpp */

#ifndef TEXT_EDIT_HPP
#define TEXT_EDIT_HPP

// Include 3D Forest.
#include <Widget.hpp>
class Application;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** TextEdit. */
class EXPORT_UI_COMMON TextEdit : public Widget
{
public:
    TextEdit();
    virtual ~TextEdit();

    void setText(const std::string &str);
    std::string text() const { return text_; }

    void setReadOnly(bool b);
    void append(const std::string &str);
    void clear();

    Signal<const std::string &> textChanged;

private:
    std::string text_;
};

#include <WarningsEnable.hpp>

#endif /* TEXT_EDIT_HPP */
