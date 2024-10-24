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

/** @file ComputeTreeAttributesLeastSquaredRegression.cpp */

// Include 3D Forest.
#include <ComputeTreeAttributesLeastSquaredRegression.hpp>
#include <Util.hpp>

// Include local.
#define LOG_MODULE_NAME "ComputeTreeAttributesLeastSquaredRegression"
#define LOG_MODULE_DEBUG_ENABLED 1
#include <Log.hpp>

/*
    Circle fit to a given set of data points (in 2D)

    This is an algebraic fit, due to Taubin, based on the journal article

    G. Taubin, "Estimation Of Planar Curves, Surfaces And Nonplanar
                Space Curves Defined By Implicit Equations, With
                Applications To Edge And Range Image Segmentation",
                IEEE Trans. PAMI, Vol. 13, pages 1115-1138, (1991)

    Input:
        data - the class of data (contains the given points):

        data.n   - the number of data points
        data.X[] - the array of X-coordinates
        data.Y[] - the array of Y-coordinates

    Output:
        circle - parameters of the fitting circle:

        circle.a - the X-coordinate of the center of the fitting circle
        circle.b - the Y-coordinate of the center of the fitting circle
        circle.r - the radius of the fitting circle
        circle.s - the root mean square error (the estimate of sigma)
        circle.j - the total number of iterations

    The method is based on the minimization of the function

                sum [(x-a)^2 + (y-b)^2 - R^2]^2
            F = -------------------------------
                sum [(x-a)^2 + (y-b)^2]

    This method is more balanced than the simple Kasa fit.

    It works well whether data points are sampled along an entire circle or
    along a small arc.

    It still has a small bias and its statistical accuracy is slightly
    lower than that of the geometric fit (minimizing geometric distances),
    but slightly higher than that of the very similar Pratt fit.
    Besides, the Taubin fit is slightly simpler than the Pratt fit

    It provides a very good initial guess for a subsequent geometric fit.

    Nikolai Chernov (September 2012)
*/
void ComputeTreeAttributesLeastSquaredRegression::taubinFit(
    FittingCircle &circle,
    const std::vector<double> &points,
    const ComputeTreeAttributesParameters &parameters)
{
    circle.result = FittingCircle::RESULT_INVALID;

    size_t n = points.size() / 3;
    if (n < 1)
    {
        LOG_DEBUG(<< "Not enough points.");
        return;
    }

    // Calculate mean coordinates.
    double meanX{0.0};
    double meanY{0.0};
    double meanZ{0.0};

    for (size_t i = 0; i < n; i++)
    {
        meanX += points[(i * 3) + 0];
        meanY += points[(i * 3) + 1];
        meanZ += points[(i * 3) + 2];
    }

    meanX /= static_cast<double>(n);
    meanY /= static_cast<double>(n);
    meanZ /= static_cast<double>(n);

    // Calculate moments.
    double Mxx{0.0};
    double Myy{0.0};
    double Mxy{0.0};
    double Mxz{0.0};
    double Myz{0.0};
    double Mzz{0.0};

    for (size_t i = 0; i < n; i++)
    {
        double Xi = points[(i * 3) + 0] - meanX; // Centered x-coordinates.
        double Yi = points[(i * 3) + 1] - meanY; // Centered y-coordinates.
        double Zi = (Xi * Xi) + (Yi * Yi);

        Mxy += Xi * Yi;
        Mxx += Xi * Xi;
        Myy += Yi * Yi;
        Mxz += Xi * Zi;
        Myz += Yi * Zi;
        Mzz += Zi * Zi;
    }

    Mxx /= static_cast<double>(n);
    Myy /= static_cast<double>(n);
    Mxy /= static_cast<double>(n);
    Mxz /= static_cast<double>(n);
    Myz /= static_cast<double>(n);
    Mzz /= static_cast<double>(n);

    // Calculate coefficients of the characteristic polynomial.
    double Mz = Mxx + Myy;
    double Cov_xy = Mxx * Myy - Mxy * Mxy;
    double Var_z = Mzz - Mz * Mz;
    double A3 = 4.0 * Mz;
    double A2 = -3.0 * Mz * Mz - Mzz;
    double A1 = Var_z * Mz + 4.0 * Cov_xy * Mz - Mxz * Mxz - Myz * Myz;
    double A0 = Mxz * (Mxz * Myy - Myz * Mxy) + Myz * (Myz * Mxx - Mxz * Mxy) -
                Var_z * Cov_xy;
    double A22 = A2 + A2;
    double A33 = A3 + A3 + A3;

    // Finding the root of the characteristic polynomial
    // using Newton's method starting at x=0.
    // It is guaranteed to converge to the right root.
    double x{0.0};
    double y = A0;

    for (size_t i = 0; i < parameters.taubinFitIterationsMax; i++)
    {
        double Dy = A1 + x * (A22 + A33 * x);

        double xnew = x - y / Dy;
        if (equal(xnew, x) || (!std::isfinite(xnew)))
        {
            break;
        }

        double ynew = A0 + xnew * (A1 + xnew * (A2 + xnew * A3));
        if (!(std::abs(ynew) < std::abs(y)))
        {
            break;
        }

        x = xnew;
        y = ynew;
    }

    // Calculate parameters of the fitting circle.
    double DET = x * x - x * Mz + Cov_xy;
    double Xcenter = (Mxz * (Myy - x) - Myz * Mxy) / DET / 2.0;
    double Ycenter = (Myz * (Mxx - x) - Mxz * Mxy) / DET / 2.0;

    // Assemble the output.
    circle.a = Xcenter + meanX;
    circle.b = Ycenter + meanY;
    circle.z = meanZ;
    circle.r = std::sqrt(Xcenter * Xcenter + Ycenter * Ycenter + Mz);
    circle.result = FittingCircle::RESULT_OK;
}

