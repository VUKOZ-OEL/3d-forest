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

#include "global.h"
#include "stamp.h"
#include "grid.h"
#include "stampcontainer.h"
#include "helper.h"

const Grid<float> *Stamp::mDistanceGrid = 0;

Stamp::Stamp()
{
    m_data=0;
}

Stamp::~Stamp()
{
   if( m_data)
       delete[] m_data;
}

void Stamp::setup(const int size)
{
    int c=size*size;
    m_size=size;
    m_offset=0;
    m_reader = 0;
    m_crownArea=0.f;
    m_crownRadius=0.f;
    if (m_data)
        delete[] m_data;
    m_data=new float[c];
    for (int i=0;i<c;i++)
        m_data[i]=0.;
    // set static variable values
    mDistanceGrid = &StampContainer::distanceGrid();
}

//inline float Stamp::distanceToCenter(const int ix, const int iy) const
//{
//    //
//    return StampContainer::distanceGrid().constValueAtIndex(abs(ix-m_offset), abs(iy-m_offset));
//}

QString Stamp::dump() const
{
    QString result, line;
    int x,y;
    for (y=0;y<m_size;++y)  {
        line="";
        for (x=0;x<m_size;++x)  {
            line+= QString::number(*data(x,y)) + " ";
        }
        line+="\r\n";
        result+=line;
    }
    return result;
}

void Stamp::loadFromFile(const QString &fileName)
{
    QString txt = Helper::loadTextFile(fileName);
    QStringList lines = txt.split("\n");

    setup(lines.count());
    int l=0;
    foreach(QString line, lines) {
        QStringList cols=line.split(";");
        if (cols.count() != lines.count())
            MSGRETURN("Stamp::loadFromFile: invalid count of rows/cols.");
        for (int i=0;i<cols.count();i++)
            *data(i,l)=cols[i].toFloat();
        l++;
    }
}

// load from stream....
void Stamp::load(QDataStream &in)
{
   // see StampContainer doc for file stamp binary format
   qint32 offset;
   in >> offset;
   m_offset = offset;
   // load data
   float data;
   for (int i=0;i<count(); i++) {
       in >> data;
       m_data[i]=data;
   }
}

void Stamp::save(QDataStream &out)
{
    // see StampContainer doc for file stamp binary format
   out << static_cast<qint32>( m_offset );
   for (int i=0;i<count(); i++) {
       out << m_data[i];
   }
}


Stamp *stampFromGrid(const FloatGrid& grid, const int width)
{
    Stamp::StampType type=Stamp::est4x4;
    int c = grid.sizeX(); // total size of input grid
    if (c%2==0 || width%2==0) {
        qDebug() << "both grid and width should be uneven!!! returning NULL.";
        return NULL;
    }

    if (width<=4) type = Stamp::est4x4;
    else if (width<=8) type = Stamp::est8x8;
    else if (width<=12) type = Stamp::est12x12;
    else if (width<=16) type = Stamp::est16x16;
    else if (width<=24) type = Stamp::est24x24;
    else if (width<=32) type = Stamp::est32x32;
    else if (width<=48) type = Stamp::est48x48;
    else type = Stamp::est64x64;

    Stamp *stamp = new Stamp(int(type));
    int swidth = width;
    if (width>63) {
        qDebug() << "Warning: grid too big, truncated stamp to 63x63px!";
        swidth = 63;
    }
    stamp->setOffset(swidth/2);
    int coff = c/2 - swidth/2; // e.g.: grid=25, width=7 -> coff = 12 - 3 = 9
    int x,y;
    for (x=0;x<swidth; x++)
        for (y=0; y<swidth; y++)
            stamp->setData(x,y, grid(coff+x, coff+y) ); // copy data (from a different rectangle)
    return stamp;

}
