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

/** @file WebSlider.hpp */

#pragma once

// Include 3D Forest.
#include <WebWidget.hpp>
class Slider;

// Include local.
#include <ExportUiWeb.hpp>
#include <WarningsDisable.hpp>

/** WebSlider. */
class EXPORT_UI_WEB WebSlider final : public WebWidget
{
public:
    WebSlider(WebApplication &application, Slider *slider);
    void bind() override;
    void sync(bool enabled, bool visible) override;
    void handleEvent(const JsonCpp::Value &event) override;

private:
    Slider *slider_;
    void updateSettings();
};

#include <WarningsEnable.hpp>
