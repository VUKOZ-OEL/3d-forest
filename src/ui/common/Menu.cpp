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

/** @file Menu.cpp */

// Include 3D Forest.
#include <Menu.hpp>
#include <Action.hpp>
#include <Application.hpp>

// Include local.
#define LOG_MODULE_NAME "Menu"
#include <Log.hpp>

Menu::Menu()
{
}

Menu::Menu(Application *app)
{
}

Menu::Menu(const std::string &title, Menu *parent)
{
}

Menu::~Menu()
{
}

Action *Menu::addAction(const std::string &title)
{
    Action *action = new Action(title);
    return action;
}

void Menu::addMenu(Menu *menu)
{
}
