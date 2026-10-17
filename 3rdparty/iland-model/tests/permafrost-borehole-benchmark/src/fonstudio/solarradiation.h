#ifndef SolarRadiation_H
#define SolarRadiation_H
//---------------------------------------------------------------------------

class HemiGrid;
/** Handles sun and radiation calculations.
  This class contains functions to calculate daylength, and hemipherical radiation intensities
  based on latitude. */
class SolarRadiation
{
public:
        SolarRadiation():  mMinDay(-1), mMaxDay(367), mDiffusRadFraction(0.5) {}
        void Setup(double BreiteGrad);

        /** calculate radiation matrix.
          calculates for each sector of the "Grid" the yearly radiation intensites.
          Intensities are influenced by latitude, vegetation period and the fraction of diffuse radiation.
          @param Step_deg size of one cell in degree (e.g. 5 -> each pixel has a size of 5°x5°)
          @param Grid results are stored in that HemiGrid (no setup required)    */
        void calculateRadMatrix(const float Step_deg, HemiGrid &Grid);
        /// set fraction of diffuse radiation (1: only diffuse rad, 0: only direct rad)
        void setDiffusRadFraction(double fraction_of_diffus_rad) { mDiffusRadFraction = fraction_of_diffus_rad; }
        /// set latitude in degree
        void setLatidude(const double lat_degree) { Latitude = lat_degree * M_PI/180.; }
        void setVegetationPeriod(int day_start, int day_end) { mMinDay=day_start; mMaxDay=day_end; }
private:
        int  FTime; ///< time of day
        double DayLength[366]; ///< length of day (seconds)
        double RadExtraTerrestrisch[366]; ///< total radiation Extraterrestrisch kJ/m2 daysum
        double SolarConstant; ///< radiation constant in space
        double Latitude;    ///< Latitude in rad
        int mMinDay; ///< start of vegetation period (day)
        int mMaxDay; ///< end of vegetation period (day)
        double mDiffusRadFraction;
        void calcDay(int day);
        ///< calculate daylength and daily radiation values
};

#endif
