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

// Include std.
#include <cfloat>
#include <climits>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

#define MATH_PI_4 (3.1415926535897932384626433832795 / 4.0)

#define MATH_ATAN_A 0.0776509570923569
#define MATH_ATAN_B -0.287434475393028
#define MATH_ATAN_C (MATH_PI_4 - MATH_ATAN_A - MATH_ATAN_B)

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

inline double fastatan(double arg)
{
    const double arg2 = arg * arg;
    return ((MATH_ATAN_A * arg2 + MATH_ATAN_B) * arg2 + MATH_ATAN_C) * arg;
}

template <class T> inline T min(const T &a, const T &b)
{
    return (a < b) ? a : b;
}

template <class T> inline T max(const T &a, const T &b)
{
    return (a > b) ? a : b;
}

template <class T> inline void clamp(T &value, const T &min, const T &max)
{
    if (value < min)
    {
        value = min;
    }
    else if (value > max)
    {
        value = max;
    }
}

template <class T> inline void updateRange(const T &value, T &min, T &max)
{
    if (value < min)
    {
        min = value;
    }

    if (value > max)
    {
        max = value;
    }
}

template <class T> inline void normalize(T &value, const T &min, const T &max)
{
    T range = max - min;
    if (range > std::numeric_limits<T>::epsilon())
    {
        value = (value - min) / range;
    }
    else
    {
        value = 0;
    }
}

inline double safeDivide(double numerator, double denominator)
{
    if (std::fabs(denominator) < std::numeric_limits<double>::epsilon())
    {
        if (std::fabs(numerator) < std::numeric_limits<double>::epsilon())
        {
            throw std::runtime_error("Division undefined (0 / 0).");
        }
        else
        {
            return (numerator > 0.0 ? std::numeric_limits<double>::max()
                                    : -std::numeric_limits<double>::max());
        }
    }

    return numerator / denominator;
}

inline bool zero(double number)
{
    return std::fabs(number) < std::numeric_limits<double>::epsilon();
}

template <typename T> inline bool equal(T a, T b)
{
    return std::abs(a - b) <= (std::max(std::abs(a), std::abs(b)) *
                               std::numeric_limits<T>::epsilon());
}

template <typename T> inline bool between(T v, T a, T b)
{
    return a <= v && v <= b;
}

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

// Set all elements to the specified value.
template <class T> void set(std::vector<T> &v, const T &element)
{
    for (size_t i = 0; i < v.size(); i++)
    {
        v[i] = element;
    }
}

// Insert an element to the vector.
template <class T> size_t insert(std::vector<T> &v, const T &element)
{
    size_t i;
    size_t n = v.size();

    v.resize(n + 1);

    i = n;
    while (i > 0 && v[i - 1] > element)
    {
        // shift
        v[i] = v[i - 1];
        --i;
    }

    v[i] = element;

    return i;
}

// Insert an element to the vector.
template <class T>
void insertAt(std::vector<T> &v, size_t idx, const T &element)
{
    size_t i;
    size_t n = v.size();

    v.resize(n + 1);

    for (i = n; i > idx; --i)
    {
        v[i] = v[i - 1];
    }

    v[idx] = element;
}

// Remove all appearances of given element from the vector.
template <class T> void remove(std::vector<T> &v, const T &element)
{
    size_t i;
    size_t n = v.size();
    size_t nRem = 0;

    for (i = 0; i < n; ++i)
    {
        if (nRem > 0)
        {
            // shift
            v[i - nRem] = v[i];
        }
        if (v[i] == element)
        {
            ++nRem;
        }
    }

    if (nRem > 0)
    {
        v.resize(n - nRem);
    }
}

// Remove an element specified by the index from the vector.
template <class T> void removeAt(std::vector<T> &v, size_t idx)
{
    size_t n = v.size();
    if (n > 1 && idx < n)
    {
        n--;

        // shift
        for (size_t i = idx; i < n; ++i)
        {
            v[i] = v[i + 1];
        }

        v.resize(n);
    }
}

