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

/** @file QtIcon.cpp */

// Include std.
#include <cstring>
#include <new>
#include <variant>

// Include 3D Forest.
#include <QtIcon.hpp>
#include <ResourceBundle.hpp>

// Include Qt.
#include <QImage>
#include <QSize>
#include <QString>

// Include local.
#define LOG_MODULE_NAME "QtIcon"
#include <Log.hpp>

namespace
{

QIcon::Mode toQtMode(Icon::Mode mode)
{
    switch (mode)
    {
        case Icon::Normal:
            return QIcon::Normal;

        case Icon::Disabled:
            return QIcon::Disabled;

        case Icon::Active:
            return QIcon::Active;

        case Icon::Selected:
            return QIcon::Selected;
    }

    return QIcon::Normal;
}

QIcon::State toQtState(Icon::State state)
{
    return state == Icon::On ? QIcon::On : QIcon::Off;
}

} // namespace

QPixmap toQPixmap(const Pixmap &pixmap)
{
    if (pixmap.isNull())
    {
        return QPixmap();
    }

    QImage image(pixmap.width(), pixmap.height(), QImage::Format_RGBA8888);

    if (image.isNull())
    {
        throw std::bad_alloc();
    }

    const std::size_t stride = pixmap.bytesPerLine();

    for (int y = 0; y < pixmap.height(); ++y)
    {
        std::memcpy(image.scanLine(y),
                    pixmap.data() + static_cast<std::size_t>(y) * stride,
                    stride);
    }

    return QPixmap::fromImage(image);
}

QIcon toQIcon(const Icon &icon)
{
    QIcon result;

    for (const auto &entry : icon.entries())
    {
        const auto mode = toQtMode(entry.mode);
        const auto state = toQtState(entry.state);

        if (const auto *path = std::get_if<std::string>(&entry.source))
        {
            if (path->compare(0, 2, ":/") == 0)
            {
                const ResourceData bytes = ResourceRegistry::get(*path);
                if (!bytes || bytes->empty() ||
                    bytes->size() > static_cast<std::size_t>(
                                        (std::numeric_limits<uint>::max)()))
                {
                    continue;
                }
                QPixmap image;
                if (image.loadFromData(bytes->data(),
                                       static_cast<uint>(bytes->size())))
                {
                    result.addPixmap(image, mode, state);
                }
            }
            else
            {
                result.addFile(QString::fromStdString(*path),
                               QSize(),
                               mode,
                               state);
            }
        }
        else
        {
            result.addPixmap(toQPixmap(std::get<Pixmap>(entry.source)),
                             mode,
                             state);
        }
    }

    return result;
}

QIcon toQIcon(const ThemeIcon &icon, bool dark)
{
    return toQIcon(icon.icon(dark));
}
