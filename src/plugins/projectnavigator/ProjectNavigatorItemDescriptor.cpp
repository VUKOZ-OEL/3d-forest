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

#include <DoubleRangeSliderWidget.hpp>
#include <MainWindow.hpp>
#include <ProjectNavigatorItemDescriptor.hpp>

#include <QVBoxLayout>

ProjectNavigatorItemDescriptor::ProjectNavigatorItemDescriptor(
    MainWindow *mainWindow,
    const QIcon &icon,
    const QString &text)
    : ProjectNavigatorItem(mainWindow, icon, text)
{
    // Input widgets
    DoubleRangeSliderWidget::create(descriptorInput_,
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
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(descriptorInput_);
    mainLayout->addStretch();

    mainLayout_->addLayout(mainLayout);
    setLayout(mainLayout_);

    // Data
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));
}

void ProjectNavigatorItemDescriptor::slotUpdate(
    void *sender,
    const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

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
    descriptorRange_.setMinimumValue(
        static_cast<float>(descriptorInput_->minimumValue()) * 0.0039216F);
    descriptorInputChanged();
}

void ProjectNavigatorItemDescriptor::slotRangeIntermediateMaximumValue()
{
    descriptorRange_.setMaximumValue(
        static_cast<float>(descriptorInput_->maximumValue()) * 0.0039216F);
    descriptorInputChanged();
}

void ProjectNavigatorItemDescriptor::descriptorInputChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setDescriptorRange(descriptorRange_);
    mainWindow_->updateFilter();
}
bool ProjectNavigatorItemDescriptor::isFilterEnabled() const
{
    return descriptorRange_.isEnabled();
}

void ProjectNavigatorItemDescriptor::setFilterEnabled(bool b)
{
    descriptorRange_.setEnabled(b);
    descriptorInputChanged();
}
