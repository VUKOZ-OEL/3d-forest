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

/** @file ExportUiQt.hpp */

#ifndef EXPORT_UI_QT_HPP
#define EXPORT_UI_QT_HPP

#if defined(_MSC_VER)
    #if defined(EXPORT_3DForestUiQt)
        #define EXPORT_UI_QT __declspec(dllexport)
        #define EXPORT_UI_QT_T
    #else
        #define EXPORT_UI_QT __declspec(dllimport)
        #define EXPORT_UI_QT_IMPORT
        #define EXPORT_UI_QT_T extern
    #endif
#else
    #define EXPORT_UI_QT
#endif

#endif /* EXPORT_UI_QT_HPP */
