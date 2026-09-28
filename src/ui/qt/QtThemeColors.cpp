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

/** @file QtSlider.cpp */

// Include 3D Forest.
#include <QtThemeColors.hpp>

// Include Qt.
#include <QApplication>
#include <QPalette>
#include <QStyleHints>

// Include local.
#define LOG_MODULE_NAME "QtSlider"
#include <Log.hpp>

bool QtThemeColors::isDesktopDarkMode(const QApplication *qapplication)
{
    const Qt::ColorScheme scheme = qapplication->styleHints()->colorScheme();

    if (scheme == Qt::ColorScheme::Dark)
    {
        return true;
    }

    if (scheme == Qt::ColorScheme::Light)
    {
        return false;
    }

    const int lightness =
        qapplication->palette().color(QPalette::Window).lightness();

    return lightness < 128;
}

void QtThemeColors::setDarkMode(bool dark)
{
    darkMode_ = dark;

    background = dark ? QColor("#171717") : QColor("#f7f7f7");
    surface = dark ? QColor("#ececec") : QColor("#202020");
    foreground = dark ? QColor("") : QColor("");
    disabledText = dark ? QColor("") : QColor("");
    border = dark ? QColor("#909090") : QColor("#707070");
    hover = dark ? QColor("#2b2b2b") : QColor("#e8e8e8");
    pressed = dark ? QColor("") : QColor("");
    highlight = dark ? QColor("#d0d0d0") : QColor("#505050");
    highlightedText = dark ? QColor("") : QColor("");

    handle = dark ? QColor("#f0f0f0") : QColor("#ffffff");
    groove = dark ? QColor("#484848") : QColor("#c6c6c6");
}

QString QtThemeColors::getStyleSheet() const
{
    QString styleSheet;

    styleSheet = "QWidget {"
                 "    color: " +
                 surface.name() +
                 ";"
                 "    background: transparent;"
                 "}"
                 ""
                 "QtSidebar {"
                 "    background: " +
                 background.name() +
                 ";"
                 "}"
                 ""
                 "QTreeWidget#sidebarNavigationTree {"
                 "    background: " +
                 background.name() +
                 ";"
                 "    color: " +
                 surface.name() +
                 ";"
                 "    border: none;"
                 "    outline: none;"
                 "    font-size: 14px;"
                 "}"
                 ""
                 "QTreeWidget#sidebarNavigationTree::item {"
                 "    color: " +
                 surface.name() +
                 ";"
                 "    min-height: 30px;"
                 "    padding: 3px 8px;"
                 "    border: none;"
                 "    border-radius: 7px;"
                 "}"
                 ""
                 "QTreeWidget#sidebarNavigationTree::item:hover {"
                 "    background: " +
                 hover.name() +
                 ";"
                 "}"
                 ""
                 "QTreeWidget#sidebarNavigationTree::branch {"
                 "    background: " +
                 background.name() +
                 ";"
                 "}"
                 ""
                 "QLabel,"
                 "QCheckBox,"
                 "QRadioButton {"
                 "    color: " +
                 surface.name() +
                 ";"
                 "    background: transparent;"
                 "}"
                 ""
                 "QSlider[singleValueSlider=\"true\"]::groove:horizontal {"
                 "    height: 4px;"
                 "    background: " +
                 groove.name() +
                 ";"
                 "    border-radius: 2px;"
                 "}"
                 ""
                 "QSlider[singleValueSlider=\"true\"]::sub-page:horizontal {"
                 "    background: " +
                 highlight.name() +
                 ";"
                 "    border-radius: 2px;"
                 "}"
                 ""
                 "QSlider[singleValueSlider=\"true\"]::handle:horizontal {"
                 "    width: 14px;"
                 "    margin: -5px 0;"
                 "    background: " +
                 handle.name() +
                 ";"
                 "    border: 1px solid " +
                 border.name() +
                 ";"
                 "    border-radius: 7px;"
                 "}"
                 "QtDoubleRangeSlider {"
                 "    qproperty-grooveColor: " +
                 groove.name() +
                 ";"
                 "    qproperty-highlightColor: " +
                 highlight.name() +
                 ";"
                 "    qproperty-borderColor: " +
                 border.name() +
                 ";"
                 "    qproperty-handleColor: " +
                 handle.name() +
                 ";"
                 "}";

    return styleSheet;
}
