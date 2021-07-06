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

/** @file WindowScreenshot.cpp */

#include <Editor.hpp>
#include <QWidget>
#include <QPixmap>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include <QImageWriter>
#include <Error.hpp>
#include <WindowScreenshot.hpp>
#include <QWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QDateTime>
#include <QDebug>

void WindowScreenshot::capture(QWidget *parent, QWidget *widget, Editor *editor)
{
    // grabWindow and dialog are based on:
    // https://doc.qt.io/qt-5/qtwidgets-desktop-screenshot-example.html

    QScreen *screen;
    const QWindow *window = parent->windowHandle();
    if (window)
    {
        screen = window->screen();
    }
    else
    {
        screen = QGuiApplication::primaryScreen();
    }

    QPixmap picture = screen->grabWindow(widget->winId());

    // File name
    const QString imageFormat = "png";

    QString path;
    path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (path.isEmpty())
    {
        path = QDir::currentPath();
    }
    path = path + QObject::tr("/untitled.") + imageFormat;

    // Dialog
    QFileDialog dialog(parent, QObject::tr("Save Picture As"), path);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(path);

    QStringList mimeTypes;
    const QList<QByteArray> baMimeTypes = QImageWriter::supportedMimeTypes();
    for (const QByteArray &bf : baMimeTypes)
    {
        mimeTypes.append(QLatin1String(bf));
    }

    dialog.setMimeTypeFilters(mimeTypes);
    dialog.selectMimeTypeFilter("image/" + imageFormat);
    dialog.setDefaultSuffix(imageFormat);

    if (dialog.exec() != QDialog::Accepted)
    {
        // Cancel
        return;
    }

    // Save filename
    const QString fileName = dialog.selectedFiles().first();

    QImageWriter writer(fileName);

    // Save metadata
    // Title         - Short (one line) title or caption for image
    // Author        - Name of image's creator
    // Description   - Description of image (possibly long)
    // Copyright     - Copyright notice
    // Creation Time - Time of original image creation
    // Software      - Software used to create the image
    // Disclaimer    - Legal disclaimer
    // Warning       - Warning of nature of content
    // Source        - Device used to create the image
    // Comment       - Miscellaneous comment

    writer.setText("Title", QString::fromStdString(editor->projectName()));

    QString userName = qgetenv("USER"); // Linux
    if (userName.isEmpty())
    {
        userName = qgetenv("USERNAME"); // Windows
    }
    writer.setText("Author", userName);

    QDateTime time = QDateTime::currentDateTimeUtc();
    QString timeStr = time.toString("dd MMM yyyy HH:mm:ss") + " UTC";
    writer.setText("CreationTime", timeStr);

    writer.setText("Software", "3D Forest");

    // Save file
    bool status = writer.write(picture.toImage());
    if (!status)
    {
        THROW("Image " + fileName.toStdString() + " could not be saved.");
    }
}
