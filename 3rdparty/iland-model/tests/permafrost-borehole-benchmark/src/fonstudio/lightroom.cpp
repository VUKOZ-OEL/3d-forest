#include "lightroom.h"
#include "../tools/helper.h"
#include "debugtimer.h"

#include <QtCore>

LightRoom::LightRoom()
{
    m_roomObject = 0;
    m_aggregationMode = 0;
}

/** setup routine.
  sets up datastructures (3d space, hemi grids)
  @param dimx size of space in x direction [m]
  @param dimy size of space in y direction [m]
  @param dimz size of space in z direction [m]
  @param cellsize metric length of cells (used for all 3 dimensions).
  @param hemigridsize size of hemigrid-cells (in degrees).
  @param latitude lat. in degrees.
  @param diffus_frac fraction [0..1] of diffus radiation of global radiation. */
void LightRoom::setup(const double dimx, const double dimy, const double dimz,
                      const double cellsize, const double hemigridsize,
                      const double latitude, const double diffus_frac)
{
    DebugTimer t1("setup of lightroom");
    m_countX = int(dimx / cellsize);
    m_countY = int(dimy / cellsize);
    m_countZ = int(dimz / cellsize);

    m_cellsize = cellsize;
    if (m_countX%2==0) m_countX++; // make uneven
    if (m_countY%2==0) m_countY++;

    QRectF rect(-m_countX/2.*cellsize, -m_countY/2.*cellsize, m_countX*cellsize, m_countY*cellsize);
    qDebug() << "Lightroom size: " << m_countX << "/" << m_countY << "/" << m_countZ << " elements. rect: " << rect;


    m_2dvalues.setup(rect, cellsize);
    m_2dvalues.initialize(0.);

    // setup hemigrids
    SolarRadiation solar;
    solar.setLatidude(latitude); // austria
    solar.setVegetationPeriod(0,367); // no. veg. period
    solar.setDiffusRadFraction(diffus_frac); // 50% diffuse radiation
    // calculate global radiation values
    DebugTimer t2("calculate solar radiation matrix");
    solar.calculateRadMatrix(hemigridsize, m_solarGrid);
    t2.showElapsed();
    m_shadowGrid.setup(hemigridsize); // setup size
    m_solarrad_factor = 1. / m_solarGrid.sum(RAD(45)); // sum of rad. > 45°
    m_centervalue = 0.;
}

double LightRoom::calculateGridAtPoint(const double p_x, const double p_y, const double p_z, bool fillShadowGrid)
{
    if (!m_roomObject)
        return 0;
    // check feasibility
    if (m_roomObject->noHitGuaranteed(p_x, p_y, p_z)) {
        //qDebug()<<"skipped";
        return 0;
    }

    if (fillShadowGrid)
        m_shadowGrid.clear(0.);

    // start with 45°
    int ie = m_shadowGrid.indexElevation(RAD(45));
    int ia = 0;
    int max_a = m_shadowGrid.matrixCountAzimuth();
    int max_e = m_shadowGrid.matrixCountElevation();
    double elevation, azimuth;
    double solar_sum=0;
    int hit;
    int c_hit = 0;
    int c_test = 0;
    for (;ie<max_e;ie++){
        for (ia=0;ia<max_a;ia++) {
            azimuth = m_shadowGrid.azimuthNorth(ia);
            elevation = m_shadowGrid.elevation(ie);
            hit = m_roomObject->hittest(p_x, p_y, p_z,azimuth,elevation);
            // if inside the crown: do nothing and return.
            // 20090708: if inside the crown: return "totally dark"
            if (hit==-1)
                return 1;
            //qDebug() << "testing azimuth" << GRAD(azimuth) << "elevation" << GRAD(elevation)<<"hit"<<hit;
            c_test++;
            if (hit==1) {
                // retrieve value from solar grid
                // Sum(cells) of solargrid =1 -> the sum of all "shadowed" pixels therefore is already the "ratio" of shaded/total radiation
                solar_sum += m_solarGrid.rGetByIndex(ia, ie);
                if (fillShadowGrid)
                  m_shadowGrid.rGetByIndex(ia, ie) = m_solarGrid.rGetByIndex(ia, ie);
                c_hit++;
            }
        }
    }
    // solar-rad-factor = 1/(sum rad > 45°)
    return solar_sum * m_solarrad_factor;
    //double ratio = c_hit / double(c_test);
    //qDebug() << "tested"<< c_test<<"hit count:" << c_hit<<"ratio"<<c_hit/double(c_test)<<"total sum"<<m_shadowGrid.getSum();
    //return ratio; // TODO: the global radiation is not calculated!!!!!

}

