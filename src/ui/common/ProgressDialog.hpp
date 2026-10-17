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

// Include 3D Forest.
#include <Dialog.hpp>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Progress Dialog. */
class EXPORT_UI_COMMON ProgressDialog : public Dialog
{
public:
    explicit ProgressDialog(Application *app);
    ~ProgressDialog() override;

    void setRange(int minimum, int maximum);
    int minimum() const { return minimum_; }
    int maximum() const { return maximum_; }

    void setValue(int value);
    int value() const { return value_; }

    void setLabelText(const std::string &text);
    const std::string &labelText() const { return labelText_; }

    void setCancelButtonText(const std::string &text);
    const std::string &cancelButtonText() const { return cancelButtonText_; }

    bool wasCanceled() const { return canceled_; }

    void cancel();
    void reset();

    Signal<int, int> rangeChanged;
    Signal<int> valueChanged;
    Signal<const std::string &> labelTextChanged;
    Signal<const std::string &> cancelButtonTextChanged;

    Signal<> canceled;
    Signal<> resetRequested;

private:
    int minimum_{0};
    int maximum_{100};
    int value_{-1};

    bool canceled_{false};

    std::string labelText_;
    std::string cancelButtonText_{"Cancel"};
};

#include <WarningsEnable.hpp>

#endif /* PROGRESS_DIALOG_HPP */
