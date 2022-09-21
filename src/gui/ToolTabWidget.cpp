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

/** @file ToolTabWidget.cpp */

#include <MainWindow.hpp>
#include <ToolTabWidget.hpp>

#include <flowlayout.h>

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#define MODULE_NAME "ToolTabWidget"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

ToolTabWidget::ToolTabWidget()
    : QWidget(),
      toolBox_(nullptr),
      icon_(nullptr),
      label_(nullptr),
      mainLayout_(nullptr)
{
    LOG_DEBUG_LOCAL("");
}

void ToolTabWidget::addTab(QWidget *widget,
                           const QIcon &icon,
                           const QString &label)
{
    LOG_DEBUG_LOCAL(label.toStdString());

    // Create tool button
    QToolButton *toolButton;

    MainWindow::createToolButton(&toolButton,
                                 label,
                                 label,
                                 icon,
                                 this,
                                 SLOT(slotToolButton()));

    toolButton->setAutoRaise(false);
    toolButton->setCheckable(true);

    // Register new tab
    toolButtonList_.push_back(toolButton);
    tabList_.push_back(widget);

    // Insert new tab
    if (!toolBox_)
    {
        // Create widgets
        toolBox_ = new FlowLayout;
        toolBox_->addWidget(toolButton);
        toolBox_->setContentsMargins(1, 1, 1, 1);

        QFrame *toolBoxFrame = new QFrame;
        toolBoxFrame->setFrameStyle(QFrame::Box | QFrame::Plain);
        toolBoxFrame->setLineWidth(1);
        toolBoxFrame->setContentsMargins(1, 1, 1, 1);
        toolBoxFrame->setLayout(toolBox_);

        // Title
        icon_ = new QLabel;
        icon_->setPixmap(icon.pixmap(icon.actualSize(QSize(16, 16))));

        label_ = new QLabel;
        label_->setText(label);

        QHBoxLayout *titleBar = new QHBoxLayout;
        titleBar->addWidget(icon_);
        titleBar->addWidget(label_);
        titleBar->addStretch();

        // The first tab is on
        toolButton->setChecked(true);
        widget->setVisible(true);

        // Create layout
        mainLayout_ = new QVBoxLayout;
        mainLayout_->setContentsMargins(1, 1, 1, 1);
        mainLayout_->addWidget(toolBoxFrame);
        mainLayout_->addLayout(titleBar);
        mainLayout_->addWidget(widget);
        setLayout(mainLayout_);
    }
    else
    {
        // The other tabs are off
        toolButton->setChecked(false);
        widget->setVisible(false);

        // Extend layout
        toolBox_->addWidget(toolButton);
        mainLayout_->addWidget(widget);
    }
}

void ToolTabWidget::slotToolButton()
{
    LOG_DEBUG_LOCAL("");

    QObject *obj = sender();

    for (size_t i = 0; i < toolButtonList_.size(); i++)
    {
        if (toolButtonList_[i] != obj)
        {
            LOG_DEBUG_LOCAL("hide <" << i << ">");
            toolButtonList_[i]->setChecked(false);
            tabList_[i]->setVisible(false);
        }
    }

    for (size_t i = 0; i < toolButtonList_.size(); i++)
    {
        if (toolButtonList_[i] == obj)
        {
            LOG_DEBUG_LOCAL("show <" << i << ">");
            const QIcon icon = toolButtonList_[i]->icon();
            icon_->setPixmap(icon.pixmap(icon.actualSize(QSize(16, 16))));
            label_->setText(toolButtonList_[i]->text());
            toolButtonList_[i]->setChecked(true);
            tabList_[i]->setVisible(true);
            break;
        }
    }
}
