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

/** @file DescriptorAlgorithmParameters.hpp */

#ifndef DESCRIPTOR_ALGORITHM_PARAMETERS_HPP
#define DESCRIPTOR_ALGORITHM_PARAMETERS_HPP

#include <cstdint>
#include <sstream>

/** Descriptor Algorithm Parameters. */
class DescriptorAlgorithmParameters
{
public:
    enum Method
    {
        METHOD_PCA = 0,
        METHOD_DENSITY = 1
    };

    int neighborhoodRadius;
    int method;

    DescriptorAlgorithmParameters() { setDefault(); }

    void clear()
    {
        neighborhoodRadius = 0;
        method = METHOD_PCA;
    }

    void setDefault()
    {
        neighborhoodRadius = 100;
        method = METHOD_PCA;
    }

    void set(int neighborhoodRadius_, Method method_)
    {
        neighborhoodRadius = neighborhoodRadius_;
        method = method_;
    }
};

inline std::ostream &operator<<(std::ostream &os,
                                const DescriptorAlgorithmParameters &obj)
{
    // clang-format off
    return os << "neighborhoodRadius <" << obj.neighborhoodRadius
              << "> method <" << obj.method
              << ">";
    // clang-format on
}

#endif /* DESCRIPTOR_ALGORITHM_PARAMETERS_HPP */
