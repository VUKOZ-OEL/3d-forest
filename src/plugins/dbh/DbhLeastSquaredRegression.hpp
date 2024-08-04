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

/** @file DbhLeastSquaredRegression.hpp */

#ifndef DBH_LEAST_SQUARED_REGRESSION_HPP
#define DBH_LEAST_SQUARED_REGRESSION_HPP

// Include 3D Forest.
#include <DbhGroup.hpp>
#include <DbhParameters.hpp>

/** DBH (Diameter at Breast Height) Least Squared Regression. */
class DbhLeastSquaredRegression
{
public:
    class FittingCircle
    {
    public:
        double a{0.0}; /**< X-coordinate of the center of the fitting circle. */
        double b{0.0}; /**< Y-coordinate of the center of the fitting circle. */
        double z{0.0}; /**< Z-coordinate of the center of the fitting circle. */

        double r{0.0}; /**< Radius of the fitting circle. */

        double s{0.0}; /**< Root mean square error (the estimate of sigma). */
        double g{0.0};
        size_t i{0}; /**< Total number of outer iterations. */
        size_t j{0}; /**< Total number of inner iterations (lambda). */

        bool operator<(const FittingCircle &obj) const { return i < obj.i; }
    };

    static void taubinFit(FittingCircle &circle,
                          const DbhGroup &group,
                          const DbhParameters &parameters);

    static void geometricCircle(FittingCircle &circle,
                                const DbhGroup &group,
                                const DbhParameters &parameters);

    static double sigma(FittingCircle &circle, const DbhGroup &group);
};

#endif /* DBH_LEAST_SQUARED_REGRESSION_HPP */
