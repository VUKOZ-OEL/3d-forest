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
    border = dark ? QColor("#77837c") : QColor("#c8ced0");
    hover = dark ? QColor("#2b2b2b") : QColor("#e8e8e8");
    pressed = dark ? QColor("") : QColor("");
    highlight = dark ? QColor("#69bd83") : QColor("#21833f");
    highlightedText = dark ? QColor("") : QColor("");

    handle = dark ? QColor("#e5ece8") : QColor("#f8faf9");
    groove = dark ? QColor("#48534d") : QColor("#d5dcda");

    panelBackground = dark ? QColor("#303030") : QColor("#eeeeee");
    panelBorder = dark ? QColor("#484848") : QColor("#d4d4d4");

    viewBackground = dark ? QColor("#000000") : QColor("#ffffff");
    viewAlternate = panelBackground;
    viewGrid = dark ? border : QColor("#e3e3e3");

    splitterColor = dark ? QColor("#454545") : QColor("#d0d0d0");
    splitterHoverColor = dark ? QColor("#686868") : QColor("#a0a0a0");

    buttonBackground = dark ? QColor("#414141") : QColor("#ffffff");
    buttonHover = dark ? QColor("#505050") : QColor("#f5f5f5");
    buttonPressed = dark ? QColor("#353535") : QColor("#e5e5e5");
    buttonBorder = dark ? QColor("#606060") : QColor("#c0c0c0");
    buttonDisabled = dark ? QString("#858585") : QString("#999999");
}

