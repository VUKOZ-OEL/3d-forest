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

/** @file Range.hpp */

#ifndef RANGE_HPP
#define RANGE_HPP

#include <Json.hpp>
#include <Util.hpp>

#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Range as [ minimum, minimumValue, maximumValue, maximum ]. */
template <class T> class Range : public std::array<T, 4>
{
public:
    // construct/copy/destroy
    Range();

    Range(T min, T max)
    {
        this->operator[](0) = min;
        this->operator[](1) = min;
        this->operator[](2) = max;
        this->operator[](3) = max;
        enabled_ = true;
    }

    Range(T min, T minValue, T maxValue, T max)
    {
        this->operator[](0) = min;
        this->operator[](1) = minValue;
        this->operator[](2) = maxValue;
        this->operator[](3) = max;
        enabled_ = true;
    }

    ~Range();

    // enabled
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

    // access
    bool empty() const;
    bool full() const;

    // Range with lower bound (minimum) and upper bound (maximum).
    void setMinimum(T v) { this->operator[](0) = v; }
    void setMaximum(T v) { this->operator[](3) = v; }

    T minimum() const { return this->operator[](0); }
    T maximum() const { return this->operator[](3); }

    // Selected part from the range.
    void setMinimumValue(T v) { this->operator[](1) = v; }
    void setMaximumValue(T v) { this->operator[](2) = v; }

    T minimumValue() const { return this->operator[](1); }
    T maximumValue() const { return this->operator[](2); }

    // modifiers
    void clear();

    void set(T min, T max)
    {
        this->operator[](0) = min;
        this->operator[](1) = min;
        this->operator[](2) = max;
        this->operator[](3) = max;
    }

    void set(T min, T minValue, T maxValue, T max)
    {
        this->operator[](0) = min;
        this->operator[](1) = minValue;
        this->operator[](2) = maxValue;
        this->operator[](3) = max;
    }

    // operations
    bool contains(T v) const;

    // I/O
    void read(const Json &in);
    Json &write(Json &out) const;

private:
    bool enabled_;
};

template <class T> inline Range<T>::Range()
{
    clear();
}

template <class T> inline Range<T>::~Range()
{
}

template <class T> inline void Range<T>::clear()
{
    this->operator[](0) = 0;
    this->operator[](1) = 0;
    this->operator[](2) = 0;
    this->operator[](3) = 0;
    enabled_ = true;
}

template <class T> inline bool Range<T>::empty() const
{
    return isEqual(minimum(), maximum());
}

template <class T> inline bool Range<T>::full() const
{
    constexpr T e = std::numeric_limits<T>::epsilon();

    if (this->operator[](1) - this->operator[](0) > e ||
        this->operator[](3) - this->operator[](2) > e)
    {
        return false;
    }

    return true;
}

template <class T> inline bool Range<T>::contains(T v) const
{
    return !(v < minimumValue() || v > maximumValue());
}

template <class T> inline void Range<T>::read(const Json &in)
{
    this->operator[](0) = static_cast<T>(in["minimum"].number());
    this->operator[](1) = static_cast<T>(in["minimumValue"].number());
    this->operator[](2) = static_cast<T>(in["maximumValue"].number());
    this->operator[](3) = static_cast<T>(in["maximum"].number());
}

template <class T> inline Json &Range<T>::write(Json &out) const
{
    out["minimum"] = this->operator[](0);
    out["minimumValue"] = this->operator[](1);
    out["maximumValue"] = this->operator[](2);
    out["maximum"] = this->operator[](3);

    return out;
}

template <class T>
std::ostream &operator<<(std::ostream &os, const Range<T> &obj)
{
    return os << std::fixed << "(" << obj[0] << ", " << obj[1] << ", " << obj[2]
              << ", " << obj[3] << ")" << std::defaultfloat;
}

#include <WarningsEnable.hpp>

#endif /* RANGE_HPP */
