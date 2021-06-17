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

/** @file Endian.hpp */

#ifndef ENDIAN_HPP
#define ENDIAN_HPP

#include <cstdint>

/** Copy 2 bytes. */
inline void copy16(uint8_t *dst, const uint8_t *src)
{
    dst[0] = src[0];
    dst[1] = src[1];
}

/** Copy 4 bytes. */
inline void copy32(uint8_t *dst, const uint8_t *src)
{
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
}

/** Copy 8 bytes. */
inline void copy64(uint8_t *dst, const uint8_t *src)
{
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[7] = src[7];
}

/** Swap 2 byte endian. */
inline void swap16(uint8_t *dst, const uint8_t *src)
{
    dst[0] = src[1];
    dst[1] = src[0];
}

/** Swap 4 byte endian. */
inline void swap32(uint8_t *dst, const uint8_t *src)
{
    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];
}

/** Swap 8 byte endian. */
inline void swap64(uint8_t *dst, const uint8_t *src)
{
    dst[0] = src[7];
    dst[1] = src[6];
    dst[2] = src[5];
    dst[3] = src[4];
    dst[4] = src[3];
    dst[5] = src[2];
    dst[6] = src[1];
    dst[7] = src[0];
}

/** Convert little to host endian in 2 bytes. */
inline uint16_t ltoh16(const uint8_t *src)
{
    return static_cast<uint16_t>((static_cast<uint32_t>(src[1]) << 8) |
                                 static_cast<uint32_t>(src[0]));
}

/** Convert little to host endian in 4 bytes. */
inline uint32_t ltoh32(const uint8_t *src)
{
    return (static_cast<uint32_t>(src[3]) << 24) |
           (static_cast<uint32_t>(src[2]) << 16) |
           (static_cast<uint32_t>(src[1]) << 8) | static_cast<uint32_t>(src[0]);
}

/** Convert network big to host endian in 4 bytes. */
inline uint32_t ntoh32(const uint8_t *src)
{
    return (static_cast<uint32_t>(src[0]) << 24) |
           (static_cast<uint32_t>(src[1]) << 16) |
           (static_cast<uint32_t>(src[2]) << 8) | static_cast<uint32_t>(src[3]);
}

/** Convert little to host endian in 8 bytes. */
inline uint64_t ltoh64(const uint8_t *src)
{
    return (static_cast<uint64_t>(src[7]) << 56) |
           (static_cast<uint64_t>(src[6]) << 48) |
           (static_cast<uint64_t>(src[5]) << 40) |
           (static_cast<uint64_t>(src[4]) << 32) |
           (static_cast<uint64_t>(src[3]) << 24) |
           (static_cast<uint64_t>(src[2]) << 16) |
           (static_cast<uint64_t>(src[1]) << 8) | static_cast<uint64_t>(src[0]);
}

/** Convert little to host endian in float. */
inline float ltohf(const uint8_t *src)
{
    float ret;
    uint8_t *dst = reinterpret_cast<uint8_t *>(&ret);
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
    copy32(dst, src);
#else
    swap32(dst, src);
#endif
    return ret;
}

/** Convert little to host endian in double. */
inline double ltohd(const uint8_t *src)
{
    double ret;
    uint8_t *dst = reinterpret_cast<uint8_t *>(&ret);
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
    copy64(dst, src);
#else
    swap64(dst, src);
#endif
    return ret;
}

/** Convert host to little endian in 2 bytes. */
inline void htol16(uint8_t *dst, uint16_t src)
{
    dst[0] = static_cast<uint8_t>(src);
    dst[1] = static_cast<uint8_t>(src >> 8);
}

/** Convert host to little endian in 4 bytes. */
inline void htol32(uint8_t *dst, uint32_t src)
{
    dst[0] = static_cast<uint8_t>(src);
    dst[1] = static_cast<uint8_t>(src >> 8);
    dst[2] = static_cast<uint8_t>(src >> 16);
    dst[3] = static_cast<uint8_t>(src >> 24);
}

/** Convert host to little endian in 8 bytes. */
inline void htol64(uint8_t *dst, uint64_t src)
{
    dst[0] = static_cast<uint8_t>(src);
    dst[1] = static_cast<uint8_t>(src >> 8);
    dst[2] = static_cast<uint8_t>(src >> 16);
    dst[3] = static_cast<uint8_t>(src >> 24);
    dst[4] = static_cast<uint8_t>(src >> 32);
    dst[5] = static_cast<uint8_t>(src >> 40);
    dst[6] = static_cast<uint8_t>(src >> 48);
    dst[7] = static_cast<uint8_t>(src >> 56);
}

/** Convert host to little endian in float. */
inline void htolf(uint8_t *dst, float src)
{
    uint8_t *src8 = reinterpret_cast<uint8_t *>(&src);
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
    copy32(dst, src8);
#else
    swap32(dst, src8);
#endif
}

/** Convert host to little endian in double. */
inline void htold(uint8_t *dst, double src)
{
    uint8_t *src8 = reinterpret_cast<uint8_t *>(&src);
#if __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
    copy64(dst, src8);
#else
    swap64(dst, src8);
#endif
}

#endif /* ENDIAN_HPP */
