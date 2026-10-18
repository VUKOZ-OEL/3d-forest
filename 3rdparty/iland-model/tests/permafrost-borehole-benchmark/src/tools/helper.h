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

#ifndef HELPER_H
#define HELPER_H

#include <QtCore>
#include <QtCore/QRect>
#include <QtCore/QString>
#include <limits>
#define QUIETDEBUG(x) if (!Helper::quiet()) { qDebug() << x; }

/** Helper contains a bunch of (static) helper functions.
  * including simplifed functions to read/write plain text files (loadTextFile(), saveToTextFile()),
  * funcitons to show message dialogs (msg(), question()), and functions to control the amount of
  * debug outputs (quiet(), debugEnabled()).
  */
class Helper
{
public:
    Helper();
    static QString loadTextFile(const QString& fileName);
    static void saveToTextFile(const QString& fileName, const QString& text);
    static QByteArray loadFile(const QString &fileName);
    static void saveToFile(const QString &fileName, const QByteArray &data);
    static void msg(const QString &message, QWidget *parent=0);
    static bool question(const QString &message, QWidget *parent=0);
    static QString userValue(const QString &message, const QString defaultValue, QWidget *parent=0); ///< ask the user for a input value
    /// open a File Dialog and let the user choose a file.
    /// @return the filename selected by the user, an empty string if user cancels.
    static QString fileDialog(const QString &title, const QString &start_directory="", const QString &filter="", QWidget *parent=0);
    static bool quiet() { return m_NoDebug || m_quiet; }
    static bool debugEnabled() { return !m_NoDebug; }
    static void setQuiet(bool quiet) { m_quiet = quiet; }
    static void setDebugEnabled(bool enable) { m_NoDebug = !enable; }
    static void openHelp(const QString& topic);
    static QString stripHtml(const QString &source);



    static QString currentRevision(); ///< svn revision number

private:
    static bool m_quiet;
    static bool m_NoDebug;
};



/** UpdateState details missing.
  */
class UpdateState
{
public:
    // available states
    UpdateState(): mCurrentVal(0), mVal(0) {}
    bool needsUpdate(); // needs local state-object an update?
    void update(); // update with master
    int value() const { return mVal; } // return current value
    void invalidate(bool self=false); // master object enters a new state
    void addChild(UpdateState* state) { mChilds.push_back(state); }
    void saveState(UpdateState* state);
    bool hasChanged(UpdateState* state);
private:
    int mCurrentVal; // state of last explicit "update"
    int mVal; // current state
    QVector<UpdateState*> mChilds;
    QMap<UpdateState*, int> mSavedStates;
};


#endif // HELPER_H
