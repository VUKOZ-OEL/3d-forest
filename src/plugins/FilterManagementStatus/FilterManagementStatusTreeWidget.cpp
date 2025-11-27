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

/** @file FilterManagementStatusTreeWidget.cpp */

// Include 3D Forest.
#include <FilterManagementStatusTreeWidget.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>

// Include local.
#define LOG_MODULE_NAME "FilterManagementStatusTreeWidget"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

FilterManagementStatusTreeWidget::FilterManagementStatusTreeWidget(MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Create.");

    // Status map.
    Editor *editor = &mainWindow_->editor();
    const ManagementStatusList &statusList = editor->managementStatusList();
    std::vector<QString> str;
    for (size_t i = 0; i < statusList.size(); i++)
    {
        const ManagementStatus &status = statusList[i];
        str.push_back(QString::fromStdString(status.label));
        statusMap_[i] = status.id;
    }

    // Checkbox list
    checkboxList_.resize(str.size());
    for (size_t i = 0; i < checkboxList_.size(); i++)
    {
        checkboxList_[i] = new QCheckBox;
        checkboxList_[i]->setChecked(false);
        checkboxList_[i]->setText(str[i]);
        connect(checkboxList_[i],
                SIGNAL(stateChanged(int)),
                this,
                SLOT(slotSetCheckbox(int)));
    }

    // Layout.
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(new QLabel(tr("Selected tree:")));
    for (size_t i = 0; i < checkboxList_.size(); i++)
    {
        mainLayout->addWidget(checkboxList_[i]);
    }
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void FilterManagementStatusTreeWidget::slotSetCheckbox(int v)
{
    LOG_DEBUG(<< "Checkbox changed.");
    (void)v;
    return;

    // for (size_t i = 0; i < checkboxList_.size(); i++)
    // {
    //     disconnect(checkboxList_[i], SIGNAL(stateChanged(int)), 0, 0);
    // }

    QObject *obj = sender();
    for (size_t i = 0; i < checkboxList_.size(); i++)
    {
        if (obj == checkboxList_[i])
        {
            LOG_DEBUG(<< "Checkbox pos <" << i << ">.");
            checkboxList_[i]->setChecked(true);
            // setCheckbox(i);
        }
        else
        {
            checkboxList_[i]->setChecked(false);
        }
    }

    // for (size_t i = 0; i < checkboxList_.size(); i++)
    // {
    //     connect(checkboxList_[i],
    //             SIGNAL(stateChanged(int)),
    //             this,
    //             SLOT(slotSetCheckbox(int)));
    // }
}

void FilterManagementStatusTreeWidget::setCheckbox(size_t idx)
{
    if (segment_.id == 0)
    {
        return;
    }

    mainWindow_->suspendThreads();
    Editor *editor = &mainWindow_->editor();
    Segments segments = editor->segments();

    for (size_t i = 0; i < segments.size(); i++)
    {
        if (segments[i].id == segment_.id)
        {
            segments[i].managementStatusId = statusMap_[idx];
            break;
        }
    }

    editor->setSegments(segments);
    mainWindow_->update({Editor::TYPE_SEGMENT, Editor::TYPE_MANAGEMENT_STATUS});
}

void FilterManagementStatusTreeWidget::setSegment(const Segment &segment)
{
    LOG_DEBUG(<< "Set segment id <" << segment.id << ">.");

    segment_ = segment;

    for (size_t i = 0; i < checkboxList_.size(); i++)
    {
        if (statusMap_[i] == segment_.managementStatusId)
        {
            checkboxList_[i]->setChecked(true);
        }
        else
        {
            checkboxList_[i]->setChecked(false);
        }
    }
}

void FilterManagementStatusTreeWidget::clear()
{
    LOG_DEBUG(<< "Clear data.");

    segment_.id = 0;

    for (size_t i = 0; i < checkboxList_.size(); i++)
    {
        checkboxList_[i]->setChecked(false);
    }
}
