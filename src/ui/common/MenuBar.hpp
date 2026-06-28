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

/** @file MenuBar.hpp */

#ifndef MENU_BAR_HPP
#define MENU_BAR_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
class Action;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Menu Bar. */
class EXPORT_UI_COMMON MenuBar
{
public:
    // Icons.
    // class IconEntry
    // {
    // public:
    //     QAction *action{nullptr};
    //     QToolButton *button{nullptr};
    //     ThemeIcon themeIcon;
    // };

    /** Main Window Menu Item. */
    class MenuItem
    {
    public:
        Action *action;
        std::string title;
        std::string toolBarTitle;
        int priority;
    };

    /** Main Window Menu. */
    class Menu
    {
    public:
        // QMenu *menu;
        std::string title;
        int priority;
        std::vector<MenuItem> items;
    };

    MenuBar();
    virtual ~MenuBar();

private:
    // std::vector<IconEntry> icons_;
    // bool isDarkMode();
    // void onThemeChanged();

    std::vector<MenuBar::Menu> menus_;
    // QHash<QString, size_t> menuIndex_;
    // QHash<QString, QToolBar *> toolBars_;

    Action *exitAction_;

    void createMenu();
};

#include <WarningsEnable.hpp>

#endif /* MENU_BAR_HPP */
