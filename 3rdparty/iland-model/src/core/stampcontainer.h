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

#ifndef STAMPCONTAINER_H
#define STAMPCONTAINER_H

#include "stamp.h"
#include "grid.h"

/** Collection of Stamp for one tree species.
  @ingroup core
  Per species several stamps are stored (different BHD, different HD relations). This class
  encapsulates storage and access to these stamps. The design goal is to deliver high
  access speeds for the "stamp()" method.
  Use getStamp(bhd, hd) or getStamp(bhd, height) to access. */
class StampContainer
{
public:
    StampContainer();
    ~StampContainer();
    void useLookup(const bool use) { m_useLookup = use; }
    /// addStamp() add a pre-allocated stamp @param stamp to internal collection. Caller must allocate stamp on the heap,
    /// freeing is done by this class.
    void addStamp(Stamp* stamp, const float dbh, const float hd_value, const float crown_radius);
    void addReaderStamp(Stamp *stamp, const float crown_radius_m);
    const Stamp* stamp(const float bhd_cm, const float height_m) const;
    const Stamp* readerStamp(const float crown_radius_m) const; ///< retrieve reader-stamp. @param radius of crown in m. @return the appropriate stamp or NULL if not found.
    int count() const { return m_stamps.count(); }
    /// save the content of the StampContainer to the output stream (binary encoding)
    void save(QDataStream &out);
    /// load the content of the StampContainer to the output stream (binary encoding)
    void load(QDataStream &in);
    void load(const QString &fileName);

    /** this functions attaches the appropriate reader (dep. on crown radius) to each stamp of the container.
        The reader-stamp is returned by a call to the reader()-function of the Stamp itself.
        @param Container holding the reader stamps.*/
    void attachReaderStamps(const StampContainer &source);
    /// static function to retrieve distance grid. See Stamp::distanceToCenter
    static const Grid<float> &distanceGrid()  { return m_distance; }
    void invert(); ///< invert stamps (value = 1. - value) (for multiplicative overlay)
    // description
    const QString &description() { return m_desc; }
    void setDescription(const QString s) { m_desc = s; }
    QString dump();

private:
    void finalizeSetup(); ///< complete lookup-grid by filling up zero values
    void setupDistanceGrid(const int size); ///< setup the grid holding precalculated distance values

    static const int cBHDclassWidth;
    static const int cHDclassWidth;
    static const int cBHDclassLow; ///< bhd classes start with 4: class 0 = 4..8, class1 = 8..12
    static const int cHDclassLow; ///< hd classes offset is 40: class 0 = 40-50, class 1 = 50-60
    static const int cBHDclassCount; ///< class count, 50: highest class = 50*4 +- 2 = 198 - 202
    static const int cHDclassCount; ///< class count. highest class: 140-150
    struct StampItem {
        Stamp* stamp;
        float dbh;
        float hd;
        float crown_radius;
    };
    inline void getKey(const float dbh, const float hd_value, int &dbh_class, int &hd_class) const;
    void addStamp(Stamp* stamp, const int cls_dbh, const int cls_hd, const float crown_radius_m, const float dbh, const float hd_value);
    int m_maxBhd;
    bool m_useLookup; // use lookup table?
    QList<StampItem> m_stamps;
    Grid<Stamp*> m_lookup;
    static Grid<float> m_distance; ///< grid holding precalculated distances to the stamp center
    QString m_desc;
    QString m_fileName;
};

#endif // STAMPCONTAINER_H
