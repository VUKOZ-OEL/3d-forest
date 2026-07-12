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

/** @file ProgressActionDialog.hpp */

#ifndef PROGRESS_ACTION_DIALOG_HPP
#define PROGRESS_ACTION_DIALOG_HPP

// Include std.
#include <string>

// Include 3D Forest.
#include <Dialog.hpp>
class Application;
class ProgressActionInterface;
class Label;
class ProgressBar;
class PushButton;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Progress Action Dialog. */
class EXPORT_UI_COMMON ProgressActionDialog : public Dialog
{
public:
    ProgressActionDialog(Application *app);
    virtual ~ProgressActionDialog();

    bool run(ProgressActionInterface *progressAction);

    static bool run(Application *app,
                    const std::string &title,
                    ProgressActionInterface *progressAction);

    void slotCancel();

    void closeEvent(CloseEvent *event) override;

private:
    bool canceledFlag_;
    double etaStartTime_;
    double etaStartPercent_;
    Label *progressStepsLabel_;
    Label *progressStepLabel_;
    Label *etaLabel_;
    ProgressBar *progressBar_;
    PushButton *cancelButton_;

    void initializeLabels(ProgressActionInterface *progressAction);
    void updateLabels(ProgressActionInterface *progressAction);
};

#include <WarningsEnable.hpp>

#endif /* PROGRESS_ACTION_DIALOG_HPP */