void ComputeTreeAttributesLeastSquaredRegression::geometricCircle(
    FittingCircle &circle,
    const std::vector<double> &points,
    const ComputeTreeAttributesParameters &parameters)
{
    circle.result = FittingCircle::RESULT_INVALID;

    size_t n = points.size() / 3;
    if (n < 1)
    {
        LOG_DEBUG(<< "Not enough points.");
        return;
    }

    // Calculate mean coordinates.
    double meanX{0.0};
    double meanY{0.0};
    double meanZ{0.0};

    for (size_t i = 0; i < n; i++)
    {
        meanX += points[(i * 3) + 0];
        meanY += points[(i * 3) + 1];
        meanZ += points[(i * 3) + 2];
    }

    if (zero(meanX))
    {
        meanX = 1e-10;
    }

    if (zero(meanY))
    {
        meanY = 1e-10;
    }

    if (zero(meanZ))
    {
        meanZ = 1e-10;
    }

    meanX /= static_cast<double>(n);
    meanY /= static_cast<double>(n);
    meanZ /= static_cast<double>(n);

    // Initialize iterations.
    double lambda = 1e-4;
    size_t nIterations = 0;
    size_t nInnerIterations = 0;

    FittingCircle Old;

    double Mu, Mv, Muu, Mvv, Muv, Mr;
    double F1, F2, F3;
    double UUl, VVl, Nl;
    double G11, G12, G13, G22, G23, G33;
    double D1, D2, D3;
    double dR, dY, dX;

    // Starting with the given initial circle (initial guess).
    FittingCircle New;
    New.a = circle.a;
    New.b = circle.b;
    New.r = circle.r;
    New.s = sigma(New, points);

nextIteration:

    Old = New;

    nIterations++;
    if (nIterations > parameters.geometricCircleIterationsMax)
    {
        circle.result = FittingCircle::RESULT_MAX_ITERATIONS_REACHED;
        goto enough;
    }

    // Calculate moments.
    Mu = 1e-9;
    Mv = 1e-9;
    Muu = 1e-9;
    Mvv = 1e-9;
    Muv = 1e-9;
    Mr = 1e-9;

    for (size_t i = 0; i < n; i++)
    {
        double dx = points[(i * 3) + 0] - Old.a;
        if (zero(dx))
        {
            dx = 1e-10;
        }

        double dy = points[(i * 3) + 1] - Old.b;
        if (zero(dy))
        {
            dy = 1e-10;
        }

        double ri = std::sqrt(dx * dx + dy * dy);
        double u = dx / ri;
        double v = dy / ri;
        Mu += u;
        Mv += v;
        Muu += u * u;
        Mvv += v * v;
        Muv += u * v;
        Mr += ri;
    }

    Mu /= static_cast<double>(n);
    Mv /= static_cast<double>(n);
    Muu /= static_cast<double>(n);
    Mvv /= static_cast<double>(n);
    Muv /= static_cast<double>(n);
    Mr /= static_cast<double>(n);

    // Calculate matrices.
    F1 = Old.a + Old.r * Mu - meanX;
    F2 = Old.b + Old.r * Mv - meanY;
    F3 = Old.r - Mr;

    Old.g = New.g = std::sqrt(F1 * F1 + F2 * F2 + F3 * F3);

tryAgain:

    UUl = Muu + lambda;
    VVl = Mvv + lambda;
    Nl = 1.0 + lambda;

    // Cholesky decomposition.
    G11 = sqrt(UUl);
    G12 = Muv / G11;
    G13 = Mu / G11;
    G22 = sqrt(VVl - G12 * G12);
    G23 = (Mv - G12 * G13) / G22;
    G33 = sqrt(Nl - G13 * G13 - G23 * G23);

    D1 = F1 / G11;
    D2 = (F2 - G12 * D1) / G22;
    D3 = (F3 - G13 * D1 - G23 * D2) / G33;

    dR = D3 / G33;
    dY = (D2 - G23 * dR) / G22;
    dX = (D1 - G12 * dY - G13 * dR) / G11;

    if ((std::abs(dR) + std::abs(dX) + std::abs(dY)) / (1.0 + Old.r) < 3e-8)
    {
        circle.result = FittingCircle::RESULT_OK;
        goto enough;
    }

    // Updating the parameters.

    New.a = Old.a - dX;
    New.b = Old.b - dY;

    if (std::abs(New.a) > parameters.geometricCircleParameterLimit ||
        std::abs(New.b) > parameters.geometricCircleParameterLimit)
    {
        circle.result = FittingCircle::RESULT_PARAMETER_LIMIT_REACHED;
        goto enough;
    }

    New.r = Old.r - dR;

    if (!(New.r > 0.0))
    {
        lambda *= parameters.geometricCircleFactorUp;

        if (++nInnerIterations > parameters.geometricCircleIterationsMax)
        {
            circle.result = FittingCircle::RESULT_MAX_INNER_ITERATIONS_REACHED;
            goto enough;
        }

        goto tryAgain;
    }

    // Calculate the root-mean-square error.
    New.s = sigma(New, points);

    // Check if improvement is gained.
    if (New.s < Old.s)
    {
        // Yes, improvement.
        lambda *= parameters.geometricCircleFactorDown;
        goto nextIteration;
    }
    else
    {
        // No improvement.
        if (++nInnerIterations > parameters.geometricCircleIterationsMax)
        {
            circle.result = FittingCircle::RESULT_MAX_INNER_ITERATIONS_REACHED;
            goto enough;
        }

        lambda *= parameters.geometricCircleFactorUp;
        goto tryAgain;
    }

    // Set the output and exit.
enough:

    circle.a = Old.a;
    circle.b = Old.b;
    circle.z = meanZ;
    circle.r = Old.r;
    circle.i = nIterations;
    circle.j = nInnerIterations;
}

double ComputeTreeAttributesLeastSquaredRegression::sigma(
    FittingCircle &circle,
    const std::vector<double> &points)
{
    double sum{0.0};

    size_t n = points.size() / 3;
    for (size_t i = 0; i < n; i++)
    {
        double dx = points[(i * 3) + 0] - circle.a;
        double dy = points[(i * 3) + 1] - circle.b;

        sum += ((std::sqrt(dx * dx + dy * dy) - circle.r) *
                (std::sqrt(dx * dx + dy * dy) - circle.r));
    }

    if (zero(sum))
    {
        sum = 1e-10;
    }

    return std::sqrt(sum / static_cast<double>(n));
}
