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
#include <Core.hpp>
#include <MainWindow.hpp>

// Include Qt.
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>

// Include local.
#define LOG_MODULE_NAME "FilterManagementStatusTreeWidget"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

FilterManagementStatusTreeWidget::FilterManagementStatusTreeWidget(
    MainWindow *mainWindow)
    : QWidget(mainWindow),
      mainWindow_(mainWindow)
{
    LOG_DEBUG(<< "Create.");

    // Status map.
    statusMap_ = createMap();

    // Layout.
    mainLayout_ = new QVBoxLayout;
    mainLayout_->setContentsMargins(0, 0, 0, 0);

    // Checkbox list
    createCheckBoxList();

    setLayout(mainLayout_);
}

std::map<size_t, FilterManagementStatusTreeWidget::Status>
FilterManagementStatusTreeWidget::createMap()
{
    std::map<size_t, Status> statusMap;

    Editor *editor = &mainWindow_->editor();
    const ManagementStatusList &statusList = editor->managementStatusList();

    for (size_t i = 0; i < statusList.size(); i++)
    {
        const ManagementStatus &status = statusList[i];
        FilterManagementStatusTreeWidget::Status statusItem;
        statusItem.statusId = status.id;
        statusItem.label = QString::fromStdString(status.label);

        statusMap[i] = statusItem;
    }

    return statusMap;
}

void FilterManagementStatusTreeWidget::createCheckBoxList()
{
    LOG_DEBUG(<< "Create check box list.");

    // Delete.
    while (QLayoutItem *item = mainLayout_->takeAt(0))
    {
        if (QWidget *w = item->widget())
        {
            w->deleteLater();
        }

        delete item;
    }

    checkboxList_.clear();

    // Checkbox list
    checkboxList_.resize(statusMap_.size());
    for (size_t i = 0; i < checkboxList_.size(); i++)
    {
        checkboxList_[i] = new QCheckBox;
        checkboxList_[i]->setChecked(false);
        auto label = core().translate(statusMap_[i].label.toStdString());
        checkboxList_[i]->setText(QString::fromStdString(label));
        connect(checkboxList_[i],
                SIGNAL(clicked(bool)),
                this,
                SLOT(slotSetCheckbox(bool)));
    }

    // Layout.
    mainLayout_->addWidget(new QLabel(tr("Selected trees:")));
    for (size_t i = 0; i < checkboxList_.size(); i++)
    {
        mainLayout_->addWidget(checkboxList_[i]);
    }
    mainLayout_->addStretch();
}

void FilterManagementStatusTreeWidget::updateCheckBoxList()
{
    std::map<size_t, Status> statusMap = createMap();

    if (statusMap != statusMap_)
    {
        LOG_DEBUG(<< "Update: The status list is different.");
        createCheckBoxList();
    }
    else
    {
        LOG_DEBUG(<< "Update: The status list is the same.");
    }
}

void FilterManagementStatusTreeWidget::slotSetCheckbox(bool b)
{
    LOG_DEBUG(<< "Checkbox clicked.");
    (void)b;

    QObject *obj = sender();
    for (size_t i = 0; i < checkboxList_.size(); i++)
    {
        if (obj == checkboxList_[i])
        {
            LOG_DEBUG(<< "Checkbox pos <" << i << "> sender.");
            checkboxList_[i]->setChecked(true);
            setCheckbox(i);
        }
        else
        {
            LOG_DEBUG(<< "Checkbox pos <" << i << "> not sender.");
            checkboxList_[i]->setChecked(false);
        }
    }
}

void FilterManagementStatusTreeWidget::setCheckbox(size_t idx)
{
    LOG_DEBUG(<< "Set checkbox index <" << idx << "> to selected segments.");

    mainWindow_->suspendThreads();
    Editor *editor = &mainWindow_->editor();
    Segments segments = editor->segments();

    size_t nSelected = 0;
    for (size_t i = 0; i < segments.size(); i++)
    {
        if (segments[i].selected)
        {
            segments[i].managementStatusId = statusMap_[idx].statusId;
            nSelected++;
        }
    }

    if (nSelected == 0)
    {
        return;
    }

    editor->setSegments(segments);
    mainWindow_->update(this,
                        {Editor::TYPE_SEGMENT, Editor::TYPE_MANAGEMENT_STATUS});
}

void FilterManagementStatusTreeWidget::setSegment(const Segment &segment)
{
    LOG_DEBUG(<< "Set segment id <" << segment.id << ">.");

    segment_ = segment;

    updateCheckBoxList();

    for (size_t i = 0; i < checkboxList_.size(); i++)
    {
        if (statusMap_[i].statusId == segment_.managementStatusId)
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
