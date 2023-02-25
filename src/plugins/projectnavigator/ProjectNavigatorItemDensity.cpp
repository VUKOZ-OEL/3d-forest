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

/** @file ProjectNavigatorItemDensity.cpp */

#include <DoubleRangeSliderWidget.hpp>
#include <MainWindow.hpp>
#include <ProjectNavigatorItemDensity.hpp>

#include <QVBoxLayout>

ProjectNavigatorItemDensity::ProjectNavigatorItemDensity(MainWindow *mainWindow,
                                                         const QIcon &icon,
                                                         const QString &text)
    : ProjectNavigatorItem(mainWindow, icon, text)
{
    // Input widgets
    DoubleRangeSliderWidget::create(densityInput_,
                                    this,
                                    SLOT(slotRangeIntermediateMinimumValue()),
                                    SLOT(slotRangeIntermediateMaximumValue()),
                                    tr("Density"),
                                    tr("Min-max density range filter"),
                                    tr("pt"),
                                    1,
                                    0,
                                    100,
                                    0,
                                    100);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(densityInput_);
    mainLayout->addStretch();

    mainLayout_->addLayout(mainLayout);
    setLayout(mainLayout_);

    // Data
    connect(mainWindow_,
            SIGNAL(signalUpdate(void *, const QSet<Editor::Type> &)),
            this,
            SLOT(slotUpdate(void *, const QSet<Editor::Type> &)));
}

void ProjectNavigatorItemDensity::slotUpdate(void *sender,
                                             const QSet<Editor::Type> &target)
{
    if (sender == this)
    {
        return;
    }

    if (target.empty() || target.contains(Editor::TYPE_DENSITY))
    {
        densityRange_ = mainWindow_->editor().descriptorRange();

        densityInput_->blockSignals(true);
        densityInput_->setMinimum(densityRange_.minimum() * 255.0);
        densityInput_->setMaximum(densityRange_.maximum() * 255.0);
        densityInput_->setMinimumValue(densityRange_.minimumValue() * 255.0);
        densityInput_->setMaximumValue(densityRange_.maximumValue() * 255.0);
        densityInput_->blockSignals(false);
    }
}

void ProjectNavigatorItemDensity::slotRangeIntermediateMinimumValue()
{
    densityRange_.setMinimumValue(densityInput_->minimumValue() * 0.0039216);
    densityInputChanged();
}

void ProjectNavigatorItemDensity::slotRangeIntermediateMaximumValue()
{
    densityRange_.setMaximumValue(densityInput_->maximumValue() * 0.0039216);
    densityInputChanged();
}

void ProjectNavigatorItemDensity::densityInputChanged()
{
    mainWindow_->suspendThreads();
    mainWindow_->editor().setDensityRange(densityRange_);
    mainWindow_->updateFilter();
}
bool ProjectNavigatorItemDensity::isFilterEnabled() const
{
    return densityRange_.isEnabled();
}

void ProjectNavigatorItemDensity::setFilterEnabled(bool b)
{
    densityRange_.setEnabled(b);
    densityInputChanged();
}
