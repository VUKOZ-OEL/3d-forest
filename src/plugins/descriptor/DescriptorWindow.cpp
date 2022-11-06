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

/** @file DescriptorWindow.cpp */

#include <DescriptorWindow.hpp>
#include <MainWindow.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>

#define MODULE_NAME "Descriptor"
#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE(MODULE_NAME, msg)

#define ICON(name) (ThemeIcon(":/descriptor/", name))
#define DESCRIPTOR_PLUGIN_NAME "Descriptor"

DescriptorWindow::DescriptorWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      mainWindow_(mainWindow),
      descriptor_(&mainWindow->editor())
{
    LOG_DEBUG_LOCAL("");

    // Widgets
    SliderWidget::create(radius_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Radius"),
                         tr("Radius"),
                         tr("pt"),
                         1,
                         10,
                         1000,
                         1000);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(radius_);
    settingsLayout->addStretch();

    // Buttons
    applyButton_ = new QPushButton(tr("Compute"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Buttons layout
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(applyButton_);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(settingsLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

    // Dialog
    setLayout(mainLayout);
    setWindowTitle(tr(DESCRIPTOR_PLUGIN_NAME));
    setWindowIcon(ICON("descriptor"));
    setMaximumHeight(height());
    setModal(true);
}

void DescriptorWindow::slotApply()
{
    LOG_DEBUG_LOCAL("");

    mainWindow_->suspendThreads();

    double radius = static_cast<double>(radius_->value());

    int maximum = descriptor_.start(radius);
    LOG_DEBUG_LOCAL("maximum <" << maximum << ">");

    QProgressDialog progressDialog(mainWindow_);
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, maximum);
    progressDialog.setWindowTitle(QObject::tr(DESCRIPTOR_PLUGIN_NAME));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(0);
    progressDialog.show();

    for (int i = 0; i < maximum; i++)
    {
        // Update progress
        int p = i + 1;
        LOG_DEBUG_LOCAL("Processing <" << p << "> from <" << maximum << ">");
        progressDialog.setValue(p);
        progressDialog.setLabelText(
            QObject::tr("Processing %1 of %n...", nullptr, maximum).arg(p));

        QCoreApplication::processEvents();
        if (progressDialog.wasCanceled())
        {
            break;
        }

        descriptor_.step();
    }

    descriptor_.clear();

    progressDialog.setValue(progressDialog.maximum());

    mainWindow_->update({Editor::TYPE_DESCRIPTOR});
}
