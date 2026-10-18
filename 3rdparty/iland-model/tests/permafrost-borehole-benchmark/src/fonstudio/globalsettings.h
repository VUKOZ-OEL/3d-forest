#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H


const int cPxSize = 2; // size of light grid (m)
const int cRUSize = 100; // size of resource unit (m)
const double cRUArea = 10000.; // area of a resource unit (m2)
const int cHeightSize = 10; // size of a height grid pixel (m)
const int cPxPerHeight = 5; // 10 / 2 LIF pixels per height pixel
const int cPxPerRU = 50; // 100/2
const int cHeightPerRU = 10; // 100/10 height pixels per resource unit
const int cPxPerHectare = 2500; // pixel/ha ( 10000 / (2*2) )
const double cHeightPixelArea = 100.; // 100m2 area of a height pixel

// make sure file save dialogs work
#define ILAND_GUI

class GlobalSettings {
    GlobalSettings() {}
};

#endif // GLOBALS_H
