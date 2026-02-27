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
#ifndef FOMESTP_H
#define FOMESTP_H

#include "fmstand.h"
#include "activity.h"
#include "actsalvage.h"

#include <QJSValue>
#include <QList>

class Expression; // forward

namespace ABE {

/**
 * @brief The FMSTP class encapsulates one "stand treatment program", that consists of several "activities".
 */
class Activity; // forward

class FMSTP
{
public:
    FMSTP();
    ~FMSTP();
    const QString &name() const {return mName; }
    /// returns the (first) Activity with the name 'name', or 0 if the activity could not be found.
    Activity *activity(const QString &name) const;
    /// gets the index of an activity given by a pointer
    int activityIndex(Activity* act) { return mActivities.indexOf(act); }
    /// gets a const list of activities
    const QVector<Activity*> activities() const { return mActivities; }

    /// read the options from a javascript structure / object
    void setup(QJSValue &js_value, const QString &name);
    /// defaultFlags() is used to initalized the flags for indiv. forest stands
    QVector<ActivityFlags> defaultFlags() {return mActivityStand; }
    Events &events() { return mEvents; }
    QJSValue *JSoptions() { return &mOptions; }

    /// rotation length (years)
    int rotationLengthOfType(const int type) { if (type>0 && type<4) return mRotationLength[type-1]; return 0;}
    int rotationLengthType(const int length) const { for (int i=0;i<3;++i) if (mRotationLength[i]==length) return i+1; return -1; } // TODO: fix
    ActSalvage *salvageActivity() const { return mSalvage; }

    /// run repeating activities
    bool executeRepeatingActivities(FMStand *stand);
    // signals
    bool signal(QString signalstr, FMStand *stand, QJSValue parameter=QJSValue());

    /// evaluate bound expressions for all activities of the STP for the given stand
    void evaluateDynamicExpressions(FMStand *stand);
    // helper functions
    QString info();
    /// if verbose is true, detailed debug information is provided.
    static void setVerbose(bool verbose) {mVerbose = verbose; }
    static bool verbose()  {return mVerbose; } ///< returns true in debug mode
    /// get a property of 'js_value' with the name 'key'. If 'errorMessage' is given, an error is thrown when the key does not exist.
    /// If key is not present and a 'default_value' is given, it is returned. Otherwise, an "undefined" JS value is returned.
    static QJSValue valueFromJs(const QJSValue &js_value, const QString &key, const QString default_value=QString(), const QString &errorMessage=QString());
    static bool boolValueFromJs(const QJSValue &js_value, const QString &key, const bool default_bool_value, const QString &errorMessage=QString());

    static bool checkObjectProperties(const QJSValue &js_value, const QStringList &allowed_properties, const QString &errorMessage=QString());
    ///
    static QJSValue evaluateJS(QJSValue value);

private:
    void internalSetup(const QJSValue &js_value, int level=0);
    QString mName; ///< the name of the stand treatment program
    void setupActivity(const QJSValue &js_value, const QString &name);
    void clear(); ///< remove all activites
    Events mEvents;
    static bool mVerbose; ///< debug mode
    bool mHasRepeatingActivities; ///< true, if the STP contains repeating activities
    QVector<Activity*> mActivities; ///< container for the activities of the STP
    QVector<ActivityFlags> mActivityStand; ///< base data for stand-specific STP info.
    QStringList mActivityNames;  ///< names of all available activities
    // special activities
    ActSalvage *mSalvage;

    // STP-level properties
    int mRotationLength[3]; ///< three levels (low, medium,high) of rotation length

    QJSValue mOptions; ///< user-defined options of the STP

};

} // namespace

#endif // FOMESTP_H
