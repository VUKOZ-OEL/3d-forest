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

/** @file ExplorerDescriptorWidget.cpp */

// Include 3D Forest.
#include <DoubleRangeSliderWidget.hpp>
#include <ExplorerDescriptorWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "ExplorerDescriptorWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

ExplorerDescriptorWidget::ExplorerDescriptorWidget(MainWindow *mainWindow,
                                                   const QIcon &icon,
                                                   const QString &text)
    : ExplorerWidgetInterface(mainWindow, icon, text)
{
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

    mainLayout_->addLayout(mainLayout);
    setLayout(mainLayout_);

    // Data.
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));
}

void ExplorerDescriptorWidget::slotUpdate(void *sender,
                                          const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_DESCRIPTOR))
    {
        LOG_TRACE_UPDATE(<< "Input descriptor filter.");

        setDescriptor(mainWindow_->editor().descriptorFilter());
    }
}

void ExplorerDescriptorWidget::filterChanged()
{
    LOG_TRACE_UPDATE(<< "Output descriptor filter <" << descriptorRange_
                     << ">.");

    mainWindow_->suspendThreads();
    mainWindow_->editor().setDescriptorFilter(descriptorRange_);
    mainWindow_->updateFilter();
}

void ExplorerDescriptorWidget::setDescriptor(
    const Range<double> &descriptorRange)
{
    LOG_DEBUG(<< "Set descriptor <" << descriptorRange << ">.");

    descriptorInput_->blockSignals(true);

    descriptorRange_ = descriptorRange;
    double min = descriptorRange_.minimum() * 100.0;
    double max = descriptorRange_.maximum() * 100.0;
    descriptorInput_->setMinimum(min);
    descriptorInput_->setMaximum(max);
    descriptorInput_->setMinimumValue(min);
    descriptorInput_->setMaximumValue(max);

    descriptorInput_->blockSignals(false);
}

void ExplorerDescriptorWidget::slotRangeIntermediateMinimumValue()
{
    descriptorRange_.setMinimumValue(descriptorInput_->minimumValue() * 0.01);
    filterChanged();
}

void ExplorerDescriptorWidget::slotRangeIntermediateMaximumValue()
{
    descriptorRange_.setMaximumValue(descriptorInput_->maximumValue() * 0.01);
    filterChanged();
}

bool ExplorerDescriptorWidget::isFilterEnabled() const
{
    return descriptorRange_.isEnabled();
}

void ExplorerDescriptorWidget::setFilterEnabled(bool b)
{
    descriptorRange_.setEnabled(b);
    filterChanged();
}
