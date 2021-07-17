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

/** @file WindowDock.cpp */

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QStyle>
#include <QToolButton>
#include <WindowDock.hpp>

WindowDock::WindowDock(QMainWindow *parent)
    : QDockWidget(parent),
      mainWindow_(parent)
{
    // Title bar
    QWidget *bar = new QWidget;
    bar->setBackgroundRole(QPalette::Dark);
    bar->setAutoFillBackground(true);

    // Icons
    QStyle *style = bar->style();

    QIcon iconClose;
    iconClose = style->standardIcon(QStyle::SP_TitleBarCloseButton, 0, bar);

    QIcon iconMax;
    iconMax = style->standardIcon(QStyle::SP_TitleBarMaxButton, 0, bar);

    // Widgets
    windowIcon_ = new QLabel;

    windowTitle_ = new QLabel;
    windowTitle_->setText("Title");

    windowButtonCollapse_ = new QToolButton;
    windowButtonCollapse_->setIcon(iconMax);
    windowButtonCollapse_->setIconSize(QSize(10, 10));
    connect(windowButtonCollapse_,
            SIGNAL(clicked()),
            this,
            SLOT(windowCollapse()));

    windowButtonClose_ = new QToolButton;
    windowButtonClose_->setIcon(iconClose);
    windowButtonClose_->setIconSize(QSize(10, 10));
    connect(windowButtonClose_, SIGNAL(clicked()), this, SLOT(windowClose()));

    // Layout
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(windowIcon_);
    layout->addSpacing(5);
    layout->addWidget(windowTitle_);
    layout->addStretch();
    layout->addWidget(windowButtonCollapse_);
    layout->addWidget(windowButtonClose_);
    layout->setContentsMargins(0, 0, 0, 0);
    bar->setLayout(layout);

    // Set title bar
    setTitleBarWidget(bar);

    connect(this,
            SIGNAL(topLevelChanged(bool)),
            this,
            SLOT(windowDockFloatEvent(bool)));
}

QMainWindow *WindowDock::mainWindow() const
{
    return mainWindow_;
}

void WindowDock::setWindowTitle(const QString &text)
{
    windowTitle_->setText(text);
    QDockWidget::setWindowTitle(text);
}

void WindowDock::setWindowIcon(const QIcon &icon)
{
    windowIcon_->setPixmap(icon.pixmap(16, 16));
}

void WindowDock::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    if (isFloating())
    {
        QPainter painter(this);
        painter.setPen(QColor(200, 200, 200));
        painter.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void WindowDock::windowDockFloatEvent(bool topLevel)
{
    if (topLevel)
    {
        // setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint);
    }
}

void WindowDock::windowCollapse()
{
    /** @todo Collapse dock functionality.

        This does not work with fixed sized widgets yet.
    */
#if 0
    if (widget()->isVisible())
    {
        widget()->setVisible(false);
    }
    else
    {
        widget()->setVisible(true);
    }
#endif
}

void WindowDock::windowClose()
{
    QWidget::close();
}
