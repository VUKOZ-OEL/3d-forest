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

/** @file icon.cpp */

// Include std.
#include <iostream>

// Include 3D Forest.
#include <ArgumentParser.hpp>
#include <Error.hpp>

// Include Qt.
#include <QDir>
#include <QPainter>
#include <QPixmap>

// Include local.
#define LOG_MODULE_NAME "icon"
#include <Log.hpp>

static void process(const QString &path, const QString &fileName);

int main(int argc, char *argv[])
{
    try
    {
        ArgumentParser arg("format icons");
        arg.add("-d",
                "--directory",
                "",
                "Path to directory with icons to be processed.",
                true);

        if (arg.parse(argc, argv))
        {
            QString path = QString::fromStdString(arg.toString("--directory"));

            QDir searchDirectory(path);
            QStringList fileList = searchDirectory.entryList(QDir::Files);

            std::cout << "directory '"
                      << searchDirectory.absolutePath().toStdString() << "'"
                      << std::endl;
            std::cout << "contains " << fileList.count() << " files"
                      << std::endl;

            for (const QString &fileName : fileList)
            {
                std::cout << "file '" << fileName.toStdString() << "'"
                          << std::endl;
                process(path, fileName);
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void process(const QString &path, const QString &fileName)
{
    if (!(fileName.endsWith(".png") || fileName.endsWith(".ico") ||
          fileName.endsWith(".svg")))
    {
        return;
    }

    QString inputPath = path + "/" + fileName;
    QString outputPath = inputPath;

    // Source image
    QImage img(inputPath);
    std::cout << " " << img.width() << "x" << img.height() << " depth "
              << img.depth() << ", format " << img.format() << std::endl;

    // Destination image
    int size;
    int offset;
    if (img.width() == 30)
    {
        // Pixel perfect resize to 32
        size = 32;
        offset = 1;
        outputPath.replace("-30.png", "-32.png");
    }
    else
    {
        // Keep the same size
        size = img.width();
        offset = 0;
    }

    if (outputPath.endsWith(".ico"))
    {
        outputPath.replace(".ico", ".png");
    }
    if (outputPath.endsWith(".svg"))
    {
        outputPath.replace(".svg", ".png");
    }

    // Copy
    QImage dest(size, size, img.format());
    dest.fill(0);
    QPainter painter(&dest);
    if (size < img.width())
    {
        painter.drawImage(0, 0, img, offset, offset, size, size);
    }
    else
    {
        painter.drawImage(offset, offset, img);
    }
    painter.end();

    // Output
    bool saved = dest.save(outputPath);
    if (saved)
    {
        std::cout << " output '" << outputPath.toStdString() << std::endl;
    }
}
#if 0
void processCut(const QString &path, const QString &fileName)
{
    if (!fileName.endsWith(".png"))
    {
        return;
    }

    QString inputPath = path + "/" + fileName;

    // Source image
    QImage img(inputPath);
    std::cout << " " << img.width() << "x" << img.height() << " depth "
              << img.depth() << ", format " << img.format() << std::endl;

    int size = 24;
    int cell = 40;
    int x = 0;
    int y = 0;
    int cx = 1;
    int cy = 1;

    // Copy
    while (y + size <= img.height())
    {
        while (x + size <= img.width())
        {
            QImage dest(size, size, img.format());
            dest.fill(0);
            QPainter painter(&dest);
            painter.drawImage(0, 0, img, x, y, size, size);
            painter.end();

            QString outputPath = inputPath;
            QString number = QString().asprintf("%02d%02d", cx, cy);
            outputPath.replace(".png", number + ".png");
            std::cout << " output '" << outputPath.toStdString() << "' "
                      << number.toStdString() << " : " << cx << ", " << cy
                      << std::endl;
            (void)dest.save(outputPath);

            x += cell;
            cx += 1;
        }

        x = 0;
        cx = 1;
        y += cell;
        cy += 1;
    }
}
#endif
