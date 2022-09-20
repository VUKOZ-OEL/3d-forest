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
      toolBar_(nullptr),
      mainLayout_(nullptr)
{
    LOG_DEBUG_LOCAL("");
}

void ToolTabWidget::addTab(QWidget *widget,
                           const QIcon &icon,
                           const QString &label)
{
    LOG_DEBUG_LOCAL(label.toStdString());

    QToolButton *toolButton;

    MainWindow::createToolButton(&toolButton,
                                 label,
                                 label,
                                 icon,
                                 this,
                                 SLOT(slotToolButton()));

    toolButton->setAutoRaise(false);
    toolButton->setCheckable(true);

    toolButtonList_.push_back(toolButton);

    tabList_.push_back(widget);

    if (!toolBar_)
    {
        toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolButton->setChecked(true);

        widget->setVisible(true);

        toolBar_ = new QToolBar;
        toolBar_->setIconSize(
            QSize(MainWindow::ICON_SIZE_TEXT, MainWindow::ICON_SIZE_TEXT));
        toolBar_->setStyleSheet("QToolBar {border-bottom: 1px solid gray;}");

        // Layout
        mainLayout_ = new QVBoxLayout;
        mainLayout_->setContentsMargins(1, 1, 1, 1);
        mainLayout_->addWidget(toolBar_);
        mainLayout_->addWidget(widget);
        setLayout(mainLayout_);
    }
    else
    {
        toolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        toolButton->setChecked(false);
        widget->setVisible(false);
        mainLayout_->addWidget(widget);
    }

    toolBar_->addWidget(toolButton);
    toolBar_->addSeparator();
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
            toolButtonList_[i]->setToolButtonStyle(Qt::ToolButtonIconOnly);
            toolButtonList_[i]->setChecked(false);
            tabList_[i]->setVisible(false);
        }
    }

    for (size_t i = 0; i < toolButtonList_.size(); i++)
    {
        if (toolButtonList_[i] == obj)
        {
            LOG_DEBUG_LOCAL("show <" << i << ">");
            toolButtonList_[i]->setToolButtonStyle(
                Qt::ToolButtonTextBesideIcon);
            toolButtonList_[i]->setChecked(true);
            tabList_[i]->setVisible(true);
            break;
        }
    }
}
