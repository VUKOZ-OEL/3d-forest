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

/** @file Pixmap.hpp */

#ifndef PIXMAP_HPP
#define PIXMAP_HPP

// Include std.
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

// Include local.
#include <ExportUiCommon.hpp>
#include <WarningsDisable.hpp>

/** Pixmap. */
class EXPORT_UI_COMMON Pixmap
{
public:
    using Byte = std::uint8_t;
    using Pixel = std::array<Byte, 4>;

    Pixmap() = default;
    Pixmap(int width, int height);

    Pixmap(int width, int height, const std::vector<Byte> &rgba);

    Pixmap(const Pixmap &) = default;
    Pixmap &operator=(const Pixmap &) = default;

    bool isNull() const { return pixels_.empty(); }

    int width() const { return width_; }
    int height() const { return height_; }

    std::size_t bytesPerLine() const
    {
        return static_cast<std::size_t>(width_) * 4;
    }

    std::size_t byteCount() const { return pixels_.size(); }

    const Byte *data() const { return pixels_.data(); }

    void clear();

    // Replaces the image with transparent pixels.
    void reset(int width, int height);

    // Copies tightly packed RGBA data.
    void setData(int width, int height, const std::vector<Byte> &rgba);

    // Channels are clamped to 0..255.
    void fill(int red, int green, int blue, int alpha = 255);

    void setPixel(int x, int y, int red, int green, int blue, int alpha = 255);

    Pixel pixel(int x, int y) const;

    Pixmap invertedRgb() const;

private:
    static std::size_t requiredBytes(int width, int height);
    static Byte channel(int value);

    std::size_t pixelOffset(int x, int y) const;

    int width_{0};
    int height_{0};
    std::vector<Byte> pixels_;
};

#include <WarningsEnable.hpp>

#endif /* PIXMAP_HPP */
