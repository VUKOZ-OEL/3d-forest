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

/** @file WindowHelp.cpp */

#include <QApplication>
#include <QHBoxLayout>
#include <QHelpContentWidget>
#include <QHelpEngine>
#include <QHelpIndexWidget>
#include <QSplitter>
#include <WindowHelp.hpp>

HelpBrowser::HelpBrowser(QHelpEngine *helpEngine, QWidget *parent)
    : QTextBrowser(parent),
      helpEngine_(helpEngine)
{
}

QVariant HelpBrowser::loadResource(int type, const QUrl &url)
{
    if (url.scheme() == "qthelp")
    {
        return QVariant(helpEngine_->fileData(url));
    }
    else
    {
        return QTextBrowser::loadResource(type, url);
    }
}

void HelpBrowser::showHelp(const QUrl &url)
{
    QTextBrowser::setSource(url);
}

WindowHelp::WindowHelp(QWidget *parent) : QDialog(parent)
{
    // Documents
    QHelpEngine *helpEngine = new QHelpEngine(
        QApplication::applicationDirPath() + "/documentation/3dforest.qhc");

    helpEngine->setupData();

    // Table of Contents & Text
    HelpBrowser *textViewer = new HelpBrowser(helpEngine);
    textViewer->setSource(QUrl("qthelp://3dforest/documentation/index.html"));
    connect(helpEngine->contentWidget(),
            SIGNAL(linkActivated(QUrl)),
            textViewer,
            SLOT(showHelp(QUrl)));

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(helpEngine->contentWidget());
    splitter->addWidget(textViewer);
    splitter->setSizes(QList<int>({200, 600}));

    // Layout
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(splitter);
    mainLayout->setContentsMargins(1, 1, 1, 1);
    setLayout(mainLayout);

    // Window
    setWindowTitle("User Manual");
}
