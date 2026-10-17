#ifndef GRIDITERATOR_H
#define GRIDITERATOR_H

#include "grid.h"

/** GridIterator is a iterator used to navigate over grids with different size.
  Usage:
      GridIterator<float> gi(HeightGrid); // create the iterator and link to a grid of type float
      gi.setRect(QRect(10., 10., 1120., 120.));
      while (float *p=gi.next()) {
        // do something with *p
      }
      // usage (2)
      gi.targetSize(targetGrid.size());
      gi.setTargetIndex(targetGrid.indexOf(coord));
      while (float *p=gi.next())
         *p++;
      // usage(3)
      gi.neighboursOf(index); // 8-neighbourhood
      gi.neighboursOf(index, 2); // 2 rings, ..24 neighbours
      while (float *p=gi.next())
        *p=0.;
  */
template <class T>
class GridIterator {
public:
    GridIterator();
    GridIterator(const Grid<T> &targetGrid);
private:

}
#endif // GRIDITERATOR_H
