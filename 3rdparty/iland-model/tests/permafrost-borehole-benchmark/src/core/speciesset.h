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

#ifndef SPECIESSET_H
#define SPECIESSET_H
#include <QtSql>

#include "stampcontainer.h"
#include "expression.h"
class Species;
class SeedDispersal;

class SpeciesSet
{
public:
    SpeciesSet();
    ~SpeciesSet();
    const QString &name() const { return mName; } ///< table name of the species set
    // access
    QList<Species*> activeSpecies() const { return mActiveSpecies; } ///< list of species that are "active" (flag active in database)
    Species *species(const QString &speciesId) const { return mSpecies.value(speciesId); }
    const Species *species(const int &index); ///< get by arbirtray index (slower than using string-id!)
    const StampContainer &readerStamps() { return mReaderStamp; }
    bool hasVar(const QString& varName); ///< test if variable exists
    QVariant var(const QString& varName); ///< return variable as QVariant
    int count() const { return mSpecies.count(); }
    /// return 2 iterators. The range between 'rBegin' and 'rEnd' are indices of the current species set (all species are included, order is random).
    void randomSpeciesOrder(QVector<int>::const_iterator &rBegin, QVector<int>::const_iterator &rEnd);
    // properties
    // calculations
    double nitrogenResponse(const double availableNitrogen, const double &responseClass) const;
    double co2Response(const double ambientCO2, const double nitrogenResponse, const double soilWaterResponse) const;
    double lightResponse(const double lightResourceIndex, const double lightResponseClass) const;
    double LRIcorrection(const double lightResourceIndex, const double relativeHeight) const  { return mLRICorrection.calculate(lightResourceIndex, relativeHeight);}
    // maintenance
    void clear();
    int setup();
    void setupRegeneration(); ///< setup of regenartion related data
    // running
    void newYear(); ///< is called at the beginning of a year
    void regeneration(); ///< run regeneration (after growth)
    void clearSaplingSeedMap(); ///< clear the seed maps that collect leaf area for saplings
private:
    QString mName;
    double nitrogenResponse(const double &availableNitrogen, const double &NA, const double &NB) const;
    void createRandomSpeciesOrder();
    QList<Species*> mActiveSpecies; ///< list of species that are "active" (flag active in database)
    QMap<QString, Species*> mSpecies;
    static const int mNRandomSets = 20;
    QVector<int> mRandomSpeciesOrder;
    QSqlQuery *mSetupQuery;
    StampContainer mReaderStamp;
    // nitrogen response classes
    double mNitrogen_1a, mNitrogen_1b; ///< parameters of nitrogen response class 1
    double mNitrogen_2a, mNitrogen_2b; ///< parameters of nitrogen response class 2
    double mNitrogen_3a, mNitrogen_3b; ///< parameters of nitrogen response class 3
    // CO2 response
    double mCO2base, mCO2comp; ///< CO2 concentration of measurements (base) and CO2 compensation point (comp)
    double mCO2p0, mCO2beta0; ///< p0: production multiplier, beta0: relative productivity increase
    // Light Response classes
    Expression mLightResponseIntolerant; ///< light response function for the the most shade tolerant species
    Expression mLightResponseTolerant; ///< light response function for the most shade intolerant species
    Expression mLRICorrection; ///< function to modfiy LRI during read

};

#endif // SPECIESSET_H
