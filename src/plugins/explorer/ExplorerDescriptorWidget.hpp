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

/** @file ExplorerDescriptorWidget.hpp */

#ifndef EXPLORER_DESCRIPTOR_WIDGET_HPP
#define EXPLORER_DESCRIPTOR_WIDGET_HPP

// Include 3D Forest.
#include <Editor.hpp>
#include <ExplorerWidgetInterface.hpp>
#include <Range.hpp>
class MainWindow;
class DoubleRangeSliderWidget;

/** Explorer Descriptor Widget. */
class ExplorerDescriptorWidget : public ExplorerWidgetInterface
{
    Q_OBJECT

public:
    ExplorerDescriptorWidget(MainWindow *mainWindow,
                             const QIcon &icon,
                             const QString &text);

    virtual bool hasColorSource() const { return true; }
    virtual SettingsView::ColorSource colorSource() const
    {
        return SettingsView::COLOR_SOURCE_DESCRIPTOR;
    }

    virtual bool hasFilter() const { return true; }
    virtual bool isFilterEnabled() const;
    virtual void setFilterEnabled(bool b);

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);

    void slotRangeIntermediateMinimumValue();
    void slotRangeIntermediateMaximumValue();

protected:
    DoubleRangeSliderWidget *descriptorInput_;
    Range<double> descriptorRange_;

    void setDescriptor(const Range<double> &descriptorRange);
    void filterChanged();
};

#endif /* EXPLORER_DESCRIPTOR_WIDGET_HPP */
