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

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <flowlayout.h>

#define LOG_MODULE_NAME "ToolTabWidget"
#include <Log.hpp>

ToolTabWidget::ToolTabWidget()
    : QWidget(),
      toolBox_(nullptr),
      icon_(nullptr),
      label_(nullptr),
      mainLayout_(nullptr)
{
    LOG_DEBUG(<< "Called.");
}

void ToolTabWidget::addTab(QWidget *widget,
                           const QIcon &icon,
                           const QString &label)
{
    LOG_DEBUG(<< "Tab text <" << label.toStdString() << ">.");

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
        toolBox_ = new FlowLayout(1, 1, 1);
        toolBox_->addWidget(toolButton);
        toolBox_->setContentsMargins(2, 2, 2, 2);

        QFrame *toolBoxFrame = new QFrame;
        toolBoxFrame->setFrameStyle(QFrame::Box | QFrame::Plain);
        toolBoxFrame->setLineWidth(0);
        toolBoxFrame->setContentsMargins(1, 1, 1, 1);
        toolBoxFrame->setLayout(toolBox_);

        QColor toolBoxBackground = qApp->palette().color(QPalette::Midlight);
        QPalette toolBoxPalette = palette();
        toolBoxPalette.setColor(QPalette::Window, toolBoxBackground);
        toolBoxFrame->setPalette(toolBoxPalette);
        toolBoxFrame->setAutoFillBackground(true);

        // Title
        icon_ = new QLabel;
        icon_->setPixmap(icon.pixmap(icon.actualSize(QSize(16, 16))));
        icon_->setContentsMargins(1, 1, 1, 1);

        label_ = new QLabel;
        label_->setText(label);
        label_->setContentsMargins(1, 1, 1, 1);

        QHBoxLayout *titleBar = new QHBoxLayout;
        titleBar->addWidget(icon_);
        titleBar->addWidget(label_);
        titleBar->addStretch();
        titleBar->setContentsMargins(1, 1, 1, 1);

        QFrame *titleFrame = new QFrame;
        titleFrame->setFrameStyle(QFrame::Box | QFrame::Plain);
        titleFrame->setLineWidth(0);
        titleFrame->setContentsMargins(1, 1, 1, 1);
        titleFrame->setLayout(titleBar);

        // The first tab is on
        toolButton->setChecked(true);
        widget->setVisible(true);

        // Create layout
        mainLayout_ = new QVBoxLayout;
        mainLayout_->setContentsMargins(1, 1, 1, 1);
        mainLayout_->setSpacing(0);
        mainLayout_->addWidget(toolBoxFrame);
        mainLayout_->addWidget(titleFrame);
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
    LOG_DEBUG(<< "Called.");

    QObject *obj = sender();

    for (size_t i = 0; i < toolButtonList_.size(); i++)
    {
        if (toolButtonList_[i] != obj)
        {
            LOG_DEBUG(<< "Hide widget <" << i << ">.");
            toolButtonList_[i]->setChecked(false);
            tabList_[i]->setVisible(false);
        }
    }

    for (size_t i = 0; i < toolButtonList_.size(); i++)
    {
        if (toolButtonList_[i] == obj)
        {
            LOG_DEBUG(<< "Show widget <" << i << ">.");
            const QIcon icon = toolButtonList_[i]->icon();
            icon_->setPixmap(icon.pixmap(icon.actualSize(QSize(16, 16))));
            label_->setText(toolButtonList_[i]->text());
            toolButtonList_[i]->setChecked(true);
            tabList_[i]->setVisible(true);
            break;
        }
    }
}