QString QtThemeColors::getStyleSheet() const
{
    QString styleSheet;
    const QString navRow = darkMode_ ? "#30363a" : "#edf0f2";
    const QString navHover = darkMode_ ? "#3a4348" : "#e4e9ec";
    const QString navActive = darkMode_ ? "#3a4145" : "#e2f2e9";
    const QString navAccent = darkMode_ ? "#69bd83" : "#21833f";
    const QString navBorder = darkMode_ ? "#454d52" : "#d4dadd";
    const QString navInput = darkMode_ ? "#202020" : "#ffffff";
    const QString navPage = darkMode_ ? "#262626" : "#f7f7f7";
    const QString navBody = darkMode_ ? "#2c3033" : "#f0f6f3";
    const QString checkmark =
        darkMode_ ? ":/UiQt/checkmark-dark.svg" : ":/UiQt/checkmark.svg";
    const QString arrowSuffix = darkMode_ ? "-dark.svg" : ".svg";

    // clang-format off
    styleSheet = 
        "QWidget[sidebarPanel=\"true\"] {"
        "    background-color: " + navBody + ";"
        "    border: 1px solid " + navBorder + ";"
        "    border-top: none;"
        "    border-radius: 3px;"
        "}"
        "QWidget[sidebarPanel=\"true\"], QWidget[sidebarPanel=\"true\"] QWidget {"
        "    font-size: 14px;"
        "}"
        ""
        "QWidget#bottomPanel {"
        "    background-color: " + panelBackground.name() + ";"
        "}"
        ""
        "QSplitter::handle {"
        "    background-color: " + splitterColor.name() + ";"
        "}"
        ""
        "QSplitter::handle:hover {"
        "    background-color: " + splitterHoverColor.name() + ";"
        "}"
        ""
        "QGroupBox {"
        "    color: " + surface.name() + ";"
        "}"
        ""
        "QToolBar:vertical {"
        "    border: none;"
        "    padding: 0px;"
        "    margin: 0px;"
        "    spacing: 1px;"
        "}"
        ""
        "QToolBar:vertical QToolButton {"
        "    padding-left: 2px;"
        "    padding-right: 2px;"
        "}"
        ""
        "QWidget#navigationSidebar {"
        "    background: " + navPage + ";"
        "    font-size: 14px;"
        "}"
        "QWidget#navigationCategoryRail {"
        "    background: " + navRow + ";"
        "    border-right: 1px solid " + navBorder + ";"
        "}"
        "QToolButton#navigationCategoryButton {"
        "    background: " + navRow + ";"
        "    color: " + surface.name() + ";"
        "    border: 0; border-radius: 3px;"
        "    padding: 4px 0px; font-size: 10px;"
        "}"
        "QToolButton#navigationCategoryButton:hover {"
        "    background: " + navHover + ";"
        "}"
        "QToolButton#navigationCategoryButton:checked {"
        "    background: " + navActive + ";"
        "    color: " + navAccent + ";"
        "}"
        "QLabel#navigationCategoryTitle {"
        "    font-size: 22px; font-weight: 600;"
        "}"
        "QLineEdit#navigationSearch {"
        "    background: " + navInput + ";"
        "    border: 1px solid " + navBorder + ";"
        "    border-radius: 3px; padding: 7px 10px;"
        "    font-size: 13px;"
        "}"
        ""
        "QTreeWidget#sidebarNavigationTree {"
        "    background: " + navPage + ";"
        "    color: " + surface.name() + ";"
        "    border: none;"
        "    outline: none;"
        "    font-size: 14px;"
        "}"
        ""
        "QTreeWidget#sidebarNavigationTree::item {"
        "    color: " + surface.name() + ";"
        "    padding: 0;"
        "    border: none;"
        "}"
        ""
        ""
        "QTreeWidget#sidebarNavigationTree::branch {"
        "    background: " + background.name() + ";"
        "}"
        ""
        "QTreeWidget, QTableWidget {"
        "    background-color: " + viewBackground.name() + ";"
        // "    alternate-background-color: " + viewAlternate.name() + ";"
        "    color: " + surface.name() + ";"
        "}"
        ""
        "QTreeWidget::indicator {"
        "    width: 16px; height: 16px;"
        "    border: 1px solid " + border.name() + ";"
        "    border-radius: 2px; background: " + viewBackground.name() + ";"
        "}"
        "QTreeWidget::indicator:checked {"
        "    background: " + navAccent + "; border-color: " + navAccent + ";"
        "    image: url(" + checkmark + ");"
        "}"
        "QTreeWidget::indicator:indeterminate {"
        "    background: " + navAccent + "; border-color: " + navAccent + ";"
        "}"
        ""
        "QTableWidget {"
        "    gridline-color: " + viewGrid.name() + ";"
        "}"
        ""
        "QHeaderView {"
        "    background-color: " + viewBackground.name() + ";"
        "}"
        "QHeaderView::section {"
        "    background-color: " + viewBackground.name() + ";"
        "    color: " + surface.name() + ";"
        "    border: none;"
        "    border-right: 1px solid " + viewGrid.name() + ";"
        "    border-bottom: 1px solid " + viewGrid.name() + ";"
        "}"
        ""
        "QTableCornerButton::section {"
        "    background-color: " + viewBackground.name() + ";"
        "    border: none;"
        "}"
        ""
        "QLabel,"
        "QCheckBox,"
        "QRadioButton,"
        // "QPushButton,"
        "QToolButton,"
        "QLineEdit,"
        "QSpinBox,"
        "QDoubleSpinBox,"
        "QComboBox {"
        "    color: " + surface.name() + ";"
        "    background: transparent;"
        "}"
    "QPushButton {"
    "    color: " + surface.name() + ";"
    "    background-color: " + buttonBackground.name() + ";"
    "    border: 1px solid " + buttonBorder.name() + ";"
    "    border-radius: 4px;"
    "    padding: 3px 10px;"
    "}"
    "QPushButton:hover:enabled {"
    "    background-color: " + buttonHover.name() + ";"
    "}"
    "QPushButton:pressed:enabled {"
    "    background-color: " + buttonPressed.name() + ";"
    "}"
    "QPushButton:disabled {"
    "    color: " + buttonDisabled.name() + ";"
    "}"
        ""
        "QSlider[singleValueSlider=\"true\"]::groove:horizontal {"
        "    height: 4px;"
        "    background: " + groove.name() + ";"
        "    border-radius: 2px;"
        "}"
        ""
        "QSlider[singleValueSlider=\"true\"]::sub-page:horizontal {"
        "    background: " + highlight.name() + ";"
        "    border-radius: 2px;"
        "}"
        ""
        "QSlider[singleValueSlider=\"true\"]::handle:horizontal {"
        "    width: 14px;"
        "    height: 14px;"
        "    margin: -5px 0;"
        "    background: " + handle.name() + ";"
        "    border: 2px solid " + highlight.name() + ";"
        "    border-radius: 7px;"
        "}"
        ""
        "QtDoubleRangeSlider {"
        "    qproperty-grooveColor: " + groove.name() + ";"
        "    qproperty-highlightColor: " + highlight.name() + ";"
        "    qproperty-borderColor: " + highlight.name() + ";"
        "    qproperty-handleColor: " + handle.name() + ";"
        "}"
        "QWidget[sidebarPanel=\"true\"] QCheckBox {"
        "    spacing: 8px; padding: 3px 0;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QCheckBox::indicator {"
        "    width: 18px; height: 18px;"
        "    border: 1px solid " + border.name() + ";"
        "    border-radius: 2px; background: " + navInput + ";"
        "}"
        "QWidget[sidebarPanel=\"true\"] QCheckBox::indicator:checked {"
        "    background: " + navAccent + "; border-color: " + navAccent + ";"
        "    image: url(" + checkmark + ");"
        "}"
        "QWidget[sidebarPanel=\"true\"] QCheckBox::indicator:indeterminate {"
        "    background: #21833f; border-color: #21833f;"
        "    image: url(:/UiQt/indeterminate.svg);"
        "}"
        "QWidget[sidebarPanel=\"true\"] QCheckBox::indicator:disabled {"
        "    background: " + groove.name() + "; border-color: " + groove.name() + ";"
        "}"
        "QWidget[sidebarPanel=\"true\"] QPushButton {"
        "    min-height: 26px; max-height: 26px; padding: 3px 12px;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QPushButton[primaryAction=\"true\"] {"
        "    color: #ffffff; background: #21833f; border: 1px solid #21833f;"
        "    min-width: 88px; padding: 3px 16px;"
        "    border-radius: 3px; font-weight: 600;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QPushButton[primaryAction=\"true\"]:hover:enabled {"
        "    background: #277b41; border-color: #277b41;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QPushButton[primaryAction=\"true\"]:pressed:enabled {"
        "    background: #196731; border-color: #196731;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QPushButton[primaryAction=\"true\"]:disabled {"
        "    color: " + buttonDisabled.name() + "; background: " + navRow + ";"
        "    border-color: " + navBorder + ";"
        "}"
        "QWidget[sidebarPanel=\"true\"] QSlider[singleValueSlider=\"true\"]::groove:horizontal {"
        "    height: 6px; border-radius: 3px;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QSlider[singleValueSlider=\"true\"]::handle:horizontal {"
        "    width: 18px; height: 18px; margin: -6px 0; border-radius: 9px;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QRadioButton { spacing: 8px; padding: 3px 0; }"
        "QWidget[sidebarPanel=\"true\"] QRadioButton::indicator {"
        "    width: 18px; height: 18px; border-radius: 9px;"
        "    border: 1px solid " + border.name() + "; background: " + navInput + ";"
        "}"
        "QWidget[sidebarPanel=\"true\"] QRadioButton::indicator:checked {"
        "    border: none; background: transparent;"
        "    image: url(:/UiQt/radio-checked" + arrowSuffix + ");"
        "}"
        "QWidget[sidebarPanel=\"true\"] QRadioButton::indicator:disabled {"
        "    background: " + groove.name() + ";"
        "}"
        "QWidget[sidebarPanel=\"true\"] QLineEdit,"
        "QWidget[sidebarPanel=\"true\"] QSpinBox,"
        "QWidget[sidebarPanel=\"true\"] QDoubleSpinBox,"
        "QWidget[sidebarPanel=\"true\"] QComboBox {"
        "    background: " + navInput + "; border: 1px solid " + navBorder + ";"
        "    border-radius: 3px; padding: 3px; min-height: 22px;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QAbstractSpinBox QLineEdit {"
        "    border: none; background: transparent; padding: 0; min-height: 0;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QAbstractSpinBox {"
        "    padding-right: 20px; min-width: 58px;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QSpinBox::up-button,"
        "QWidget[sidebarPanel=\"true\"] QDoubleSpinBox::up-button {"
        "    subcontrol-origin: border; subcontrol-position: top right;"
        "    width: 18px; border-left: 1px solid " + navBorder + ";"
        "}"
        "QWidget[sidebarPanel=\"true\"] QSpinBox::down-button,"
        "QWidget[sidebarPanel=\"true\"] QDoubleSpinBox::down-button {"
        "    subcontrol-origin: border; subcontrol-position: bottom right;"
        "    width: 18px; border-left: 1px solid " + navBorder + ";"
        "    border-top: 1px solid " + navBorder + ";"
        "}"
        "QWidget[sidebarPanel=\"true\"] QSpinBox::up-arrow,"
        "QWidget[sidebarPanel=\"true\"] QDoubleSpinBox::up-arrow {"
        "    image: url(:/UiQt/chevron-up" + arrowSuffix + "); width: 10px; height: 6px;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QSpinBox::down-arrow,"
        "QWidget[sidebarPanel=\"true\"] QDoubleSpinBox::down-arrow,"
        "QWidget[sidebarPanel=\"true\"] QComboBox::down-arrow {"
        "    image: url(:/UiQt/chevron-down" + arrowSuffix + "); width: 10px; height: 6px;"
        "}"
        "QWidget[sidebarPanel=\"true\"] QComboBox { padding-right: 18px; }"
        "QWidget[sidebarPanel=\"true\"] QComboBox::drop-down {"
        "    subcontrol-origin: border; subcontrol-position: top right;"
        "    width: 18px; border: none;"
        "}";
    // clang-format on

    return styleSheet;
}
