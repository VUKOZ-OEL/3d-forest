// This file is part of libigl, a simple c++ geometry processing library.
// 
// Copyright 2020-present VUKOZ
// 
// This Source Code Form is subject to the terms of the Mozilla Public License 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.
//
// Disable compiler warnings which are present in libigl

#ifndef IGL_WARNINGS_DISABLED
#define IGL_WARNINGS_DISABLED

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
    #pragma GCC diagnostic ignored "-Wswitch-default"
    #pragma GCC diagnostic ignored "-Wcast-qual"
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif /* __GNUC__ */

#endif /* IGL_WARNINGS_DISABLED */
