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

/** @file ElevationPlugin.cpp */

#include <Elevation.hpp>
#include <ElevationPlugin.hpp>
#include <MainWindow.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>

#define ICON(name) (ThemeIcon(":/elevation/", name))
#define ELEVATION_PLUGIN_NAME "Elevation"

#define MODULE_NAME "ElevationPlugin"
#define LOG_DEBUG_LOCAL(msg)
// #define LOG_DEBUG_LOCAL(msg) LOG_MESSAGE(LOG_DEBUG, MODULE_NAME, msg)

/** Elevation Window. */
class ElevationWindow : public QDialog
{
    Q_OBJECT

public:
    ElevationWindow(MainWindow *mainWindow);

protected slots:
    void slotApply();

protected:
    MainWindow *mainWindow_;
    Elevation elevation_;

    QWidget *widget_;
    SliderWidget *nPointsInput_;
    SliderWidget *lengthInput_;
    QPushButton *applyButton_;
};

ElevationWindow::ElevationWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      mainWindow_(mainWindow),
      elevation_(&mainWindow->editor())
{
    LOG_DEBUG_LOCAL();

    // Widgets
    SliderWidget::create(nPointsInput_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Points per cell"),
                         tr("Points per cell"),
                         tr("pt"),
                         1,
                         1000,
                         1000000,
                         10000);

    SliderWidget::create(lengthInput_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Cell min length"),
                         tr("Cell min length"),
                         tr("%"),
                         1,
                         1,
                         100,
                         5);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(nPointsInput_);
    settingsLayout->addWidget(lengthInput_);
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
    setWindowTitle(tr(ELEVATION_PLUGIN_NAME));
    setWindowIcon(ICON("elevation"));
    setMaximumHeight(height());
    setModal(true);
}

void ElevationWindow::slotApply()
{
    LOG_DEBUG_LOCAL();

    mainWindow_->suspendThreads();

    size_t pointsPerCell = static_cast<size_t>(nPointsInput_->value());
    double cellLengthMinPercent = static_cast<double>(lengthInput_->value());

    int maximum = elevation_.start(pointsPerCell, cellLengthMinPercent);
    LOG_DEBUG_LOCAL(<< "maximum <" << maximum << ">");

    QProgressDialog progressDialog(mainWindow_);
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, maximum);
    progressDialog.setWindowTitle(QObject::tr(ELEVATION_PLUGIN_NAME));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(0);
    progressDialog.show();

    for (int i = 0; i < maximum; i++)
    {
        // Update progress
        int p = i + 1;
        LOG_DEBUG_LOCAL(<< "Processing <" << p << "> from <" << maximum << ">");
        progressDialog.setValue(p);
        progressDialog.setLabelText(
            QObject::tr("Processing %1 of %n...", nullptr, maximum).arg(p));

        QCoreApplication::processEvents();
        if (progressDialog.wasCanceled())
        {
            break;
        }

        elevation_.step();
    }

    elevation_.clear();

    progressDialog.setValue(progressDialog.maximum());

    mainWindow_->update({Editor::TYPE_ELEVATION});
}

ElevationPlugin::ElevationPlugin() : mainWindow_(nullptr), dockWindow_(nullptr)
{
}

void ElevationPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "Utilities",
                              "Utilities",
                              tr("Elevation"),
                              tr("Compute elevation above ground"
                                 " for each point"),
                              ICON("elevation"),
                              this,
                              SLOT(slotPlugin()));
}

void ElevationPlugin::slotPlugin()
{
    // Create GUI only when this plugin is used for the first time
    if (!dockWindow_)
    {
        dockWindow_ = new ElevationWindow(mainWindow_);
    }

    dockWindow_->show();
    dockWindow_->raise();
    dockWindow_->activateWindow();
}

// Q_OBJECT is used in this cpp file
#include "ElevationPlugin.moc"