void LightRoom::calculateFullGrid()
{
    float *v = m_2dvalues.begin();
    float *vend = m_2dvalues.end();
    int z;
    QPoint pindex;
    QPointF coord;
    double hit_ratio;
    DebugTimer t("calculate full grid");
    int c=0;
    float maxh = m_roomObject->maxHeight();

    float *values = new float[m_countZ];

    //double h_realized;
    double sum;

    while (v!=vend) {
        pindex = m_2dvalues.indexOf(v);
        coord = m_2dvalues.cellCenterPoint(pindex);
        double hor_distance = sqrt(coord.x()*coord.x() + coord.y()*coord.y());

        for (z=0;z<m_countZ && z*m_cellsize <= maxh;z++) {
            // only calculate values up to the 45° line
            // tan(45°)=1 -> so this is the case when the distance p->tree > height of the tree
            if (hor_distance > maxh-z*m_cellsize)
                break;
            hit_ratio = calculateGridAtPoint(coord.x(), coord.y(), // coords x,y
                                             z*m_cellsize,false); // heigth (z), false: do not clear and fill shadow grid structure
            // stop calculating when return = -1 (e.g. when inside the crown)
            if (hit_ratio==-1)
                break;
            values[z]=hit_ratio; // this value * height of cells
        }
        // calculate average
        sum = 0;
        // 20090708: do not average, but keep the sum
        // aggregate mean for all cells with angles>45° to the tree-top!
        // 20090711: again average it, but now include the tree top.
        // 20090713: go back to sum...
        // 20090812: remove the weighting for the 10m cells again.
        // 20100520: again, use the average value (per meter)
        for(int i=0;i<z;i++)
            sum+=values[i];
        if (m_aggregationMode==0) {
            // average shadow / meter height (within the 45° cone)
            if (z)
                sum/=float(z);
        } else {
            // aggregation: sum
            sum *= m_cellsize;  // multiply with height (shadow * meter)
        }
        *v = sum; // store in matrix
        // sum

        v++;
        c++;
        if (c%1000==0) {
            qDebug() << c << "processed...time: ms: " << t.elapsed();
            QCoreApplication::processEvents();
        }
        // save value of the middle column...
        m_centervalue = m_2dvalues(0.f, 0.f);
    } // while(v!=vend)
}

//////////////////////////////////////////////////////////
// Lightroom Object
//////////////////////////////////////////////////////////

LightRoomObject::~LightRoomObject()
{
    if (m_radiusFormula)
        delete m_radiusFormula;
}

void LightRoomObject::setuptree(const double height, const double crownheight, const QString &formula)
{
    if (m_radiusFormula)
        delete m_radiusFormula;

    m_radiusFormula = new Expression(formula);

    m_baseradius = m_radiusFormula->calculate(crownheight/height);
    m_height = height;
    m_crownheight = crownheight;
    double h=0., r;
    m_treeHeights.clear();
    // preprocess crown radii for each meter step
    while (h<=m_height) {
        if (h<m_crownheight)
            r=0.;
        else
            r = m_radiusFormula->calculate(h/m_height);
        m_treeHeights.push_back(r);
        h++;
    }
}


// Angle-function. return the  difference between
// two angles as a radian value between -pi..pi [-180..180°]
// >0: directionB is "left" (ccw) of directionA, <0: "right", clockwise
// e.g.: result=-10: 10° cw, result=10°: 10° ccw
// result:-180/+180: antiparallel.
double DiffOfAngles(double DirectionA, double DirectionB)
{
    DirectionA = fmod(DirectionA, PI2); // -> -2pi .. 2pi
    if (DirectionA < 0) DirectionA+=PI2; // -> 0..2pi (e.g.: AngleA = -30 -> 330)
    DirectionB = fmod(DirectionB, PI2);
    if (DirectionB < 0) DirectionB+=PI2;
    double Delta = DirectionB - DirectionA;
    if (Delta<0) {
      if (Delta<-M_PI)
         return Delta  + PI2; // ccw
      else
         return Delta; // cw
    } else {
      if (Delta>M_PI)
         return Delta - PI2; // cw
      else
         return Delta;  // ccw
    }

}


