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

#include <iostream>

#include <QDir>
#include <QPainter>
#include <QPixmap>

static void process(const QString &path, const QString &fileName);

int main(int argc, char *argv[])
{
    QString path;
    if (argc > 1)
    {
        path = QString(argv[1]);
    }

    QDir searchDirectory(path);
    QStringList fileList = searchDirectory.entryList(QDir::Files);

    std::cout << "directory '" << searchDirectory.absolutePath().toStdString()
              << "'" << std::endl;
    std::cout << "contains " << fileList.count() << " files" << std::endl;

    for (const QString &fileName : fileList)
    {
        std::cout << "file '" << fileName.toStdString() << "'" << std::endl;
        process(path, fileName);
    }

    return 0;
}

void process(const QString &path, const QString &fileName)
{
    if (!(fileName.endsWith(".png") || fileName.endsWith(".ico")))
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
    if (fileName.endsWith("-30.png"))
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

    if (fileName.endsWith(".ico"))
    {
        outputPath.replace(".ico", ".png");
    }

    // Copy
    QImage dest(size, size, img.format());
    dest.fill(0);
    QPainter painter(&dest);
    painter.drawImage(offset, offset, img);
    painter.end();

    // Output
    bool saved = dest.save(outputPath);
    if (saved)
    {
        std::cout << " output '" << outputPath.toStdString() << std::endl;
    }
}
