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

#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <stdexcept>
#include <QtCore/QString>
/** Exception IException is the iLand model exception class.
    The class uses a string to store exception messages.
  */
class IException : public std::runtime_error {
 public:
   ~IException () throw() {  }
   IException() : std::runtime_error("iLand model exception.") { }
   //IException(QString msg)  { GlobalSettings::instance()->addErrorMessage(msg); }
   //QString toString() const { return GlobalSettings::instance()->errorMessage(); }
   IException(QString msg) : std::runtime_error("iLand model exception.") { add(msg); }
   const QString &message() const { return mMsg; }
   void add(const QString &msg) { if(!mMsg.isEmpty()) mMsg+="\n"; mMsg += msg; }
private:
   QString mMsg;
};


#endif // EXCEPTION_H
