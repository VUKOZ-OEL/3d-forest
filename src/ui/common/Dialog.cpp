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

/** @file Dialog.cpp */

// Include std.

// Include 3D Forest.
#include <Dialog.hpp>
#include <Application.hpp>
#include <Layout.hpp>

// Include local.
#define LOG_MODULE_NAME "Dialog"
#include <Log.hpp>

Dialog::Dialog()
{
}

Dialog::Dialog(Application *app)
{
}

Dialog::~Dialog()
{
}

void Dialog::setWindowTitle(const std::string &str)
{
}

void Dialog::setWindowIcon(const ThemeIcon &icon)
{
}

void Dialog::setLayout(Layout *layout)
{
}

void Dialog::setFixedHeight(int h)
{
}

void Dialog::setMaximumHeight(int h)
{
}

int Dialog::height() const
{
    return 0;
}

void Dialog::setModal(bool b)
{
}
