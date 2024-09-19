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

/** @file ExplorerWidgetInterface.hpp */

#ifndef EXPLORER_WIDGET_INTERFACE_HPP
#define EXPLORER_WIDGET_INTERFACE_HPP

// Include 3D Forest.
#include <SettingsView.hpp>
class MainWindow;

// Include Qt.
#include <QIcon>
#include <QWidget>
class QLabel;
class QVBoxLayout;

/** Explorer Widget Interface. */
class ExplorerWidgetInterface : public QWidget
{
    Q_OBJECT

public:
    ExplorerWidgetInterface(MainWindow *mainWindow,
                            const QIcon &icon,
                            const QString &text);
    virtual ~ExplorerWidgetInterface() = default;

    const QIcon &icon() { return icon_; }
    const QString &text() { return text_; }

    virtual bool hasColorSource() const = 0;
    virtual SettingsView::ColorSource colorSource() const = 0;

    virtual bool filterAvailable() const = 0;
    virtual bool filterEnabled() const { return filterEnabled_; }
    virtual void setFilterEnabled(bool b) { filterEnabled_ = b; }

protected:
    MainWindow *mainWindow_;

    QIcon icon_;
    QString text_;
    QLabel *titleIcon_;
    QLabel *titleText_;
    QVBoxLayout *mainLayout_;

private:
    bool filterEnabled_;
};

#endif /* EXPLORER_WIDGET_INTERFACE_HPP */
