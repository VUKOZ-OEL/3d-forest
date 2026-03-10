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

/** @file ApplicationSettingsWidget.hpp */

#ifndef APPLICATION_SETTINGS_WIDGET_HPP
#define APPLICATION_SETTINGS_WIDGET_HPP

// Include std.

// Include 3D Forest.
#include <Editor.hpp>
#include <Settings.hpp>
class MainWindow;

// Include Qt.
#include <QWidget>
class QComboBox;

/** Application Settings Widget. */
class ApplicationSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    ApplicationSettingsWidget(MainWindow *mainWindow);

    QSize sizeHint() const override { return QSize(300, 80); }

public slots:
    void slotUpdate(void *sender, const QSet<Editor::Type> &target);

    void slotLanguageChanged(int index);

private:
    MainWindow *mainWindow_;

    QComboBox *languageComboBox_;

    ApplicationSettings settings_;

    void dataChanged(bool modifiers = false);
    void setApplicationSettings(const ApplicationSettings &settings);
    void block();
    void unblock();
};

#endif /* APPLICATION_SETTINGS_WIDGET_HPP */
