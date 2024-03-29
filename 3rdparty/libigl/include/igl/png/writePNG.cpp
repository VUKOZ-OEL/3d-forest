// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2016 Daniele Panozzo <daniele.panozzo@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#include "writePNG.h"
#include <stb_image_write.h>

IGL_INLINE bool igl::png::writePNG(
  const Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic>& R,
  const Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic>& G,
  const Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic>& B,
  const Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic>& A,
  const std::string png_file
)
{
  assert((R.rows() == G.rows()) && (G.rows() == B.rows()) && (B.rows() == A.rows()));
  assert((R.cols() == G.cols()) && (G.cols() == B.cols()) && (B.cols() == A.cols()));

  const int comp = 4;                                  // 4 Channels Red, Green, Blue, Alpha
  const int stride_in_bytes = R.rows()*comp;           // Length of one row in bytes
  std::vector<unsigned char> data(R.size()*comp,0);     // The image itself;

  for (unsigned i = 0; i<R.rows();++i)
  {
    for (unsigned j = 0; j < R.cols(); ++j)
    {
        data[(j * R.rows() * comp) + (i * comp) + 0] = R(i,R.cols()-1-j);
        data[(j * R.rows() * comp) + (i * comp) + 1] = G(i,R.cols()-1-j);
        data[(j * R.rows() * comp) + (i * comp) + 2] = B(i,R.cols()-1-j);
        data[(j * R.rows() * comp) + (i * comp) + 3] = A(i,R.cols()-1-j);
    }
  }

  stbi_write_png(png_file.c_str(), R.rows(), R.cols(), comp, data.data(), stride_in_bytes);

  return true;
}

IGL_INLINE bool igl::png::writePNG(
  const Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic>& I,
  const std::string png_file
)
{
  const int comp = 1;                                  // 1 Channel
  const int stride_in_bytes = I.rows()*comp;           // Length of one row in bytes
  std::vector<unsigned char> data(I.size()*comp,0);    // The image itself;

  for (unsigned i = 0; i < I.rows(); ++i)
  {
    for (unsigned j = 0; j < I.cols(); ++j)
    {
        data[(j * I.rows()) + i] = I(i,I.cols()-1-j);
    }
  }

  stbi_write_png(png_file.c_str(), I.rows(), I.cols(), comp, data.data(), stride_in_bytes);

  return true;
}

#ifdef IGL_STATIC_LIBRARY
// Explicit template instantiation
// generated by autoexplicit.sh
#endif
