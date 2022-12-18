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

/** @file ProjectNavigatorItemDescriptor.cpp */

#include <MainWindow.hpp>
#include <ProjectNavigatorItemDescriptor.hpp>
#include <RangeSliderWidget.hpp>

#include <QVBoxLayout>

ProjectNavigatorItemDescriptor::ProjectNavigatorItemDescriptor(
    MainWindow *mainWindow)
    : ProjectNavigatorItem(),
      mainWindow_(mainWindow)
{
    // Input widgets
    RangeSliderWidget::create(descriptorInput_,
                              this,
                              SLOT(slotRangeIntermediateMinimumValue()),
                              SLOT(slotRangeIntermediateMaximumValue()),
                              tr("Descriptor"),
                              tr("Min-max descriptor range filter"),
                              tr("pt"),
                              1,
                              0,
                              100,
                              0,
                              100);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(descriptorInput_);
    mainLayout->addStretch();
    setLayout(mainLayout);

    // Data
    connect(mainWindow_,
            SIGNAL(signalUpdate(const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(const QSet<Editor::Type> &)));
}

void ProjectNavigatorItemDescriptor::slotUpdate(
    const QSet<Editor::Type> &target)
{
    if (target.empty() || target.contains(Editor::TYPE_DESCRIPTOR))
    {
        descriptorRange_ = mainWindow_->editor().descriptorRange();

        descriptorInput_->blockSignals(true);
        descriptorInput_->setMinimum(descriptorRange_.minimum() * 255.0);
        descriptorInput_->setMaximum(descriptorRange_.maximum() * 255.0);
        descriptorInput_->setMinimumValue(descriptorRange_.minimumValue() *
                                          255.0);
        descriptorInput_->setMaximumValue(descriptorRange_.maximumValue() *
                                          255.0);
        descriptorInput_->blockSignals(false);
    }
}

void ProjectNavigatorItemDescriptor::slotRangeIntermediateMinimumValue()
{
    descriptorRange_.setMinimumValue(descriptorInput_->minimumValue() *
                                     0.0039216F);
    descriptorInputChanged();
}

void ProjectNavigatorItemDescriptor::slotRangeIntermediateMaximumValue()
{
    descriptorRange_.setMaximumValue(descriptorInput_->maximumValue() *
                                     0.0039216F);
    descriptorInputChanged();
}

void ProjectNavigatorItemDescriptor::descriptorInputChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setDescriptorRange(descriptorRange_);
    mainWindow_->updateFilter();
}
