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

/** @file Dialog.cpp */

// Include std.
#include <stdexcept>

// Include 3D Forest.
#include <Application.hpp>
#include <Dialog.hpp>
#include <Layout.hpp>

// Include local.
#define LOG_MODULE_NAME "Dialog"
#include <Log.hpp>

Dialog::Dialog()
{
}

Dialog::Dialog(Application *app) : app_(app)
{
}

Dialog::~Dialog()
{
    destroyed();
    delete layout_;
}

void Dialog::setApplication(Application *app)
{
    app_ = app;
}

Application &Dialog::requireApplication() const
{
    if (!app_)
    {
        throw std::logic_error("Dialog requires an Application");
    }

    return *app_;
}

void Dialog::setWindowTitle(const std::string &title)
{
    if (windowTitle_ == title)
    {
        return;
    }

    windowTitle_ = title;

    windowTitle_ = title;
    propertiesChanged();
}

void Dialog::setWindowIcon(const ThemeIcon &icon)
{
    windowIcon_ = icon;
    propertiesChanged();
}

void Dialog::setStandardButtons(int buttons)
{
    if (buttons_ == buttons)
    {
        return;
    }

    buttons_ = buttons;
    propertiesChanged();
}

void Dialog::setDefaultButton(StandardButton button)
{
    if (defaultButton_ == button)
    {
        return;
    }

    defaultButton_ = button;
    propertiesChanged();
}

void Dialog::setLayout(Layout *layout)
{
    if (layout_ == layout)
    {
        return;
    }

    delete layout_;
    layout_ = layout;
}

void Dialog::setFixedHeight(int height)
{
    if (height < -1)
    {
        throw std::invalid_argument("Invalid fixed height");
    }

    fixedHeight_ = height;
    propertiesChanged();
}

void Dialog::setMaximumWidth(int width)
{
    if (width < -1)
    {
        throw std::invalid_argument("Invalid maximum width");
    }

    maximumWidth_ = width;
    propertiesChanged();
}

void Dialog::setMaximumHeight(int height)
{
    if (height < -1)
    {
        throw std::invalid_argument("Invalid maximum height");
    }

    maximumHeight_ = height;
    propertiesChanged();
}

void Dialog::updateSize(int width, int height)
{
    width_ = width;
    height_ = height;
}

void Dialog::setModal(bool modal)
{
    setWindowModality(modal ? ApplicationModal : NonModal);
}

void Dialog::setWindowModality(int modality)
{
    if (modality < NonModal || modality > ApplicationModal)
    {
        throw std::invalid_argument("Invalid window modality");
    }

    modality_ = static_cast<WindowModality>(modality);
    propertiesChanged();
}

int Dialog::exec()
{
    Application &app = requireApplication();

    result_ = Rejected;
    result_ = app.showDialog(*this);

    return result_;
}

void Dialog::show()
{
    requireApplication().openDialog(*this);
}

void Dialog::hide()
{
    hideRequested();
}

void Dialog::raise()
{
    raiseRequested();
}

void Dialog::activateWindow()
{
    activateWindowRequested();
}

void Dialog::close()
{
    CloseEvent event;
    event.accept();

    closeEvent(&event);

    if (event.isAccepted())
    {
        reject();
    }
}

void Dialog::closeEvent(CloseEvent *event)
{
    event->accept();
}

void Dialog::accept()
{
    done(Accepted);
}

void Dialog::reject()
{
    done(Rejected);
}

void Dialog::done(int result)
{
    setResult(result);
    finished(result);
}
