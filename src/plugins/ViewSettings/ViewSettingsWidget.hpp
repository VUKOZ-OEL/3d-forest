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

/** @file ViewSettingsWidget.hpp */

#ifndef VIEW_SETTINGS_WIDGET_HPP
#define VIEW_SETTINGS_WIDGET_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <Editor.hpp>
#include <Settings.hpp>
#include <Widget.hpp>
class Application;
class ColorSwitch;
class CheckBox;
class ComboBox;
class Slider;

/** View Settings Widget. */
class ViewSettingsWidget : public Widget
{
public:
    ViewSettingsWidget(Application *app);

    // Size sizeHint() const override { return Size(300, 200); }

    void slotUpdate(void *sender, const std::set<Editor::Type> &target);

    void slotSetPointSize(int v);
    void slotSetColor();
    void slotColorSourceChanged(int index);

    void slotSetDistanceBasedFadingVisible(int v);
    void slotSetSceneBoundingBoxVisible(int v);

private:
    Application *app_;

    Slider *pointSizeSlider_;
    ColorSwitch *colorSwitch_;
    ComboBox *colorSourceComboBox_;

    CheckBox *distanceBasedFadingVisibleCheckBox_;
    CheckBox *sceneBoundingBoxVisibleCheckBox_;

    ViewSettings settings_;

    void dataChanged(bool modifiers = false);
    void setViewSettings(const ViewSettings &settings);
    void block();
    void unblock();
};

#endif /* VIEW_SETTINGS_WIDGET_HPP */
