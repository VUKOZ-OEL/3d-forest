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

/** @file ProgressDialog.hpp */

#ifndef PROGRESS_DIALOG_HPP
#define PROGRESS_DIALOG_HPP

class MainWindow;
class ProgressActionInterface;

#include <QDialog>
class QCloseEvent;
class QLabel;
class QProgressBar;
class QPushButton;

#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

/** Progress Dialog. */
class EXPORT_GUI ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    ProgressDialog(MainWindow *mainWindow, const char *title);

    bool run(ProgressActionInterface *progressAction);

    static bool run(MainWindow *mainWindow,
                    const char *title,
                    ProgressActionInterface *progressAction);

public slots:
    void slotCancel();

private:
    bool canceledFlag_;
    double etaStartTime_;
    double etaStartPercent_;
    QLabel *progressStepsLabel_;
    QLabel *progressStepLabel_;
    QLabel *etaLabel_;
    QProgressBar *progressBar_;
    QPushButton *cancelButton_;

    void closeEvent(QCloseEvent *event) override;
    void initializeLabels(ProgressActionInterface *progressAction);
    void updateLabels(ProgressActionInterface *progressAction);
};

#include <WarningsEnable.hpp>

#endif /* PROGRESS_DIALOG_HPP */
