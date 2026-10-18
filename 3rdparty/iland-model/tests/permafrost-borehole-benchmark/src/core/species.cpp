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

/** @class Species
  @ingroup core
  The behavior and general properties of tree species.
  Because the individual trees are designed as leightweight as possible, lots of stuff is done by the Species.
  Inter alia, Species do:
  - store all the precalcualted patterns for light competition (LIP, stamps)
  - do most of the growth (3PG) calculation
  */


#include <QtCore>
#include "globalsettings.h"

#include "species.h"
#include "speciesset.h"
#include "stampcontainer.h"
#include "exception.h"
#include "seeddispersal.h"
#include "tree.h"


Species::~Species()
{
    if (mSeedDispersal)
        delete mSeedDispersal;
    mSeedDispersal = nullptr;
}

/** main setup routine for tree species.
  Data is fetched from the open query (or file, ...) in the parent SpeciesSet using xyzVar() functions.
  This is called
*/
void Species::setup()
{
    Q_ASSERT(mSet != nullptr);
    const XmlHelper &xml=GlobalSettings::instance()->settings();

    // setup general information
    mId = stringVar("shortName");
    mName = stringVar("name");
#ifdef ILAND_GUI
    QString col_name = '#' + stringVar("displayColor");
    mDisplayColor.setNamedColor(col_name);
#else
    mDisplayColor = 0;
#endif
    QString stampFile = stringVar("LIPFile");
    // load stamps
    mLIPs.load( GlobalSettings::instance()->path(stampFile, "lip") );
    // attach writer stamps to reader stamps
    mLIPs.attachReaderStamps(mSet->readerStamps());
    if (GlobalSettings::instance()->settings().paramValueBool("debugDumpStamps", false) )
        qDebug() << mLIPs.dump();

    // general properties
    mConiferous = boolVar("isConiferous");
    mEvergreen = boolVar("isEvergreen");

    // setup allometries
    mFoliage_a = doubleVar("bmFoliage_a");
    mFoliage_b = doubleVar("bmFoliage_b");

    mStem_a = doubleVar("bmWoody_a");
    mStem_b = doubleVar("bmWoody_b");

    mRoot_a = doubleVar("bmRoot_a");
    mRoot_b = doubleVar("bmRoot_b");

    mBranch_a = doubleVar("bmBranch_a");
    mBranch_b = doubleVar("bmBranch_b");

    mSpecificLeafArea = doubleVar("specificLeafArea");
    mFinerootFoliageRatio = doubleVar("finerootFoliageRatio");

    mBarkThicknessFactor = doubleVar("barkThickness");

    // cn-ratios
    mCNFoliage = doubleVar("cnFoliage");
    mCNFineroot = doubleVar("cnFineRoot");
    mCNWood = doubleVar("cnWood");
    if (mCNFineroot*mCNFoliage*mCNWood == 0.) {
        throw IException( QString("Error setting up species %1: CN ratio is 0.").arg(id()));
    }


    // turnover rates
    mTurnoverLeaf = doubleVar("turnoverLeaf");
    mTurnoverRoot = doubleVar("turnoverRoot");

    // hd-relations
    mHDlow.setAndParse(stringVar("HDlow"));
    mHDhigh.setAndParse(stringVar("HDhigh"));
    mHDlow.linearize(0., 100.); // input: dbh (cm). above 100cm the formula will be directly executed
    mHDhigh.linearize(0., 100.);

    // form/density
    mWoodDensity = doubleVar("woodDensity");
    mFormFactor = doubleVar("formFactor");
    // volume = formfactor*pi/4 *d^2*h -> volume = volumefactor * d^2 * h
    mVolumeFactor = mFormFactor * M_PI_4;

    // snags
    mSnagKSW = doubleVar("snagKSW"); // decay rate of SWD
    mSnagHalflife = doubleVar("snagHalfLife");
    mSnagKYL = doubleVar("snagKYL"); // decay rate labile
    mSnagKYR = doubleVar("snagKYR"); // decay rate refractory matter

    if (mFoliage_a*mFoliage_b*mRoot_a*mRoot_b*mStem_a*mStem_b*mBranch_a*mBranch_b*mWoodDensity*mFormFactor*mSpecificLeafArea*mFinerootFoliageRatio == 0.) {
        throw IException( QString("Error setting up species %1: one value is NULL in database.").arg(id()));
    }
    // Aging
    mMaximumAge = doubleVar("maximumAge");
    mMaximumHeight = doubleVar("maximumHeight");
    mAging.setAndParse(stringVar("aging"));
    mAging.linearize(0.,1.); // input is harmonic mean of relative age and relative height
    if (mMaximumAge*mMaximumHeight==0.)
        throw IException( QString("Error setting up species %1:invalid aging parameters.").arg(id()));

    // mortality
    // the probabilites (mDeathProb_...) are the yearly prob. of death.
    // from a population a fraction of p_lucky remains after ageMax years. see wiki: base+mortality
    double p_lucky = doubleVar("probIntrinsic");
    double p_lucky_stress = doubleVar("probStress");

    if (p_lucky * mMaximumAge * p_lucky_stress == 0.) {
        throw IException( QString("Error setting up species %1: invalid mortality parameters.").arg(id()));
    }

    mDeathProb_intrinsic = 1. - pow(p_lucky, 1. / mMaximumAge);
    mDeathProb_stress = p_lucky_stress;

    if (logLevelInfo()) qDebug() << "species" << name() << "probStress" << p_lucky_stress << "resulting probability:" << mDeathProb_stress;

    // envirionmental responses
    mRespVpdExponent = doubleVar("respVpdExponent");
    mRespTempMin  =doubleVar("respTempMin");
    mRespTempMax  =doubleVar("respTempMax");
    if (mRespVpdExponent>=0) throw IException( QString("Error: vpd exponent >=0 for species (must be a negative value).").arg(id()));
    if (mRespTempMax==0. || mRespTempMin>=mRespTempMax) throw IException( QString("temperature response parameters invalid for species").arg(id()));

    mRespNitrogenClass = doubleVar("respNitrogenClass");
    if (mRespNitrogenClass<1 || mRespNitrogenClass>3) throw IException( QString("nitrogen class invalid (must be >=1 and <=3) for species").arg(id()));

    // phenology
    mPhenologyClass = intVar("phenologyClass");

    // water
    mMaxCanopyConductance = doubleVar("maxCanopyConductance");
    mPsiMin = -fabs(doubleVar("psiMin")); // force a negative value

    // light
    mLightResponseClass = doubleVar("lightResponseClass");
    if (mLightResponseClass<1. || mLightResponseClass>5.)
        throw IException( QString("invalid light response class for species %1. Allowed: 1..5.").arg(id()));

    // regeneration
    int seed_year_interval = intVar("seedYearInterval");
    if (seed_year_interval==0)
        throw IException(QString("seedYearInterval = 0 for %1").arg(id()));
    mSeedYearProbability = 1 / static_cast<double>(seed_year_interval);
    mMaturityYears = intVar("maturityYears");
    mTM_as1 = doubleVar("seedKernel_as1");
    mTM_as2 = doubleVar("seedKernel_as2");
    mTM_ks = doubleVar("seedKernel_ks0");
    mFecundity_m2 = doubleVar("fecundity_m2");
    mNonSeedYearFraction = doubleVar("nonSeedYearFraction");
    // special case for serotinous trees (US)
    mSerotiny.setExpression(stringVar("serotinyFormula"));
    mSerotinyFecundity = doubleVar("serotinyFecundity");

    // establishment parameters
    mEstablishmentParams.min_temp = doubleVar("estMinTemp");
    mEstablishmentParams.chill_requirement = intVar("estChillRequirement");
    mEstablishmentParams.GDD_min = intVar("estGDDMin");
    mEstablishmentParams.GDD_max = intVar("estGDDMax");
    mEstablishmentParams.GDD_baseTemperature = doubleVar("estGDDBaseTemp");
    mEstablishmentParams.bud_birst = intVar("estBudBirstGDD");
    mEstablishmentParams.frost_free = intVar("estFrostFreeDays");
    mEstablishmentParams.frost_tolerance = doubleVar("estFrostTolerance");
    mEstablishmentParams.psi_min = -fabs(doubleVar("estPsiMin")); // force negative value

    if (xml.valueBool("model.settings.permafrost.enabled"))
        mEstablishmentParams.SOL_thickness = fabs(doubleVar("estSOLthickness")); // force positive value

    // sapling and sapling growth parameters
    mSaplingGrowthParams.heightGrowthPotential.setAndParse(stringVar("sapHeightGrowthPotential"));
    mSaplingGrowthParams.heightGrowthPotential.linearize(0., cSapHeight);
    mSaplingGrowthParams.hdSapling = static_cast<float>( doubleVar("sapHDSapling") );
    mSaplingGrowthParams.stressThreshold = doubleVar("sapStressThreshold");
    mSaplingGrowthParams.maxStressYears = intVar("sapMaxStressYears");
    mSaplingGrowthParams.referenceRatio = doubleVar("sapReferenceRatio");
    mSaplingGrowthParams.ReinekesR = doubleVar("sapReinekesR");
    mSaplingGrowthParams.browsingProbability = doubleVar("browsingProbability");
    mSaplingGrowthParams.sproutGrowth = doubleVar("sapSproutGrowth");
    if (mSaplingGrowthParams.sproutGrowth>0.)
        if (mSaplingGrowthParams.sproutGrowth<1. || mSaplingGrowthParams.sproutGrowth>10)
            qWarning() << "Value of 'sapSproutGrowth' dubious for species" << name() << "(value: " << mSaplingGrowthParams.sproutGrowth << ", expected range: 1-10)";
    mSaplingGrowthParams.setupReinekeLookup();

    mSaplingGrowthParams.adultSproutProbability = 0.;
    QString adult_sprout = GlobalSettings::instance()->settings().value("model.species.sprouting.adultSproutProbability");
    if (!adult_sprout.isEmpty()) {
        QStringList sprout_prob_list = adult_sprout.split(QRegExp("([^\\.\\w]+)"));

        if (sprout_prob_list.length() == 1)
            mSaplingGrowthParams.adultSproutProbability = adult_sprout.toDouble();
        if (sprout_prob_list.length() > 1) {
            int index = sprout_prob_list.indexOf(id());
            if (index>=0 && index+1 < sprout_prob_list.length()) {
                mSaplingGrowthParams.adultSproutProbability = sprout_prob_list[index+1].toDouble();
                qDebug() << "enabled species specific sprouting probability for" << id() << ": p=" << mSaplingGrowthParams.adultSproutProbability;
            }
        }
    }
}


