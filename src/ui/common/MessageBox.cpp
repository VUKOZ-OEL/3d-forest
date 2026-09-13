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

/** @file MessageBox.cpp */

// Include std.

// Include 3D Forest.
#include <Application.hpp>
#include <MessageBox.hpp>

// Include local.
#define LOG_MODULE_NAME "MessageBox"
#include <Log.hpp>

MessageBox::MessageBox(Application *app) : app_(app)
{
}

void MessageBox::setTitle(const std::string &title)
{
    title_ = title;
}

void MessageBox::setText(const std::string &text)
{
    text_ = text;
}

void MessageBox::setInformativeText(const std::string &text)
{
    informativeText_ = text;
}

void MessageBox::setStandardButtons(int buttons)
{
    buttons_ = buttons;
}

void MessageBox::setDefaultButton(StandardButton button)
{
    defaultButton_ = button;
}

int MessageBox::exec()
{
    return app_->showMessageBox(*this);
}

int MessageBox::information(Application *app,
                            const std::string &title,
                            const std::string &text,
                            int buttons,
                            int defaultButton)
{
    return defaultButton;
}

int MessageBox::about(Application *app,
                      const std::string &title,
                      const std::string &text,
                      int buttons,
                      int defaultButton)
{
    MessageBox box(app);
    box.setTitle(title);
    box.setText(text);
    box.setStandardButtons(buttons);
    return box.exec();
}

int MessageBox::question(Application *app,
                         const std::string &title,
                         const std::string &text,
                         int buttons,
                         int defaultButton)
{
    MessageBox box(app);
    box.setTitle(title);
    box.setText(text);
    box.setStandardButtons(buttons);
    return box.exec();
}

int MessageBox::warning(Application *app,
                        const std::string &title,
                        const std::string &text,
                        int buttons,
                        int defaultButton)
{
    MessageBox box(app);
    box.setTitle(title);
    box.setText(text);
    box.setStandardButtons(buttons);
    return box.exec();
}

int MessageBox::critical(Application *app,
                         const std::string &title,
                         const std::string &text,
                         int buttons,
                         int defaultButton)
{
    MessageBox box(app);
    box.setTitle(title);
    box.setText(text);
    box.setStandardButtons(buttons);
    return box.exec();
}
