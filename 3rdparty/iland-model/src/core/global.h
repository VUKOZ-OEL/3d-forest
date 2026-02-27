/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

#define MSGRETURN(x) { qDebug() << x; return; }
#define WARNINGRETURN(x) { qWarning() << x; return; }
#define ERRORRETURN(x) { qError() << x; return; }
// conversions rad/degree
#define RAD(x) (x*M_PI/180.)
#define GRAD(x) (x/M_PI*180.)
#define PI2 2*M_PI

#include <cstdlib>
#include "math.h"
#include "exception.h"
// general datatypes
//typedef int TreeSpecies;

// global debug helpers (used by macros!)
void dbg_helper(const char *where, const char *what,const char* file,int line);
void dbg_helper_ext(const char *where, const char *what,const char* file,int line, const QString &s);

// change to enabled detailed debug messages.
// if NO_DEBUG_MSGS is defined, NO debug outputs are generated.
// NO_DEBUG_MSG
//#if !defined(QT_DEBUG)
//#define NO_DEBUG_MSGS
//#endif

#if !defined(DBG_IF)
#  ifndef NO_DEBUG_MSGS
#    define DBG_IF(cond, where, what) ((cond) ? dbg_helper(where, what, __FILE__, __LINE__) : qt_noop())
#  else
#    define DBG_IF(cond, where, what) qt_noop()
#  endif
#endif

#if !defined(DBG_IF_X)
#  ifndef NO_DEBUG_MSGS
#    define DBG_IF_X(cond, where, what,more) ((cond) ? dbg_helper_ext(where, what, __FILE__, __LINE__,more) : qt_noop())
#  else
#    define DBG_IF_X(cond, where, what,more) qt_noop()
#  endif
#endif

#if !defined(DBGMODE)
#  ifndef NO_DEBUG_MSGS
#    define DBGMODE(stmts) { stmts }
#  else
#    define DBGMODE(stmts) qt_noop()
#  endif
#endif

// log level functions
bool logLevelDebug(); // true, if detailed debug information is logged
bool logLevelInfo(); // true, if only important aggreate info is logged
bool logLevelWarning(); // true if only severe warnings/errors are logged.
void setLogLevel(int loglevel); // setter function

// the random number generator:
#include "randomgenerator.h"


inline double limit(const double value, const double lower, const double upper)
{
    return qMax(qMin(value, upper), lower);
}
inline int limit(const int value, const int lower, const int upper)
{
    return qMax(qMin(value, upper), lower);
}
inline void setBit(unsigned int &rTarget, const int bit, const bool value)
{
    if (value)
        rTarget |= (1 << bit);  // set bit
    else
        rTarget &= ~(1 << bit); // clear bit
        // rTarget &= ( (1 << bit) ^ 0xffffff ); // clear bit
}
inline bool isBitSet(const unsigned int value, const int bit)
{
    return value & (1 << bit);
}

// define a global isnan() function
// #ifndef isnan
// #define isnan(x) ((x) != (x))
// #endif

#include "globalsettings.h"

#endif // GLOBAL_H
