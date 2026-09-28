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

/** @file QtLabel.hpp */

#ifndef QT_THEME_COLORS_HPP
#define QT_THEME_COLORS_HPP

// Include Qt.
#include <QColor>
class QApplication;

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtThemeColors. */
class EXPORT_UI_QT QtThemeColors
{
public:
    QColor background;      // Window/panel background.
    QColor surface;         // Button/input background.
    QColor foreground;      // Normal text.
    QColor disabledText;    // Disabled text.
    QColor border;          // Control borders.
    QColor hover;           // Hovered control background.
    QColor pressed;         // Pressed control background.
    QColor highlight;       // Selection or active range.
    QColor highlightedText; // Text drawn over highlight.

    QColor handle; // Slider handle fill.
    QColor groove; // Slider groove background.

    QColor panelBackground;
    QColor panelBorder;

    static bool isDesktopDarkMode(const QApplication *qapplication);

    void setDarkMode(bool dark);

    QString getStyleSheet() const;

private:
    bool darkMode_{false};
};

#include <WarningsEnable.hpp>

#endif /* QT_THEME_COLORS_HPP */
