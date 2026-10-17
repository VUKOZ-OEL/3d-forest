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

/** @file Pixmap.cpp */

// Include std.
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

// Include 3D Forest.
#include <Pixmap.hpp>

// Include local.
#define LOG_MODULE_NAME "Pixmap"
#include <Log.hpp>

Pixmap::Pixmap(int width, int height)
{
    reset(width, height);
}

Pixmap::Pixmap(int width, int height, const std::vector<Byte> &rgba)
{
    setData(width, height, rgba);
}

std::size_t Pixmap::requiredBytes(int width, int height)
{
    if (width < 0 || height < 0)
    {
        throw std::invalid_argument("Pixmap dimensions must not be negative.");
    }

    if (width == 0 || height == 0)
    {
        return 0;
    }

    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);

    const std::size_t maximum = (std::numeric_limits<std::size_t>::max)();

    if (w > maximum / 4 || h > maximum / (w * 4))
    {
        throw std::length_error("Pixmap dimensions are too large.");
    }

    return w * h * 4;
}

Pixmap::Byte Pixmap::channel(int value)
{
    return static_cast<Byte>(std::clamp(value, 0, 255));
}

void Pixmap::clear()
{
    pixels_.clear();
    width_ = 0;
    height_ = 0;
}

void Pixmap::reset(int width, int height)
{
    const std::size_t count = requiredBytes(width, height);

    if (count == 0)
    {
        clear();
        return;
    }

    // Allocate first so allocation failure preserves the old image.
    std::vector<Byte> pixels(count, 0);

    pixels_ = std::move(pixels);
    width_ = width;
    height_ = height;
}

void Pixmap::setData(int width, int height, const std::vector<Byte> &rgba)
{
    const std::size_t count = requiredBytes(width, height);

    if (rgba.size() != count)
    {
        throw std::invalid_argument(
            "Pixmap RGBA buffer size does not match its dimensions.");
    }

    if (count == 0)
    {
        clear();
        return;
    }

    std::vector<Byte> pixels(rgba);

    pixels_ = std::move(pixels);
    width_ = width;
    height_ = height;
}

void Pixmap::fill(int red, int green, int blue, int alpha)
{
    const Pixel color{channel(red),
                      channel(green),
                      channel(blue),
                      channel(alpha)};

    for (std::size_t i = 0; i < pixels_.size(); i += 4)
    {
        pixels_[i] = color[0];
        pixels_[i + 1] = color[1];
        pixels_[i + 2] = color[2];
        pixels_[i + 3] = color[3];
    }
}

std::size_t Pixmap::pixelOffset(int x, int y) const
{
    if (x < 0 || y < 0 || x >= width_ || y >= height_)
    {
        throw std::out_of_range("Pixmap pixel coordinates.");
    }

    return static_cast<std::size_t>(y) * bytesPerLine() +
           static_cast<std::size_t>(x) * 4;
}

void Pixmap::setPixel(int x, int y, int red, int green, int blue, int alpha)
{
    const std::size_t i = pixelOffset(x, y);

    pixels_[i] = channel(red);
    pixels_[i + 1] = channel(green);
    pixels_[i + 2] = channel(blue);
    pixels_[i + 3] = channel(alpha);
}

Pixmap::Pixel Pixmap::pixel(int x, int y) const
{
    const std::size_t i = pixelOffset(x, y);

    return {pixels_[i], pixels_[i + 1], pixels_[i + 2], pixels_[i + 3]};
}

Pixmap Pixmap::invertedRgb() const
{
    Pixmap result(*this);

    for (std::size_t i = 0; i < result.pixels_.size(); i += 4)
    {
        result.pixels_[i] = static_cast<Byte>(255 - result.pixels_[i]);
        result.pixels_[i + 1] = static_cast<Byte>(255 - result.pixels_[i + 1]);
        result.pixels_[i + 2] = static_cast<Byte>(255 - result.pixels_[i + 2]);
    }

    return result;
}