// Angle-function. return the absolute difference between
// two angles as a radian value between 0..pi [0..180°]
// e.g. 10° = +10° or -10°; maximum value is 180°
double AbsDiffOfAngles(double AngleA, double AngleB)
{
     return fabs(DiffOfAngles(AngleA, AngleB));
}


/** The tree is located in x/y=0/0.
*/
int LightRoomObject::hittest(const double p_x, const double p_y, const double p_z,
                              const double azimuth_rad, const double elevation_rad)
{
    bool inside_crown=false;
    if (p_z > m_height)
        return 0;
    // Test 1: does the ray (azimuth) direction hit the crown?
    double phi = atan2(-p_y, -p_x); // angle between P and the tree center
    double dist2d = sqrt(p_x*p_x + p_y*p_y); // distance at ground
    //if (dist2d==0)
    //    return true;

    double alpha = DiffOfAngles(phi, azimuth_rad); //  phi - azimuth_rad; // angle between the ray and the center of the tree
    if (dist2d>m_baseradius) { // test only, if p not the crown
        double half_max_angle = asin(m_baseradius / dist2d); // maximum deviation angle from direct connection p - tree where ray hits maxradius
        if (fabs(alpha) > half_max_angle)
            return 0;
    } else {
        inside_crown = true;
        // test if p is inside the crown
        if (p_z<=m_height && p_z>=m_crownheight) {
            double radius_hit = m_radiusFormula->calculate(p_z / m_height);
            if (dist2d <= radius_hit)
                return -1;
        }
    }

    // Test 2: test if the crown-"plate" at the bottom of the crown is hit.
    if (elevation_rad>0. && p_z<m_crownheight) {
        // calc. base distance between p and the point where the height of the ray reaches the bottom of the crown:
        double r_hitbottom = dist2d; // for 90°
        if (elevation_rad < M_PI_2) {
            double d_hitbottom = (m_crownheight - p_z) / tan(elevation_rad);
            // calc. position (projected) of that hit point
            double rx = p_x + cos(azimuth_rad)*d_hitbottom;
            double ry = p_y + sin(azimuth_rad)*d_hitbottom;
            r_hitbottom = sqrt(rx*rx + ry*ry);
        }
        if (r_hitbottom <= m_baseradius)
            return 1;
    }
    // Test 3: test for height steps...
    // distance from p to the plane normal to p-vector through the center of the tree
    // do only when p is
    double rx,ry,rhit;
    double d_center = cos(alpha)*dist2d;
    if (d_center>=0) {
        double h_center = p_z + d_center*tan(elevation_rad);
        if (h_center<=m_height && h_center>=m_crownheight) {
            rx = p_x + cos(azimuth_rad)*d_center;
            ry = p_y + sin(azimuth_rad)*d_center;
            rhit = sqrt(rx*rx + ry*ry);
            double r_h = m_radiusFormula->calculate(h_center / m_height);
            if (rhit < r_h)
                return 1;
        }
    }

    // Test 4: "walk" through crown using 1m steps.
    double h=floor(p_z);
    double d_1m = 1 / tan(elevation_rad); //projected ground distance equivalent of 1m height difference
    double d_cur = 0;
    if (h!=p_z) {
       d_cur += ((h+1)-p_z)*d_1m;
    }

    while (h<=m_height) {
        double r_tree = m_treeHeights[int(h)];
        rx = p_x + cos(azimuth_rad)*d_cur;
        ry = p_y + sin(azimuth_rad)*d_cur;
        rhit = rx*rx + ry*ry;
        // hit if inside of radius
        if (rhit < r_tree*r_tree)
            return 1;
        // no hit: if formerly was inside crown and now left
        if (inside_crown && rhit > m_baseradius*m_baseradius)
            return 0;

        // enter crown
        if (!inside_crown && rhit <=  m_baseradius*m_baseradius)
            inside_crown = true;
        d_cur+=d_1m;
        h++;
    }
    return 0;


}

bool LightRoomObject::noHitGuaranteed(const double p_x, const double p_y, const double p_z)
{
    // 1. simple: compare height...
    if (p_z > m_height)
        return true;
    // 2. 45° test:
    if (p_z > m_height - sqrt(p_x*p_x + p_y*p_y)) // 45°: height = distance from tree center
        return true;

    return false;
}