/** calculate fraction of stem wood increment base on dbh.
    allometric equation: a*d^b -> first derivation: a*b*d^(b-1)
    the ratio for stem is 1 minus the ratio of twigs to total woody increment at current "dbh". */
double Species::allometricFractionStem(const double dbh) const
{
    double inc_branch_per_d = (mBranch_a*mBranch_b*pow(dbh, mBranch_b-1.));
    double inc_woody_per_d = (mStem_a*mStem_b*pow(dbh, mStem_b-1));
    //double fraction_stem = 1. - inc_branch_per_d / inc_woody_per_d; // old
    double fraction_stem = inc_woody_per_d / (inc_branch_per_d + inc_woody_per_d);
    return fraction_stem;
}

/** Aging formula.
   calculates a relative "age" by combining a height- and an age-related term using a harmonic mean,
   and feeding this into the Landsberg and Waring formula.
   see https://iland-model.org/primary+production#respiration_and_aging
   @param useAge set to true if "real" tree age is available. If false, only the tree height is used.
  */
double Species::aging(const float height, const int age) const
{
    double rel_height = qMin(height/mMaximumHeight, 0.999999); // 0.999999 -> avoid div/0
    double rel_age = qMin(age/mMaximumAge, 0.999999);

    // harmonic mean: http://en.wikipedia.org/wiki/Harmonic_mean
    double x = 1. - 2. / (1./(1.-rel_height) + 1./(1.-rel_age)); // Note:

    double aging_factor = mAging.calculate(x);

    return limit(aging_factor, 0., 1.); // limit to [0..1]
}

