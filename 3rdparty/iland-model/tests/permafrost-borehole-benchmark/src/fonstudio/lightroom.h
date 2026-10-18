#ifndef LIGHTROOM_H
#define LIGHTROOM_H

#include "grid.h"
#include "solarradiation.h"
#include "hemigrid.h"
#include "expression.h"

class LightRoomObject
{
public:
    LightRoomObject(): m_radiusFormula(0) {}
    ~LightRoomObject();
    /** Test if the ray starting at "p" hits the object.
        the ray has direction azimuth/elevation and starts from the position denoted by p_x, p_y and p_z
        @return 1: object is hit, 0: object is missed, -1: p is inside the object.*/
    int hittest(const double p_x, const double p_y, const double p_z,
                 const double azimuth_rad, const double elevation_rad);
    /** sets up a tree as the obstacle.
        @param height treehight in meter
        @param crownheight height of the start of crown (above ground) in meter
        @param formula (as string representation) that yields the radius as f(relativeheight).
              the variable of the formula is 0 for ground 1 for tree top. */
    void setuptree(const double height, const double crownheight, const QString &formula);
    /// returns true if there is no way that a ray hits the object starting from p.
    bool noHitGuaranteed(const double p_x, const double p_y, const double p_z);
    double maxHeight() const { return m_height; }
    double maxRadius() const { return m_baseradius; }
private:
    Expression *m_radiusFormula; ///< formula for calculation of crown widht as f(relative_height)
    double m_baseradius; ///< maximum radius of the crown (at the bottom of the crown) [m]
    double m_height; ///< treehight [m]
    double m_crownheight; ///< height of the beginning of the living crown [m]
    QVector<double> m_treeHeights;
};

/** virtual room to do some light-experiments.
  The basic use of this class is to derive the size/pattern of the light-influence FON for a single tree.
  It uses SolarRadiation for the calculation of global radiation and HemiGrid to calculate and store the results.
  This calculation is done for each node of a 3D space and afterwards accumulated into a 2D pattern. */
class LightRoom
{
public:
    LightRoom();
    ~LightRoom() { if (m_roomObject) delete m_roomObject; }
    /// setup the spatial grid.
    void setup(const double dimx, const double dimy, const double dimz,
               const double cellsize, const double hemigridsize,
               const double latitude=48., const double diffus_frac=0.5);
    void setLightRoomObject(LightRoomObject *lro) { if (m_roomObject) delete m_roomObject; m_roomObject = lro; }
    void setAggregationMode(const int mode ) { m_aggregationMode = mode; }
    /// calculate a full hemiview image from given point
    double calculateGridAtPoint(const double p_x, const double p_y, const double p_z, bool fillShadowGrid=true);
    /// calculate a hemigrid for each node of the grid (store results in m_3dvalues).
    /// @return fraction of "blocked" radiation [0..1]. -1: point inside the object
    void calculateFullGrid();
        /// access to the hemigrid
    const HemiGrid &shadowGrid() const { return m_shadowGrid; }
    const HemiGrid &solarGrid() const { return m_solarGrid; }
    const FloatGrid &result() const { return m_2dvalues; }
    double centerValue() const { return m_centervalue; }

private:
    //Grid< QVector<float> > m_3dvalues; ///< storage for resulting 3d light values
    FloatGrid m_2dvalues; ///< resulting 2d pattern (derived from 3dvalues)
    int m_countX; ///< size of the grid in x-direction
    int m_countY;
    int m_countZ;
    double m_cellsize; ///< length of the side of one cell (equal for all 3 directions)
    double m_solarrad_factor; ///< multiplier accounting for the difference of total radiation and the used part of the sky (45°)
    HemiGrid m_solarGrid; ///< grid used for solar radiation (direct + diffus)
    HemiGrid m_shadowGrid; ///< grid used for shadow calculations
   double m_centervalue; ///< value (shadow*height) of the tree center (relative) [-]
   int m_aggregationMode; ///< aggregation mod

    LightRoomObject *m_roomObject;
};

#endif // LIGHTROOM_H
