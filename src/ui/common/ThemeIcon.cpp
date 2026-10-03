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

/** @file ThemeIcon.cpp */

// Include std.
#include <cstdlib>
#include <stdexcept>

// Include 3D Forest.
#include <Application.hpp>
#include <ThemeIcon.hpp>

// Include local.
#define LOG_MODULE_NAME "ThemeIcon"
// #define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

ThemeIcon::ThemeIcon(Application *app,
                     const std::string &prefix,
                     const std::string &name)
{
    if (!app)
    {
        throw std::invalid_argument("ThemeIcon requires an Application.");
    }

    std::string base = prefix;

    if (!base.empty() && base.back() != '/')
    {
        base += '/';
    }

    base += name;

    for (int size : {16, 20, 24})
    {
        addFile(app, base + "-" + std::to_string(size) + "px");
    }
}

void ThemeIcon::addFile(Application *app, const std::string &baseName)
{
    Pixmap image;

    loadPixmap(image, app, baseName + "-color.png");

    if (image.isNull())
    {
        loadPixmap(image, app, baseName + ".png");
    }

    if (image.isNull())
    {
        return;
    }

    light_.push_back(image);
    dark_.push_back(image.invertedRgb());
}

void ThemeIcon::loadPixmap(Pixmap &pixmap,
                           Application *app,
                           const std::string &path)
{
    pixmap = app->loadPixmap(path);

    if (pixmap.isNull())
    {
        LOG_DEBUG(<< "Pixmap from <" << path << "> is null.");
    }
    else
    {
        LOG_DEBUG(<< "Pixmap from <" << path << "> loaded.");
    }
}

Icon ThemeIcon::icon(bool dark) const
{
    Icon result;

    const auto &images = dark ? dark_ : light_;

    for (const Pixmap &image : images)
    {
        result.addPixmap(image);
    }

    return result;
}

Pixmap ThemeIcon::pixmap(int size, bool dark) const
{
    const auto &images = dark ? dark_ : light_;

    if (images.empty() || size <= 0)
    {
        return {};
    }

    const Pixmap *best = &images.front();
    int bestDifference = std::abs(best->width() - size);

    for (const Pixmap &image : images)
    {
        const int difference = std::abs(image.width() - size);

        if (difference < bestDifference)
        {
            best = &image;
            bestDifference = difference;
        }
    }

    return *best;
}

std::string ThemeIcon::toString() const
{
    return "light count <" + std::to_string(light_.size()) + "> dark count <" +
           std::to_string(dark_.size()) + ">";
}
