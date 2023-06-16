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

/** @file Util.hpp */

#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>

#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

extern void assertionFailure(const char *file, int line, const char *assertion);

#define EXIT abort()

#if defined(DEBUG_MODE)
    #define ASSERT(cond)                                                       \
        do                                                                     \
        {                                                                      \
            if (!(cond))                                                       \
            {                                                                  \
                assertionFailure(__FILE__, __LINE__, #cond);                   \
                EXIT;                                                          \
            }                                                                  \
        } while (0)
#else
    #define ASSERT(cond) ((void)0)
#endif

inline char *ustrcpy(char *dst, const char *src)
{
    while (*src)
    {
        *dst++ = *src++;
    }
    *dst = 0;
    return dst;
}

inline char *ustrcat(char *dst, const char *src)
{
    while (*dst)
    {
        dst++;
    }
    return ustrcpy(dst, src);
}

class Null
{
};

inline size_t memorySize(bool b)
{
    return sizeof(b);
}
inline size_t memorySize(char c)
{
    return sizeof(c);
}
inline size_t memorySize(signed char c)
{
    return sizeof(c);
}
inline size_t memorySize(unsigned char c)
{
    return sizeof(c);
}
inline size_t memorySize(short i)
{
    return sizeof(i);
}
inline size_t memorySize(unsigned short i)
{
    return sizeof(i);
}
inline size_t memorySize(int i)
{
    return sizeof(i);
}
inline size_t memorySize(unsigned int i)
{
    return sizeof(i);
}
inline size_t memorySize(long i)
{
    return sizeof(i);
}
inline size_t memorySize(unsigned long i)
{
    return sizeof(i);
}
inline size_t memorySize(long long i)
{
    return sizeof(i);
}
inline size_t memorySize(unsigned long long i)
{
    return sizeof(i);
}
inline size_t memorySize(float f)
{
    return sizeof(f);
}
inline size_t memorySize(double d)
{
    return sizeof(d);
}
inline size_t memorySize(void *ptr)
{
    return sizeof(ptr);
}

class Numeric
{
public:
    template <class T> static T min() { return 0; }
    template <class T> static T max() { return 0; }

    template <class T> static T min(const T &, const T &);
};

template <class T> inline T Numeric::min(const T &a, const T &b)
{
    return (a < b) ? a : b;
}

template <> inline signed char Numeric::min<signed char>()
{
    return SCHAR_MIN;
}

template <> inline signed char Numeric::max<signed char>()
{
    return SCHAR_MIN;
}

template <> inline unsigned char Numeric::max<unsigned char>()
{
    return UCHAR_MAX;
}

template <> inline int Numeric::min<int>()
{
    return INT_MIN;
}

template <> inline int Numeric::max<int>()
{
    return INT_MAX;
}

template <> inline unsigned int Numeric::max<unsigned int>()
{
    return UINT_MAX;
}

template <> inline long Numeric::min<long>()
{
    return LONG_MIN;
}

template <> inline long Numeric::max<long>()
{
    return LONG_MAX;
}

template <> inline unsigned long Numeric::max<unsigned long>()
{
    return ULONG_MAX;
}

template <> inline long long Numeric::min<long long>()
{
    return LLONG_MIN;
}

template <> inline long long Numeric::max<long long>()
{
    return LLONG_MAX;
}

template <> inline unsigned long long Numeric::max<unsigned long long>()
{
    return ULLONG_MAX;
}

inline std::string toLower(const std::string &str)
{
    std::string ret = str;
    for (auto &c : ret)
    {
        c = static_cast<char>(std::tolower(c));
    }
    return ret;
}

#include <WarningsEnable.hpp>

#endif /* UTIL_HPP */
