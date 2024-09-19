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

// Include std.
#include <array>

// Include 3D Forest.
#include <Json.hpp>
#include <Util.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Range as [ minimum, minimumValue, maximumValue, maximum ]. */
template <class T> class Range : public std::array<T, 4>
{
public:
    // Construct/Copy/Destroy.
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

    // Enabled.
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool enabled() const { return enabled_; }

    // Access.
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

    // Modifiers.
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

    // Operations.
    bool contains(T v) const;

protected:
    bool enabled_;

    template <class U> friend void fromJson(Range<U> &out, const Json &in);

    template <class U> friend void toJson(Json &out, const Range<U> &in);

    template <class U> friend std::string toString(const Range<U> &in);

    template <class U>
    friend std::ostream &operator<<(std::ostream &out, const Range<U> &in);
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
    return equal(minimum(), maximum());
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

template <class U> inline void fromJson(Range<U> &out, const Json &in)
{
    fromJson(out[0], in["minimum"]);
    fromJson(out[1], in["minimumValue"]);
    fromJson(out[2], in["maximumValue"]);
    fromJson(out[3], in["maximum"]);
    fromJson(out.enabled_, in["enabled"]);
}

template <class U> inline void toJson(Json &out, const Range<U> &in)
{
    toJson(out["minimum"], in[0]);
    toJson(out["minimumValue"], in[1]);
    toJson(out["maximumValue"], in[2]);
    toJson(out["maximum"], in[3]);
    toJson(out["enabled"], in.enabled_);
}

template <class U> inline std::string toString(const Range<U> &in)
{
    Json json;
    toJson(json, in);
    return json.serialize(0);
}

template <class U>
std::ostream &operator<<(std::ostream &out, const Range<U> &in)
{
    return out << toString(in);
}

#include <WarningsEnable.hpp>

#endif /* RANGE_HPP */