// Find the first appearances of given element in the vector.
template <class T> bool contains(const std::vector<T> &v, const T &element)
{
    size_t i = 0;
    size_t n = v.size();
    size_t idx = SIZE_MAX;

    while (i < n)
    {
        if (v[i] == element)
        {
            idx = i;
            break;
        }
        ++i;
    }

    return idx != SIZE_MAX;
}

// Perform binary search to find an index of given element in the vector.
// Returns SIZE_MAX when the element is not found.
template <class T>
size_t binarySearch(const std::vector<T> &v, const T &element)
{
    size_t idx = SIZE_MAX;

    if (v.size() > 0)
    {
        size_t first = 0;
        size_t last = v.size() - 1;

        while (first <= last)
        {
            size_t i = (first + last) / 2;
            if (element > v[i])
            {
                first = i + 1;
            }
            else if (element < v[i])
            {
                last = i - 1;
            }
            else
            {
                // found at index i
                idx = i;
                break;
            }
        }
    }

    return idx;
}

inline std::vector<std::string> split(const std::string &string, char c)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end;

    // Manual split on c.
    while ((end = string.find(c, start)) != std::string::npos)
    {
        tokens.push_back(string.substr(start, end - start));
        start = end + 1;
    }

    // Add the last token after the last c.
    tokens.push_back(string.substr(start));

    return tokens;
}

inline size_t threadId()
{
    return std::hash<std::thread::id>()(std::this_thread::get_id());
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
};

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

template <> inline float Numeric::min<float>()
{
    return -FLT_MAX;
}

template <> inline float Numeric::max<float>()
{
    return FLT_MAX;
}

template <> inline double Numeric::min<double>()
{
    return -DBL_MAX;
}

template <> inline double Numeric::max<double>()
{
    return DBL_MAX;
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

inline std::string toString(bool in)
{
    return (in) ? "true" : "false";
}

inline std::string toString(int in)
{
    return std::to_string(in);
}

inline std::string toString(unsigned int in)
{
    return std::to_string(in);
}

inline std::string toString(double in, int precision)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << in;
    std::string str = oss.str();

    size_t end = str.find_last_not_of('0');
    if (str[end] == '.')
    {
        end--;
    }
    str.erase(end + 1, std::string::npos);

    if (str.find('.') == std::string::npos)
    {
        str += ".0";
    }

    return str;
}

inline std::string toString(double in)
{
    return toString(in, 15);
}

inline std::string toString(long in)
{
    return std::to_string(in);
}

inline std::string toString(unsigned long in)
{
    return std::to_string(in);
}

inline std::string toString(long long in)
{
    return std::to_string(in);
}

inline std::string toString(unsigned long long in)
{
    return std::to_string(in);
}

inline void fromString(double &out, const std::string &in)
{
    auto index = in.find(".");
    if (index == std::string::npos)
    {
        out = static_cast<double>(std::stoll(in));
        return;
    }
    double whole = static_cast<double>(std::stoll(in.substr(0, index)));
    double fraction = static_cast<double>(std::stoll(in.substr(index + 1)));
    double exponent = static_cast<double>(in.substr(index + 1).size());
    out = whole + (fraction / std::pow(10.0, exponent));
}

inline bool toBool(const std::string &string)
{
    return toLower(string) == "true";
}

inline float toFloat(const std::string &string)
{
    return std::stof(string);
}

inline double toDouble(const std::string &string)
{
    return std::stod(string);
}

inline int toInt(const std::string &string)
{
    return static_cast<int>(std::stoll(string));
}

inline size_t toSize(const std::string &string)
{
    return static_cast<size_t>(std::stoull(string));
}

inline uint32_t toUint32(const std::string &string)
{
    return static_cast<uint32_t>(std::stoull(string));
}

inline uint64_t toUint64(const std::string &string)
{
    return static_cast<uint64_t>(std::stoull(string));
}

#include <WarningsEnable.hpp>

#endif /* UTIL_HPP */
