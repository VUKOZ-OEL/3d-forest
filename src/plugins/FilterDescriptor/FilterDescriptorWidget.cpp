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

/** @file FilterDescriptorWidget.cpp */

// Include 3D Forest.
#include <DoubleRangeSliderWidget.hpp>
#include <FilterDescriptorWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "FilterDescriptorWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

FilterDescriptorWidget::FilterDescriptorWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Start creating descriptor filter widget.");

    // Input widgets.
    DoubleRangeSliderWidget::create(descriptorInput_,
                                    this,
                                    SLOT(slotRangeIntermediateMinimumValue()),
                                    SLOT(slotRangeIntermediateMaximumValue()),
                                    tr("Descriptor"),
                                    tr("Min-max descriptor range filter"),
                                    tr("%"),
                                    1,
                                    0,
                                    100,
                                    0,
                                    100);

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(descriptorInput_);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // Data.
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));

    slotUpdate(nullptr, QSet<Editor::Type>());

    LOG_DEBUG(<< "Finished creating descriptor filter widget.");
}

void FilterDescriptorWidget::slotUpdate(void *sender,
                                        const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_DESCRIPTOR) ||
        target.contains(Editor::TYPE_SETTINGS))
    {
        LOG_DEBUG_UPDATE(<< "Input descriptor filter.");
        setDescriptor(mainWindow_->editor().descriptorFilter());
    }
}

void FilterDescriptorWidget::setDescriptor(const Range<double> &range)
{
    LOG_DEBUG(<< "Set descriptor filter range <" << range << ">.");

    descriptorRange_ = range;
    double min = descriptorRange_.minimum() * 100.0;
    double max = descriptorRange_.maximum() * 100.0;

    descriptorInput_->blockSignals(true);
    descriptorInput_->setMinimum(min);
    descriptorInput_->setMaximum(max);
    descriptorInput_->setMinimumValue(min);
    descriptorInput_->setMaximumValue(max);
    descriptorInput_->blockSignals(false);
}

void FilterDescriptorWidget::filterChanged()
{
    LOG_DEBUG(<< "Descriptor filer changed.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setDescriptorFilter(descriptorRange_);
    mainWindow_->updateFilter();
}

void FilterDescriptorWidget::setFilterEnabled(bool b)
{
    LOG_DEBUG(<< "Set descriptor filer enabled <" << toString(b) << ">.");

    descriptorRange_.setEnabled(b);
    filterChanged();
}

void FilterDescriptorWidget::slotRangeIntermediateMinimumValue()
{
    LOG_DEBUG(<< "Minimum value changed.");

    descriptorRange_.setMinimumValue(descriptorInput_->minimumValue() * 0.01);
    filterChanged();
}

void FilterDescriptorWidget::slotRangeIntermediateMaximumValue()
{
    LOG_DEBUG(<< "Maximum value changed.");

    descriptorRange_.setMaximumValue(descriptorInput_->maximumValue() * 0.01);
    filterChanged();
}
