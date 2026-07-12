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

/** @file ProgressActionDialog.cpp */

// Include std.
#include <cinttypes>

// Include 3D Forest.
#include <ProgressActionDialog.hpp>
#include <ProgressActionInterface.hpp>
#include <Application.hpp>
#include <GridLayout.hpp>
#include <HBoxLayout.hpp>
#include <Label.hpp>
#include <ProgressBar.hpp>
#include <PushButton.hpp>
#include <VBoxLayout.hpp>
#include <Time.hpp>

// Include local.
#define LOG_MODULE_NAME "ProgressActionDialog"
#include <Log.hpp>

#define PROGRESS_DIALOG_ETA_MIN 5.0

bool ProgressActionDialog::run(Application *app,
                         const std::string &title,
                         ProgressActionInterface *progressAction)
{
    LOG_DEBUG(<< "Run progress dialog <" << title << ">.");
    ProgressActionDialog *progressDialog = new ProgressActionDialog(app);
    progressDialog->setWindowTitle(title);
    return progressDialog->run(progressAction);
}

ProgressActionDialog::ProgressActionDialog(Application *app)
    : Dialog(app),
      canceledFlag_(false)
{
    LOG_DEBUG(<< "Create progress dialog <" << std::string(title) << ">.");

    // Create modal progress dialog with custom progress bar.
    // Custom progress bar allows to display percentage with fractional part.
    setWindowTitle(windowTitle_);
    setWindowModality(Ui::WindowModal);

    // Progress info labels.
    progressStepsLabel_ = new Label(tr(" "));
    progressStepLabel_ = new Label(tr(" "));
    etaLabel_ = new Label(tr(" "));

    GridLayout *progressLabelsLayout = new GridLayout;
    progressLabelsLayout->addWidget(progressStepsLabel_, 0, 0);
    progressLabelsLayout->addWidget(etaLabel_, 0, 1);
    progressLabelsLayout->addWidget(progressStepLabel_, 0, 2);

    // Progress bar.
    progressBar_ = new ProgressBar;
    progressBar_->setRange(0, 100);
    progressBar_->setValue(progressBar_->minimum());

    // Buttons.
    cancelButton_ = new PushButton(tr("Cancel"));
    cancelButton_->clicked.connect([this]()
    {
        slotCancel();
    });

    HBoxLayout *buttonsLayout = new HBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelButton_);

    // Main layout.
    VBoxLayout *dialogLayout = new VBoxLayout;
    dialogLayout->addLayout(progressLabelsLayout);
    dialogLayout->addWidget(progressBar_);
    dialogLayout->addSpacing(10);
    dialogLayout->addLayout(buttonsLayout);
    dialogLayout->addStretch();

    setLayout(dialogLayout);
}

void ProgressActionDialog::slotCancel()
{
    LOG_DEBUG(<< "Cancel progress dialog.");
    canceledFlag_ = true;
}

void ProgressActionDialog::closeEvent(CloseEvent *event)
{
    LOG_DEBUG(<< "Close progress dialog.");
    slotCancel();
    Dialog::closeEvent(event);
}

bool ProgressActionDialog::run(ProgressActionInterface *progressAction)
{
    LOG_DEBUG(<< "Run progress dialog.");

    initializeLabels(progressAction);

    // Show the progress dialog.
    show();

    // Process all iteration steps.
    while (!progressAction->end())
    {
        // Update current progress.
        updateLabels(progressAction);

        // Keep processing qt events.
        app_->processEvents();

        // Canceled?
        if (canceledFlag_)
        {
            LOG_DEBUG(<< "Canceled.");
            hide();
            return false;
        }

        // Process the action for few milliseconds.
        progressAction->next();
    }

    hide();

    return true;
}

void ProgressActionDialog::initializeLabels(ProgressActionInterface *progressAction)
{
    if (progressAction->progressMaximumSteps() > 1)
    {
        progressStepsLabel_->setVisible(true);
    }
    else
    {
        progressStepsLabel_->setVisible(false);
    }

    etaStartTime_ = Time::realTime();
    etaStartPercent_ = 0.0;
}

void ProgressActionDialog::updateLabels(ProgressActionInterface *progressAction)
{
    char buffer[64];

    double progressPercent = progressAction->progressPercent();

    // Progress bar.
    progressBar_->setValue(static_cast<int>(progressPercent));

    // Steps.
    if (progressAction->progressMaximumSteps() > 1)
    {
        std::snprintf(buffer,
                      sizeof(buffer),
                      "Step %zu of %zu",
                      progressAction->progressValueSteps() + 1U,
                      progressAction->progressMaximumSteps());

        progressStepsLabel_->setText(buffer);
    }

    // Current step.
    if (progressAction->progressMaximumStep() != ProgressCounter::npos)
    {
        std::snprintf(buffer,
                      sizeof(buffer),
                      "%" PRIu64 " / %" PRIu64,
                      progressAction->progressValueStep() + 1U,
                      progressAction->progressMaximumStep());

        progressStepLabel_->setText(buffer);
    }

    // ETA.
    double etaCurrentTime = Time::realTime();
    double etaTimeDiff = etaCurrentTime - etaStartTime_;
    double etaProgressDiff = progressPercent - etaStartPercent_;
    double etaProgressRemains = 100.0 - progressPercent;

    int etaTime;
    bool etaTimeAvailable;

    if (etaTimeDiff > PROGRESS_DIALOG_ETA_MIN && etaProgressDiff > 0.0 &&
        etaProgressRemains > 0.0)
    {
        etaTimeAvailable = true;
        etaTime = static_cast<int>((etaTimeDiff / etaProgressDiff) *
                                   etaProgressRemains);
    }
    else
    {
        etaTimeAvailable = false;
        etaTime = 0;
    }

    int etaTimeH = etaTime / 3600;
    int etaTimeM = (etaTime - (etaTimeH * 3600)) / 60;
    int etaTimeS = etaTime - (etaTimeH * 3600) - (etaTimeM * 60);

    if (etaTimeH > 0)
    {
        std::snprintf(buffer, sizeof(buffer), "(%d h)", etaTimeH);
    }
    else if (etaTimeM > 0)
    {
        std::snprintf(buffer, sizeof(buffer), "(%d m)", etaTimeM);
    }
    else
    {
        if (etaTimeAvailable)
        {
            std::snprintf(buffer, sizeof(buffer), "(%d s)", etaTimeS);
        }
        else
        {
            buffer[0] = 0;
        }
    }

    etaLabel_->setText(buffer);
}
