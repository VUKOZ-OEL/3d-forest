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

/** @file TextEdit.cpp */

// Include std.

// Include 3D Forest.
#include <Application.hpp>
#include <TextEdit.hpp>

// Include local.
#define LOG_MODULE_NAME "TextEdit"
#include <Log.hpp>

TextEdit::TextEdit()
{
}

TextEdit::~TextEdit()
{
}

void TextEdit::setText(const std::string &str, bool notify)
{
    if (text_ == str)
    {
        return;
    }

    text_ = str;
    textUpdated(text_);

    if (notify && !signalsBlocked())
    {
        textChanged(text_);
    }
}

void TextEdit::setReadOnly(bool readOnly)
{
    if (readOnly_ == readOnly)
    {
        return;
    }

    readOnly_ = readOnly;
    readOnlyChanged(readOnly_);
}

void TextEdit::append(const std::string &str)
{
    std::string text = text_;

    if (!text.empty())
    {
        text += '\n';
    }

    text += str;
    setText(text);
}

void TextEdit::clear()
{
    setText("");
}
