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

/** @file LineEdit.hpp */

#ifndef LINE_EDIT_HPP
#define LINE_EDIT_HPP

// Include 3D Forest.
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** LineEdit. */
class EXPORT_UI_COMMON LineEdit : public Widget
{
public:
    explicit LineEdit(const std::string &text = "");
    ~LineEdit() override;

    void setText(const std::string &text, bool notify = false);
    std::string text() const { return text_; }

    void clear();

    void setPlaceholderText(const std::string &text);
    const std::string &placeholderText() const { return placeholderText_; }

    void setReadOnly(bool readOnly);
    bool isReadOnly() const { return readOnly_; }

    Signal<const std::string &> textUpdated;
    Signal<const std::string &> textChanged;
    Signal<const std::string &> placeholderTextChanged;
    Signal<bool> readOnlyChanged;

    Signal<> returnPressed;
    Signal<> editingFinished;

private:
    std::string text_;
    std::string placeholderText_;
    bool readOnly_{false};
};

#include <WarningsEnable.hpp>

#endif /* LINE_EDIT_HPP */
