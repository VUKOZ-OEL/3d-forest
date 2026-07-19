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

/** @file QtApplication.hpp */

#ifndef QT_APPLICATION_HPP
#define QT_APPLICATION_HPP

// Include std.
#include <string>

// Include 3D Forest.
#include <Application.hpp>

#ifdef interface
    #error interface is defined
#endif

#ifdef signals
    #error signals is defined
#endif

#ifdef slots
    #error slots is defined
#endif

#ifdef emit
    #error emit is defined
#endif

#ifdef const
    #error const is defined
#endif

#ifdef tr
    #undef tr
#endif

// Include Qt.
#include <QApplication>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtApplication. */
class EXPORT_UI_QT QtApplication : public Application
{
public:
    QtApplication(int &argc, char **argv);
    virtual ~QtApplication();

    void init();

    void setOrganizationName(const std::string &str);
    void setApplicationName(const std::string &str);
    void setApplicationVersion(const std::string &str);

    int exec();

private:
    QApplication qapplication_;
};

#include <WarningsEnable.hpp>

#endif /* QT_APPLICATION_HPP */
