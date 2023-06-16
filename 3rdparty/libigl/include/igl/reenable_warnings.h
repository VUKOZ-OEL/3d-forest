// This file is part of libigl, a simple c++ geometry processing library.
// 
// Copyright 2020-present VUKOZ
// 
// This Source Code Form is subject to the terms of the Mozilla Public License 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.
//
// Reenable compiler warnings which are present in libigl

#ifdef IGL_WARNINGS_DISABLED
#undef IGL_WARNINGS_DISABLED

#ifdef _MSC_VER
    #pragma warning( pop )
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
    #pragma GCC diagnostic pop
#endif /* __GNUC__ */

#endif /* IGL_WARNINGS_DISABLED */
