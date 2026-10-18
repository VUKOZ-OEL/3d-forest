
#include "solarradiation.h"

#include "hemigrid.h"


//#include "globals.h"
//#include "modell.h"
//#include "DBUtil.h"


//--------------------- SolarRad -------------
// ------ Mini-Modul zur Berechnung der Globalstrahlung
// eigentlich ähnlich wie Strahlungsmodell Picus 1.x,
// jedoch gleich selber machen ist einfacher....
void SolarRadiation::Setup(double BreiteGrad)
{
  Latitude=BreiteGrad * M_PI / 180.;
  for (int day=0; day<366; day++) {
     calcDay(day);
  }
}



//////////////////////////////////////////////////////
/// Calculate factor for optical mass (used for radiation calculation)
/// http://www.sortie-nd.org/help/manuals/developer_documentation/cplusplus/light.html
//////////////////////////////////////////////////////
double GetOpticalMass(const double elevation_rad)
{
   if (elevation_rad < 5*M_PI/180.)
      return 10.39;

   if (elevation_rad < 15*M_PI/180.)
      return 5.6;

   if (elevation_rad < 25*M_PI/180.)
      return 2.9;

   // if >= 25°:
   // cos(zenith) == cos(90° - elevation)....
   // here i am not completely sure: but sec(x)=1/cos(x)
   // seems to be right. sec(25°)=2.3, sec(20)=2.9 (see above!), sec(0)=1
   // the concrete steps above (5,15,25°) can be observed in cumulated outputs
   // e.g. global radiation over elevation - maybe a smoother variant should be used.

   return 1./cos(M_PI/2. - elevation_rad);
}
//////////////////////////////////////////////////////
// calc relative radiation matrix.
//////////////////////////////////////////////////////
void SolarRadiation::calculateRadMatrix(const float Step_deg, HemiGrid &Grid)
{

//      const double transmissionskoeff[12]={
//              0.4, 0.48, 0.47, 0.46, 0.47, 0.44,
//              0.48, 0.44, 0.39, 0.38, 0.32, 0.34};

      // elevation angle threshold
      const double min_elevation_angle = 0*M_PI/180.;
      double latidude = Latitude;

      Grid.setup(Step_deg);
      int cntAzimuth = Grid.matrixCountAzimuth();
      int cntElevation = Grid.matrixCountElevation();

      int day;
      double sin_declination, declination, eccentricity;
      double current_transmission;
      double sun_day, diff_hour;
      double hour, sun_hour;
      double step = 1./60.; // 1 minute
      bool sun_rise, sun_down;
      double total_rad_sum = 0;
      double elevation, sin_elevation, cos_azimuth, azimuth;
      double Iincoming;
      for (day = 0; day < 366; day++) {
        // skip days due to vegetation period limit:
        if (day<mMinDay || day>mMaxDay)
           continue;
        sin_declination=0.39785*sin((278.97+0.9856*day+1.9165*sin((356.6+0.9856*day)*M_PI/180.))*M_PI/180.);
        declination=asin(sin_declination); // declination of earth (rad)
        // solarconstant: Gassel p. 15
        SolarConstant=1356.5+48.5*cos(0.01721*(day-15));
        sun_day = (day-1)*2.*M_PI/365.; // rad
        // difference between "real sunhour" and hour
        diff_hour = 12+0.12357*sin(sun_day)-0.004289*cos(sun_day)+0.153809*sin(2.*sun_day)+0.060783*cos(2.*sun_day);
        // eccentricity: SORTIE (http://www.sortie-nd.org/help/manuals/developer_documentation/cplusplus/light.html)
        eccentricity = 1.000110 + 0.034221* cos(sun_day) + 0.001280*sin(sun_day) + 0.000719 * cos(2*sun_day) + 0.000077* sin(2*sun_day);
        hour = 0;
        sun_rise = sun_down = false;
        //current_transmission = transmissionskoeff[(day/30)%12];
        current_transmission = 0.75;
        while (!sun_down) {
          sun_hour=(hour-diff_hour)*15./180*M_PI; // s in rad
          // sin elevation: =sin(lat)*sin(decl) + cos(lat)*cos(sunhour)*cos(decl)
          sin_elevation=sin(latidude)*sin_declination + cos(latidude)*cos(sun_hour)*cos(declination);
          elevation =asin(sin_elevation); // Sonnenhöhe [rad]
          // check for state of the sun
          if (!sun_rise && elevation>min_elevation_angle)
             sun_rise=true;
          if (sun_rise && elevation<min_elevation_angle)
             sun_down=true;
          if (elevation>=min_elevation_angle) {
              // calculate azimuth
              cos_azimuth = ( sin_elevation * sin(latidude) - sin_declination) / ( cos(elevation)*cos(latidude) );
              azimuth = acos(cos_azimuth);
              // azimuth is always > 0: use sign of sun_hour: if negative, than direction is "east" (before noon)
              if (sun_hour < 0.)
                 azimuth*=-1; 
              // calculate the energy (reduction due to transmissioncoeffiecients..)
              // I = eccentricity * trans^optical_mass * cos(zeninthangle!)
              Iincoming = eccentricity * pow(current_transmission, GetOpticalMass(elevation)) * cos(M_PI/2. - elevation);
              total_rad_sum += Iincoming;

              // azimuth is 0 for "south". the grid is north-oriented -> reverse (azimuth(North)=0)
              if (azimuth <= 0.)
                 azimuth += M_PI;
              else
                 azimuth -= M_PI;

              // store in grid:
              Grid.rGet(azimuth, elevation) += Iincoming;

          } // elevation>min_elevation_angle
          hour += step;
          if (hour>24)
             throw QString("tsolarrad::calcmatrix: elevation never above minimum elevation.");
        } // while
      }  // for (each day)

      // normalize the array and add the diffus radiation...
      // direct: sum(all rad) should be fraction of direct radiation
      double direct_rad_multiplier = (1.-mDiffusRadFraction) / ( total_rad_sum );
      // diffuse: the light is evenly distributed (above the minimum angle)
      // add a little to lower bound to avoid rounding down
      int min_elev_index = int( (min_elevation_angle+0.0001) / (M_PI / 2.) * cntElevation);
      double diffuse_rad_addon = mDiffusRadFraction / ((cntElevation - min_elev_index) * cntAzimuth);

      for (int i=min_elev_index; i<cntElevation; i++) {
         for (int j=0;j<cntAzimuth;j++) {
            double &cell_value = Grid.rGetByIndex(j, i);
            cell_value*=direct_rad_multiplier;
            cell_value+=diffuse_rad_addon;
         }
      }

}


