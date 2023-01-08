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

/** @file ClassificationPlugin.cpp */

#include <Classification.hpp>
#include <ClassificationPlugin.hpp>
#include <MainWindow.hpp>
#include <SliderWidget.hpp>
#include <ThemeIcon.hpp>

#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>

#define ICON(name) (ThemeIcon(":/classification/", name))
#define CLASSIFICATION_PLUGIN_NAME "Classification"

#define MODULE_NAME "ClassificationPlugin"
#define LOG_DEBUG_LOCAL(msg)
// #define LOG_DEBUG_LOCAL(msg) LOG_MESSAGE(LOG_DEBUG, MODULE_NAME, msg)

/** Classification Window. */
class ClassificationWindow : public QDialog
{
    Q_OBJECT

public:
    ClassificationWindow(MainWindow *mainWindow);

protected slots:
    void slotApply();

protected:
    MainWindow *mainWindow_;
    Classification classification_;

    QWidget *widget_;
    SliderWidget *nPointsSlider_;
    SliderWidget *lengthSlider_;
    SliderWidget *rangeSlider_;
    SliderWidget *angleSlider_;
    QPushButton *applyButton_;
};

ClassificationWindow::ClassificationWindow(MainWindow *mainWindow)
    : QDialog(mainWindow),
      mainWindow_(mainWindow),
      classification_(&mainWindow->editor())
{
    LOG_DEBUG_LOCAL();

    // Widgets
    SliderWidget::create(nPointsSlider_,
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

    SliderWidget::create(lengthSlider_,
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

    SliderWidget::create(rangeSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Ground level"),
                         tr("Ground level maximum"),
                         tr("%"),
                         1,
                         1,
                         100,
                         15);

    SliderWidget::create(angleSlider_,
                         this,
                         nullptr,
                         nullptr,
                         tr("Ground angle"),
                         tr("Ground angle"),
                         tr("deg"),
                         1,
                         1,
                         89,
                         60);

    // Settings layout
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->addWidget(nPointsSlider_);
    settingsLayout->addWidget(lengthSlider_);
    settingsLayout->addWidget(rangeSlider_);
    settingsLayout->addWidget(angleSlider_);
    settingsLayout->addStretch();

    // Buttons
    applyButton_ = new QPushButton(tr("Classify"));
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
    setWindowTitle(tr(CLASSIFICATION_PLUGIN_NAME));
    setWindowIcon(ICON("soil"));
    setMaximumHeight(height());
    setModal(true);
}

void ClassificationWindow::slotApply()
{
    LOG_DEBUG_LOCAL();

    mainWindow_->suspendThreads();

    size_t pointsPerCell = static_cast<size_t>(nPointsSlider_->value());
    double cellLengthMinPercent = static_cast<double>(lengthSlider_->value());
    double groundErrorPercent = static_cast<double>(rangeSlider_->value());
    double angleDeg = static_cast<double>(angleSlider_->value());

    int maximum = classification_.start(pointsPerCell,
                                        cellLengthMinPercent,
                                        groundErrorPercent,
                                        angleDeg);

    LOG_DEBUG_LOCAL(<< "maximum <" << maximum << ">");

    QProgressDialog progressDialog(mainWindow_);
    progressDialog.setCancelButtonText(QObject::tr("&Cancel"));
    progressDialog.setRange(0, maximum);
    progressDialog.setWindowTitle(QObject::tr(CLASSIFICATION_PLUGIN_NAME));
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

        classification_.step();
    }

    classification_.clear();

    progressDialog.setValue(progressDialog.maximum());

    mainWindow_->editor().viewports().setState(Page::STATE_READ);

    mainWindow_->resumeThreads();
}

ClassificationPlugin::ClassificationPlugin()
    : mainWindow_(nullptr),
      dockWindow_(nullptr)
{
}

void ClassificationPlugin::initialize(MainWindow *mainWindow)
{
    mainWindow_ = mainWindow;

    mainWindow_->createAction(nullptr,
                              "Utilities",
                              "Utilities",
                              tr("Classification"),
                              tr("Classify points to ground and unassigned"),
                              ICON("soil"),
                              this,
                              SLOT(slotPlugin()));
}

void ClassificationPlugin::slotPlugin()
{
    // Create GUI only when this plugin is used for the first time
    if (!dockWindow_)
    {
        dockWindow_ = new ClassificationWindow(mainWindow_);
    }

    dockWindow_->show();
    dockWindow_->raise();
    dockWindow_->activateWindow();
}

// Q_OBJECT is used in this cpp file
#include "ClassificationPlugin.moc"