int Species::estimateAge(const float height) const
{
    int age_rel = int( mMaximumAge * height / mMaximumHeight );
    return age_rel;
}

/** Seed production.
   This function produces seeds if the tree is older than a species-specific age ("maturity")
   If seeds are produced, this information is stored in a "SeedMap"
  */
void Species::seedProduction(const Tree *tree)
{
    if (!mSeedDispersal)
        return; // regeneration is disabled

    // if the tree is considered as serotinous (i.e. seeds need external trigger such as fire)
    if (isTreeSerotinous(tree->age()))
        return;

    // no seed production if maturity age is not yet reached (species parameter)
    if (tree->age() > mMaturityYears) {
        mSeedDispersal->setMatureTree(tree->positionIndex(), tree->leafArea());
    }
}


bool Species::isTreeSerotinous(const int age) const
{
    if (mSerotiny.isEmpty())
        return false;
    // the function result (e.g. from a logistic regression model, e.g. Schoennagel 2013) is interpreted as probability
    double p_serotinous = mSerotiny.calculate(age);
    if (drandom()<p_serotinous)
        return true;
    else
        return false;
}


/** newYear is called by the SpeciesSet at the beginning of a year before any growth occurs.
  This is used for various initializations, e.g. to clear seed dispersal maps
  */
void Species::newYear()
{
    if (seedDispersal()) {
        // decide whether current year is a seed year
        mIsSeedYear = (drandom() < mSeedYearProbability);
        if (mIsSeedYear && logLevelDebug())
            qDebug() << "species" << id() << "has a seed year.";
        // clear seed map
        seedDispersal()->newYear();
    }
}


void SaplingGrowthParameters::setupReinekeLookup()
{
    mRepresentedClasses.clear();
    for (int i=0;i<401;i++) {
        double h = i/100.;
        if (i==0) h=0.01; // avoid infinity
        double dbh = h / hdSapling  * 100.;
        // we use a lower limit of 0.25cm dbh, i.e. the reineke formula is truncated
        // as it is highly unplausible with very small dbh
        if (dbh < 0.25)
            dbh = 0.25;
        mRepresentedClasses.push_back(representedStemNumber(dbh));
    }
}
