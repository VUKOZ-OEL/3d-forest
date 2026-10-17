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

/** @file Splitter.hpp */

#ifndef SPLITTER_HPP
#define SPLITTER_HPP

// Include std.
#include <vector>

// Include 3D Forest.
#include <Widget.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Splitter. */
class EXPORT_UI_COMMON Splitter : public Widget
{
public:
    Splitter();
    ~Splitter() override;

    Splitter(const Splitter &) = delete;
    Splitter &operator=(const Splitter &) = delete;

    // Takes ownership of the common widget.
    void addWidget(Widget *widget);

    int count() const;
    Widget *widget(int index) const;

    const std::vector<Widget *> &widgets() const { return widgets_; }

    void setOrientation(int orientation);
    int orientation() const { return orientation_; }

    void setSizes(const std::vector<int> &sizes);

    // Configured sizes, used when creating the backend representation.
    const std::vector<int> &requestedSizes() const { return sizes_; }

    void setChildrenCollapsible(bool collapsible);
    bool childrenCollapsible() const { return childrenCollapsible_; }

    Signal<Widget *> widgetAdded;
    Signal<> settingsChanged;
    Signal<const std::vector<int> &> sizesUpdated;

    // User moved a handle: position and handle index.
    Signal<int, int> splitterMoved;

private:
    std::vector<Widget *> widgets_;
    std::vector<int> sizes_;

    int orientation_{Ui::Horizontal};
    bool childrenCollapsible_{true};
};

#include <WarningsEnable.hpp>

#endif /* SPLITTER_HPP */