void SolarRadiation::calcDay(int day)
{
      // Berechnungen pro Tag:
      // Deklination:
      //sin D=0,39785*SIN((278,97+0,9856*G8+1,9165*SIN((356,6+0,9856*G8)*M_PI()/180))*M_PI()/180)
      double sinDekl=0.39785*sin((278.97+0.9856*day+1.9165*sin((356.6+0.9856*day)*M_PI/180.))*M_PI/180.);
      double Dekl=asin(sinDekl); // Deklination in Grad
      //S = 1356,5+48,5*cos[0,01721*(n-15)] Gassel p. 15
      
      SolarConstant=1356.5+48.5*cos(0.01721*(day-15));
      // Sonnentag
      double SunDay;
      SunDay=(day-1)*2.*M_PI/365.; // [rad]
      // Unterschied wahre Sonnenstunde - Stunde
      double Diff=12+0.12357*sin(SunDay)-0.004289*cos(SunDay)+0.153809*sin(2.*SunDay)+0.060783*cos(2.*SunDay);

      // Integration über den Tag
      double Sum=0., Length=0., Rad;
      // Sonnenstunde
      double Hour, SunHour, sinH, Altitude;
      bool   SunRise=false;
      bool   SunDown=false;
      Hour=0.;
      const double Step=1/60.; // Minutentakt
      // berechnung der tageslänge methode 2:
      // tag: wenn strahlung > threshold
      double RadThreshold = -1; // not used: Picus specific: =GModell->Settings["BBDayLengthMode"].AsDouble();
      const double transmissionskoeff[12]={
              0.4, 0.48, 0.47, 0.46, 0.47, 0.44,
              0.48, 0.44, 0.39, 0.38, 0.32, 0.34};
      while (!SunDown)
      {
          SunHour=(Hour-Diff)*15./180*M_PI; // s in rad
          // sin Sonnenhöhe: =sin(Breite)*sin(Dekl) + cos(Breite)*cos(Sonnenstunde)*cos(Deklination)
          sinH=sin(Latitude)*sinDekl + cos(Latitude)*cos(SunHour)*cos(Dekl);
          Altitude=asin(sinH); // Sonnenhöhe [rad]
          // calculate azimuth-angle
          if (!SunRise && Altitude>0)
             SunRise=true;
          if (SunRise && Altitude<0)
             SunDown=true;
          if (Altitude>0) {
              // Length: Tageslänge [Stunden]
              if (RadThreshold==-1)
                 Length+=Step;   // default
              else if (SolarConstant*sinH*transmissionskoeff[(day/30)%12] > RadThreshold) {
                 // SolarConst[W/m2]*frac*frac=[W/m2]
                 Length+=Step; // strahlungs-grenze-fall
              }
              // Rad: W * h * frac: Wh
              Rad=SolarConstant * Step * sinH;
              Sum+=Rad;
          }
          Hour+=Step;
      }
      RadExtraTerrestrisch[day]=Sum*3.6; // Wh-> kJ (*3600/1000)
      DayLength[day]=Length*3600;    // sekunden
}

