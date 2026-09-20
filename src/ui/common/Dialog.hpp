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

/** @file Dialog.hpp */

#ifndef DIALOG_HPP
#define DIALOG_HPP

// Include std.
#include <string>
#include <vector>

// Include 3D Forest.
#include <CloseEvent.hpp>
#include <Signal.hpp>
#include <Size.hpp>
#include <ThemeIcon.hpp>
#include <Ui.hpp>
class Application;
class Layout;

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Dialog. */
class EXPORT_UI_COMMON Dialog
{
public:
    enum DialogCode
    {
        Rejected,
        Accepted
    };

    enum WindowModality
    {
        NonModal = 0,
        WindowModal = 1,
        ApplicationModal = 2
    };

    enum StandardButton
    {
        NoButton = 0x00000000,
        Ok = 0x00000400,
        Save = 0x00000800,
        SaveAll = 0x00001000,
        Open = 0x00002000,
        Yes = 0x00004000,
        YesToAll = 0x00008000,
        No = 0x00010000,
        NoToAll = 0x00020000,
        Abort = 0x00040000,
        Retry = 0x00080000,
        Ignore = 0x00100000,
        Close = 0x00200000,
        Cancel = 0x00400000,
        Discard = 0x00800000,
        Help = 0x01000000,
        Apply = 0x02000000,
        Reset = 0x04000000,
        RestoreDefaults = 0x08000000
    };

    Dialog();
    explicit Dialog(Application *app);
    virtual ~Dialog();

    void setApplication(Application *app);
    Application *application() const { return app_; }

    void setWindowTitle(const std::string &title);
    void setWindowIcon(const ThemeIcon &icon);

    const std::string &windowTitle() const { return windowTitle_; }
    const ThemeIcon &windowIcon() const { return windowIcon_; }

    void setStandardButtons(int buttons);
    void setDefaultButton(StandardButton button);

    int standardButtons() const { return buttons_; }
    StandardButton defaultButton() const { return defaultButton_; }

    void setLayout(Layout *layout);
    Layout *layout() const { return layout_; }

    void setFixedHeight(int height);
    void setMaximumWidth(int width);
    void setMaximumHeight(int height);

    int fixedHeight() const { return fixedHeight_; }
    int maximumWidth() const { return maximumWidth_; }
    int maximumHeight() const { return maximumHeight_; }

    int width() const { return width_; }
    int height() const { return height_; }

    void updateSize(int width, int height);

    void setModal(bool modal);
    void setWindowModality(int modality);

    bool isModal() const { return modality_ != NonModal; }
    WindowModality windowModality() const { return modality_; }

    int exec();

    void show();
    void hide();
    void raise();
    void activateWindow();

    void close();

    void setResult(int result) { result_ = result; }
    int result() const { return result_; }

    virtual Size sizeHint() const { return Size(); }
    virtual Size minimumSizeHint() const { return Size(); }

    virtual void accept();
    virtual void reject();
    virtual void done(int result);

    virtual void closeEvent(CloseEvent *event);

    // Backend notifications.
    Signal<> propertiesChanged;
    Signal<> hideRequested;
    Signal<> raiseRequested;
    Signal<> activateWindowRequested;

    // Completion notification, also used by the backend to close its dialog.
    Signal<int> finished;

    // For custom operations such as Apply, Reset, and Help.
    Signal<StandardButton> buttonClicked;

    // lifetime signal
    Signal<> destroyed;

protected:
    Application *app_{nullptr};

    std::string windowTitle_;
    ThemeIcon windowIcon_;

    Layout *layout_{nullptr};

    int buttons_{NoButton};
    StandardButton defaultButton_{NoButton};

    int result_{Rejected};

    // -1 means no explicit constraint.
    int fixedHeight_{-1};
    int maximumWidth_{-1};
    int maximumHeight_{-1};

    int width_{0};
    int height_{0};

    WindowModality modality_{NonModal};

private:
    Application &requireApplication() const;
};

#include <WarningsEnable.hpp>

#endif /* DIALOG_HPP */
