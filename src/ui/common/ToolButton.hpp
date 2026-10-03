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

/** @file ToolButton.hpp */

#ifndef TOOL_BUTTON_HPP
#define TOOL_BUTTON_HPP

// Include 3D Forest.
#include <Pixmap.hpp>
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** ToolButton. */
class EXPORT_UI_COMMON ToolButton : public Widget
{
public:
    enum ToolButtonStyle
    {
        IconOnly,
        TextOnly,
        TextBesideIcon,
        TextUnderIcon
    };

    ToolButton(const std::string &text = "");
    virtual ~ToolButton();

    ToolButton(const ToolButton &) = delete;
    ToolButton &operator=(const ToolButton &) = delete;

    const std::string &text() const { return text_; }
    void setText(const std::string &text);

    const std::string &toolTip() const { return toolTip_; }
    void setToolTip(const std::string &toolTip);

    const ThemeIcon &icon() const { return icon_; }
    void setIcon(const ThemeIcon &icon);

    // void setPixmap(const Pixmap &pixmap);
    // const Pixmap *pixmap() const { return &pixmap_; }

    ToolButtonStyle toolButtonStyle() const { return style_; }
    void setToolButtonStyle(ToolButtonStyle style);

    bool autoRaise() const { return autoRaise_; }
    void setAutoRaise(bool enabled);

    Signal<> clicked;

    // Backend notifications.
    Signal<> settingsChanged;
    Signal<> destroying;

private:
    std::string text_;
    std::string toolTip_;
    ThemeIcon icon_;
    // Pixmap pixmap_;

    ToolButtonStyle style_{IconOnly};
    bool autoRaise_{true};
};

#include <WarningsEnable.hpp>

#endif /* TOOL_BUTTON_HPP */
