// This file is part of libigl, a simple c++ geometry processing library.
// 
// Copyright (C) 2013 Alec Jacobson <alecjacobson@gmail.com>
// 
// This Source Code Form is subject to the terms of the Mozilla Public License 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.

// Modifications Copyright 2020-present VUKOZ
// Disabled useful compiler warnings which are present in libigl

// This should *NOT* be contained in a IGL_*_H ifdef, since it may be defined
// differently based on when it is included
#ifdef IGL_INLINE
#undef IGL_INLINE
#endif

#ifndef IGL_STATIC_LIBRARY
#  define IGL_INLINE inline
#else
#  define IGL_INLINE
#endif

#ifndef IGL_INLINE_H
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#pragma GCC diagnostic push
// The following warnings are unfortunately useful, they can detect bugs and exploits
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wold-style-cast"
// Good practice
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#endif /* IGL_INLINE_H */