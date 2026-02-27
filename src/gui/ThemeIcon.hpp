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

/** @file ThemeIcon.hpp */

#ifndef THEME_ICON_HPP
#define THEME_ICON_HPP

// Include Qt.
#include <QFile>
#include <QIcon>
#include <QString>

// Include local.
#include <ExportGui.hpp>
#include <WarningsDisable.hpp>

#define THEME_ICON(name) (ThemeIcon(":/gui/", name))

/** Theme Icon.

    QIcon encapsulates multiple pixel perfect icon sizes.
    Qt will automatically select the best size for a given rendering.
*/
class EXPORT_GUI ThemeIcon
{
public:
    ThemeIcon() = default;
    ThemeIcon(const QString &prefix, const QString &name)
    {
        addFileExists(prefix + name + "-16px", QSize(16, 16));
        addFileExists(prefix + name + "-20px", QSize(20, 20));
        addFileExists(prefix + name + "-24px", QSize(24, 24));
    }

    QIcon icon(bool dark = false) const
    {
        QIcon ic;
        const QList<QPixmap> &src = dark ? dark_ : light_;
        for (const auto &pm : src)
        {
            ic.addPixmap(pm);
        }
        return ic;
    }

    QPixmap pixmap(int size) const
    {
        const QList<QPixmap> &src = light_;
        if (src.isEmpty())
        {
            return {};
        }

        // Choose closest match.
        const QPixmap *best = &src.first();
        int bestDiff = std::abs(best->width() - size);
        for (const auto &pm : src)
        {
            int diff = std::abs(pm.width() - size);
            if (diff < bestDiff)
            {
                best = &pm;
                bestDiff = diff;
            }
        }

        return *best;
    }

    QString toQString() const
    {
        return "light count <" + QString::number(light_.size()) +
               "dark count <" + QString::number(dark_.size());
    }

private:
    void addFileExists(const QString &filename, const QSize &size)
    {
        if (QFile(filename + "-color.png").exists())
        {
            addFile(filename + "-color.png", size);
        }
        else if (QFile(filename + ".png").exists())
        {
            addFile(filename + ".png", size);
        }
    }

    void addFile(const QString &filename, const QSize &size)
    {
        (void)size;

        QImage image(filename);
        if (image.isNull())
        {
            return;
        }

        QImage inverted(image);
        inverted.invertPixels(QImage::InvertRgb);

        light_ << QPixmap::fromImage(image);
        dark_ << QPixmap::fromImage(inverted);
    }

    QList<QPixmap> light_;
    QList<QPixmap> dark_;
};

#include <WarningsEnable.hpp>

#endif /* THEME_ICON_HPP */
