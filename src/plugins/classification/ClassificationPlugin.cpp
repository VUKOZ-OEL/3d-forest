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
#include <ThemeIcon.hpp>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDockWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#define ICON(name) (ThemeIcon(":/classification/", name))
#define CLASSIFICATION_PLUGIN_NAME "Classification"

#define LOG_DEBUG_LOCAL(msg)
//#define LOG_DEBUG_LOCAL(msg) LOG_MODULE("ClassificationPlugin", msg)

/** Classification Window. */
class ClassificationWindow : public QDockWidget
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
    QSpinBox *nPointsSpinBox_;
    QSpinBox *lengthSpinBox_;
    QSpinBox *rangeSpinBox_;
    QSpinBox *angleSpinBox_;
    QCheckBox *liveCheckBox_;
    QPushButton *applyButton_;
};

ClassificationWindow::ClassificationWindow(MainWindow *mainWindow)
    : QDockWidget(mainWindow),
      mainWindow_(mainWindow),
      classification_(&mainWindow->editor())
{
    LOG_DEBUG_LOCAL("");

    // Widgets
    nPointsSpinBox_ = new QSpinBox;
    nPointsSpinBox_->setRange(1000, 1000000);
    nPointsSpinBox_->setValue(100000);
    nPointsSpinBox_->setSingleStep(1);

    lengthSpinBox_ = new QSpinBox;
    lengthSpinBox_->setRange(1, 100);
    lengthSpinBox_->setValue(1);
    lengthSpinBox_->setSingleStep(1);

    rangeSpinBox_ = new QSpinBox;
    rangeSpinBox_->setRange(1, 100);
    rangeSpinBox_->setValue(15);
    rangeSpinBox_->setSingleStep(1);

    angleSpinBox_ = new QSpinBox;
    angleSpinBox_->setRange(1, 89);
    angleSpinBox_->setValue(60);
    angleSpinBox_->setSingleStep(1);

    liveCheckBox_ = new QCheckBox;
    liveCheckBox_->setChecked(false);
    liveCheckBox_->setEnabled(false);

    applyButton_ = new QPushButton(tr("Classify"));
    applyButton_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(applyButton_, SIGNAL(clicked()), this, SLOT(slotApply()));

    // Layout
    QGridLayout *groupBoxLayout = new QGridLayout;
    groupBoxLayout->addWidget(new QLabel(tr("Points per cell")), 0, 0);
    groupBoxLayout->addWidget(nPointsSpinBox_, 0, 1);
    groupBoxLayout->addWidget(new QLabel(tr("Cell min length (%)")), 1, 0);
    groupBoxLayout->addWidget(lengthSpinBox_, 1, 1);
    groupBoxLayout->addWidget(new QLabel(tr("Ground level (%)")), 2, 0);
    groupBoxLayout->addWidget(rangeSpinBox_, 2, 1);
    groupBoxLayout->addWidget(new QLabel(tr("Ground angle (deg)")), 3, 0);
    groupBoxLayout->addWidget(angleSpinBox_, 3, 1);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(liveCheckBox_);
    hbox->addWidget(new QLabel(tr("Live")));
    hbox->addStretch();
    hbox->addWidget(applyButton_, 0, Qt::AlignRight);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(groupBoxLayout);
    vbox->addSpacing(10);
    vbox->addLayout(hbox);

    // Dock
    widget_ = new QWidget;
    widget_->setLayout(vbox);
    widget_->setFixedHeight(180);
    setWidget(widget_);
    setWindowTitle(QObject::tr(CLASSIFICATION_PLUGIN_NAME));
    setFloating(true);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, this);
}

void ClassificationWindow::slotApply()
{
    LOG_DEBUG_LOCAL("");

    mainWindow_->suspendThreads();

    size_t pointsPerCell = static_cast<size_t>(nPointsSpinBox_->value());
    double cellLengthMinPercent = static_cast<double>(lengthSpinBox_->value());
    double groundErrorPercent = static_cast<double>(rangeSpinBox_->value());
    double angleDeg = static_cast<double>(angleSpinBox_->value());

    int maximum = classification_.start(pointsPerCell,
                                        cellLengthMinPercent,
                                        groundErrorPercent,
                                        angleDeg);

    LOG_DEBUG_LOCAL("maximum <" << maximum << ">");

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
        LOG_DEBUG_LOCAL("Processing <" << p << "> from <" << maximum << ">");
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
