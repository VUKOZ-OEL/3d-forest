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

/** @file QtApplication.hpp */

#ifndef QT_APPLICATION_HPP
#define QT_APPLICATION_HPP

// Include std.
#include <string>

// Include 3D Forest.
#include <Application.hpp>
#include <QtSidebar.hpp>

// Include Qt.
#include <QApplication>
#include <QMainWindow>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

// Include local.
#include <ExportUiQt.hpp>
#include <WarningsDisable.hpp>

/** QtApplication. */
class EXPORT_UI_QT QtApplication : public Application
{
public:
    QtApplication(int &argc, char **argv);
    virtual ~QtApplication();

    void init();

    void setOrganizationName(const std::string &str);
    void setApplicationName(const std::string &str);
    void setApplicationVersion(const std::string &str);

    int exec();

    void addMenuItem(const std::vector<std::string> &path,
                     Action *action) override;
    void removeMenuItem(Action *action) override;
    void addPanel(const std::vector<std::string> &path,
                  Widget *widget) override;
    void removePanel(Widget *widget) override;
    void setViewer(Widget *widget) override;
    void removeViewer(Widget *widget) override;

    QWidget *createWidget(Widget *widget, QWidget *parent = nullptr);
    QLayout *createLayout(Layout *layout, QWidget *parent = nullptr);

private:
    QApplication qapplication_;
    QMainWindow mainWindow_;

    QSplitter *splitter_{nullptr};
    QtSidebar *sidebar_{nullptr};
    QWidget *viewerContainer_{nullptr};
    QVBoxLayout *viewerLayout_{nullptr};
    Widget *commonViewer_{nullptr};
    QWidget *qtViewer_{nullptr};

    void initLayout();
};

#include <WarningsEnable.hpp>

#endif /* QT_APPLICATION_HPP */
