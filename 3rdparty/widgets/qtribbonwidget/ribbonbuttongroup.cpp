/*
 * Copyright (C) Martijn Koopman
 * All Rights Reserved
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 */

/*
    Modifications Copyright 2020 VUKOZ

    Removed setIconSize().
    Updated setMinimumSize().
*/

#include "ribbonbuttongroup.h"
#include "ui_ribbonbuttongroup.h"

#include <QToolButton>

RibbonButtonGroup::RibbonButtonGroup(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::RibbonButtonGroup)
  , m_title("")
{
  ui->setupUi(this);
}

RibbonButtonGroup::~RibbonButtonGroup()
{
  delete ui;
}

void RibbonButtonGroup::setTitle(const QString &title)
{
  m_title = title;
  //ui->label->setText(m_title);
}

QString RibbonButtonGroup::title() const
{
  return m_title;
}

int RibbonButtonGroup::buttonCount() const
{
  return ui->horizontalLayout->count();
}

void RibbonButtonGroup::addButton(QToolButton *button)
{
  button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  button->setMinimumSize(50, 50);
  button->setMaximumSize(70, 70);
  button->setAutoRaise(true);
  button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  // QFont font = button->font();
  // font.setPointSize(font.pointSize() - 1);
  // button->setFont(font);

  ui->horizontalLayout->addWidget(button);
}

void RibbonButtonGroup::removeButton(QToolButton *button)
{
  /// \todo What happens if button is not part of the layout?
  ui->horizontalLayout->removeWidget(button);
}
