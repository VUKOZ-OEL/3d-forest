// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2013 Alec Jacobson <alecjacobson@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

// Modifications Copyright 2020-present VUKOZ
// Add EIGEN_HAS_OPENMP condition to silence warning when used without -fopenmp

#include "project_to_line_segment.h"
#include "project_to_line.h"
#include <Eigen/Core>

template <
  typename DerivedP,
  typename DerivedS,
  typename DerivedD,
  typename Derivedt,
  typename DerivedsqrD>
IGL_INLINE void igl::project_to_line_segment(
  const Eigen::MatrixBase<DerivedP> & P,
  const Eigen::MatrixBase<DerivedS> & S,
  const Eigen::MatrixBase<DerivedD> & D,
  Eigen::PlainObjectBase<Derivedt> & t,
  Eigen::PlainObjectBase<DerivedsqrD> & sqrD)
{
  project_to_line(P,S,D,t,sqrD);
  const int np = P.rows();
  // loop over points and fix those that projected beyond endpoints
#ifdef EIGEN_HAS_OPENMP
#pragma omp parallel for if (np>10000)
#endif
  for(int p = 0;p<np;p++)
  {
    const DerivedP Pp = P.row(p);
    if(t(p)<0)
    {
      sqrD(p) = (Pp-S).squaredNorm();
      t(p) = 0;
    }else if(t(p)>1)
    {
      sqrD(p) = (Pp-D).squaredNorm();
      t(p) = 1;
    }
  }
}

#ifdef IGL_STATIC_LIBRARY
// Explicit template instantiation
// generated by autoexplicit.sh
template void igl::project_to_line_segment<Eigen::Matrix<float, 1, -1, 1, 1, -1>, Eigen::Matrix<float, 1, 3, 1, 1, 3>, Eigen::Matrix<float, 1, 3, 1, 1, 3>, Eigen::Matrix<double, 1, 1, 0, 1, 1>, Eigen::Matrix<double, 1, 1, 0, 1, 1> >(Eigen::MatrixBase<Eigen::Matrix<float, 1, -1, 1, 1, -1> > const&, Eigen::MatrixBase<Eigen::Matrix<float, 1, 3, 1, 1, 3> > const&, Eigen::MatrixBase<Eigen::Matrix<float, 1, 3, 1, 1, 3> > const&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 1, 0, 1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 1, 0, 1, 1> >&);
// generated by autoexplicit.sh
template void igl::project_to_line_segment<Eigen::Matrix<double, 1, -1, 1, 1, -1>, Eigen::Matrix<double, 1, 3, 1, 1, 3>, Eigen::Matrix<double, 1, 3, 1, 1, 3>, Eigen::Matrix<double, 1, 1, 0, 1, 1>, Eigen::Matrix<double, 1, 1, 0, 1, 1> >(Eigen::MatrixBase<Eigen::Matrix<double, 1, -1, 1, 1, -1> > const&, Eigen::MatrixBase<Eigen::Matrix<double, 1, 3, 1, 1, 3> > const&, Eigen::MatrixBase<Eigen::Matrix<double, 1, 3, 1, 1, 3> > const&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 1, 0, 1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 1, 0, 1, 1> >&);
// generated by autoexplicit.sh
template void igl::project_to_line_segment<Eigen::Matrix<float, 1, 3, 1, 1, 3>, Eigen::Matrix<float, 1, 3, 1, 1, 3>, Eigen::Matrix<float, 1, 3, 1, 1, 3>, Eigen::Matrix<double, 1, 1, 0, 1, 1>, Eigen::Matrix<double, 1, 1, 0, 1, 1> >(Eigen::MatrixBase<Eigen::Matrix<float, 1, 3, 1, 1, 3> > const&, Eigen::MatrixBase<Eigen::Matrix<float, 1, 3, 1, 1, 3> > const&, Eigen::MatrixBase<Eigen::Matrix<float, 1, 3, 1, 1, 3> > const&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 1, 0, 1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 1, 0, 1, 1> >&);
// generated by autoexplicit.sh
template void igl::project_to_line_segment<Eigen::Matrix<double, 1, 2, 1, 1, 2>, Eigen::Matrix<double, 1, 2, 1, 1, 2>, Eigen::Matrix<double, 1, 2, 1, 1, 2>, Eigen::Matrix<double, 1, 1, 0, 1, 1>, Eigen::Matrix<double, 1, 1, 0, 1, 1> >(Eigen::MatrixBase<Eigen::Matrix<double, 1, 2, 1, 1, 2> > const&, Eigen::MatrixBase<Eigen::Matrix<double, 1, 2, 1, 1, 2> > const&, Eigen::MatrixBase<Eigen::Matrix<double, 1, 2, 1, 1, 2> > const&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 1, 0, 1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 1, 0, 1, 1> >&);
template void igl::project_to_line_segment<Eigen::Matrix<double, 1, 3, 1, 1, 3>, Eigen::Matrix<double, 1, 3, 1, 1, 3>, Eigen::Matrix<double, 1, 3, 1, 1, 3>, Eigen::Matrix<double, 1, 1, 0, 1, 1>, Eigen::Matrix<double, 1, 1, 0, 1, 1> >(Eigen::MatrixBase<Eigen::Matrix<double, 1, 3, 1, 1, 3> > const&, Eigen::MatrixBase<Eigen::Matrix<double, 1, 3, 1, 1, 3> > const&, Eigen::MatrixBase<Eigen::Matrix<double, 1, 3, 1, 1, 3> > const&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 1, 0, 1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 1, 0, 1, 1> >&);
#endif
