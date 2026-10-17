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

/** @file WebControls.hpp */

#pragma once

// Include 3D Forest.
#include <WebWidget.hpp>

// Include local.
#include <ExportUiWeb.hpp>
#include <WarningsDisable.hpp>

/** WebControls

    Adapts the non-rendering common controls.
*/
class WebControls final : public WebWidget
{
public:
    WebControls(WebApplication &app, Widget *widget, const std::string &type);
    static std::string typeOf(Widget *widget);
    void bind() override;
    void sync(bool enabled, bool visible) override;
    void handleEvent(const JsonCpp::Value &event) override;

private:
    // Selection changes do not invalidate item addresses, but edits, sorting,
    // and rebuilding a model do. Reject events referring to obsolete rows.
    JsonCpp::Value model_;
    unsigned int modelVersion_{0};
};

#include <WarningsEnable.hpp>
