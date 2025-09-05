//    This file is part of 3DFOREST  www.3dforest.eu
//
//    3DFOREST is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    3DFOREST is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with 3DFOREST.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef stred_h
#define stred_h

namespace f3d
{

//! Struct consist for hold tree centre and diameter.
/*! Struct of holding information about computed tree DBH with coordinates of centre (a,b,z) radius (r) and voting value when computing (i) . */
struct stred {
    float a;    /**< the X-coordinate of the center of the fitting circle */
    float b;    /**< the Y-coordinate of the center of the fitting circle */
    float z;    /**< the Z-coordinate of the center of the fitting circle */
    float i;    /**< the total number of outer iterations, can be removed */
    float r;    /**< the radius of the fitting circle */
    bool operator < (const stred& str) const {return (i < str.i);}
};

}

#endif // stred_h
