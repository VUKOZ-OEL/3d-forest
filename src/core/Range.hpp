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

#include <ExportCore.hpp>
#include <Json.hpp>

/** Range as [ minimum, maximum, minimumValue, maximumValue ]. */
template <class T> class Range : public std::array<T, 4>
{
public:
    Range();
    ~Range();

    void clear();

    void set(T min, T max, T minValue, T maxValue)
    {
        this->operator[](0) = min;
        this->operator[](1) = max;
        this->operator[](2) = minValue;
        this->operator[](3) = maxValue;
    }

    // Range with lower bound (minimum) and upper bound (maximum).
    void setMinimum(T v) { this->operator[](0) = v; }
    void setMaximum(T v) { this->operator[](1) = v; }

    T minimum() const { return this->operator[](0); }
    T maximum() const { return this->operator[](1); }

    // Selected part from the range.
    void setMinimumValue(T v) { this->operator[](2) = v; }
    void setMaximumValue(T v) { this->operator[](3) = v; }

    T minimumValue() const { return this->operator[](2); }
    T maximumValue() const { return this->operator[](3); }

    // Is minimumValue at minimum and maximumValue at maximum?
    bool hasBoundaryValues() const;

    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

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

template <class T> inline bool Range<T>::hasBoundaryValues() const
{
    constexpr T e = std::numeric_limits<T>::epsilon();

    if (this->operator[](2) - this->operator[](0) > e ||
        this->operator[](1) - this->operator[](3) > e)
    {
        return false;
    }

    return true;
}

template <class T> inline void Range<T>::read(const Json &in)
{
    this->operator[](0) = static_cast<T>(in["minimum"].number());
    this->operator[](1) = static_cast<T>(in["maximum"].number());
    this->operator[](2) = static_cast<T>(in["minimumValue"].number());
    this->operator[](3) = static_cast<T>(in["maximumValue"].number());
}

template <class T> inline Json &Range<T>::write(Json &out) const
{
    out["minimum"] = this->operator[](0);
    out["maximum"] = this->operator[](1);
    out["minimumValue"] = this->operator[](2);
    out["maximumValue"] = this->operator[](3);

    return out;
}

template <class T>
std::ostream &operator<<(std::ostream &os, const Range<T> &obj)
{
    return os << std::fixed << "(" << obj[0] << ", " << obj[1] << ", " << obj[2]
              << ", " << obj[3] << ")" << std::defaultfloat;
}

#endif /* RANGE_HPP */
