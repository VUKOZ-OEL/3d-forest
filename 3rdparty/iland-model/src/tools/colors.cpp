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
#include "colors.h"
#include "dem.h"

QVector<QColor> Colors::mBrewerDiv = QVector<QColor>() << QColor("#543005") << QColor("#8c510a") << QColor("#bf812d") << QColor("#dfc27d")
                                                       << QColor("#f6e8c3") << QColor("#f5f5f5") << QColor("#fdbf6f") << QColor("##c7eae5")
                                                       << QColor("#80cdc1") << QColor("#35978f") << QColor("#01665e") <<  QColor("#003c30");

QVector<QColor> Colors::mBrewerQual = QVector<QColor>() << QColor("#a6cee3") << QColor("#1f78b4") << QColor("#b2df8a") << QColor("#33a02c")
                                                       << QColor("#fb9a99") << QColor("#e31a1c") << QColor("#fdbf6f") << QColor("#ff7f00")
                                                       << QColor("#cab2d6") << QColor("#6a3d9a") << QColor("#ffff99") <<  QColor("#b15928");


QVector<QColor> Colors::mTerrainCol = QVector<QColor>() << QColor("#00A600") << QColor("#24B300") << QColor("#4CBF00") << QColor("#7ACC00")
                                                       << QColor("#ADD900") << QColor("#E6E600") << QColor("#E8C727") << QColor("#EAB64E")
                                                       << QColor("#ECB176") << QColor("#EEB99F") << QColor("#F0CFC8") <<  QColor("#F2F2F2");

QVector<QColor> Colors::mCustomColors = QVector<QColor>();
QVector<QColor> Colors::mTurboColors = QVector<QColor>();


void Colors::setPalette(const GridViewType type, const float min_val, const float max_val)
{
    if (mNeedsPaletteUpdate==false && type==mCurrentType &&
            (mAutoScale==false || (minValue()==min_val && maxValue()==max_val))  )
        return;

    mHasFactors = false;
    int n = 50;
    if (type >= GridViewBrewerDiv) {
        // categorical values...
        mHasFactors = true;
        n=mFactorLabels.size();
        if (mFactorLabels.isEmpty()) {
            n=max_val;
            mFactorLabels.clear();
            for (int i=0;i<n;++i)
                mFactorLabels.append(QString("Label %1").arg(i));
        }
    }
    if (type != GridViewCustom) {
        mColors.clear();
        for (int i=0;i<n;++i)
            if (mHasFactors)
                mColors.append(colorFromValue(i, type, 0., 1.).name());
            else
                mColors.append(colorFromValue(1. - i/double(n), type, 0., 1.).name());

    }
    mLabels = QStringList() << QString::number(min_val)
                            << QString::number((3.*min_val + max_val)/4.)
                            << QString::number((min_val+max_val)/2.)
                            << QString::number((min_val + 3.*max_val)/4.)
                            << QString::number(max_val);

    if (mAutoScale) {
        mMinValue = min_val;
        mMaxValue = max_val;
    }
    mCurrentType = type;
    mNeedsPaletteUpdate = false;
    emit colorsChanged();
}

void Colors::setFactorLabels(QStringList labels)
{
    mFactorLabels = labels;
    mNeedsPaletteUpdate = true;
}

void Colors::setFactorColors(QStringList colors)
{
    mColors = colors;
    // fill also the custom colors
    mCustomColors.clear();
    for (int i=0;i<mColors.size();++i)
        mCustomColors.append(QColor(mColors[i]));
}

Colors::Colors(QWidget *parent): QObject(parent)
{
    mNeedsPaletteUpdate =true;
    mAutoScale = true;
    mHasFactors = false;
    mMeterPerPixel = 1.;
    setupFixedPalettes();
    //default start palette
    //setPalette(GridViewRainbow, 0, 1);
    // factors test
    setCaption("");
    setPalette(GridViewTerrain, 0, 4);
}

QColor Colors::colorFromPalette(const int value, const GridViewType view_type)
{
    if (value<0)
        return Qt::white;
    int n = qMax(value,0) % 12;
    QColor col;
    switch(view_type) {
    case GridViewBrewerDiv: col = mBrewerDiv[n]; break;
    case GridViewBrewerQual: col = mBrewerQual[n]; break;
    case GridViewTerrain: col = mTerrainCol[n]; break;
    case GridViewCustom: col = n<mCustomColors.size() ?  mCustomColors[n] : QColor(); break;
    default: return QColor();
    }
    if (value < 12)
        return col;
    n = qMax(value,0) % 60;
    if (n<12) return col;
    if (n<24) return col.darker(200);
    if (n<36) return col.lighter(150);
    if (n<48) return col.darker(300);
    return col.lighter(200);

}

QColor Colors::shadeColor(const QColor col, const QPointF &coordinates, const DEM *dem)
{
    if (dem) {
        float val = dem->viewGrid()->constValueAt(coordinates); // scales from 0..1
        if (val == 0.f) // area for which no DEM is available
            return col;
        float h, s, v;
        col.getHsvF(&h, &s, &v);
        // we adjust the 'v', the lightness: if val=0.5 -> nothing changes
        v=limit( v - (1.-val)*0.4, 0.1, 1.);
        QColor c;
        c.setHsvF(h,s,v);
        return c;

    } else
        return col;
}

void Colors::setupFixedPalettes()
{
    // the Google "Turbo" palette
    // https://gist.github.com/mikhailov-work/6a308c20e494d9e0ccc29036b28faa7a
    // https://ai.googleblog.com/2019/08/turbo-improved-rainbow-colormap-for.html
    int turbo_srgb_bytes[256][3] = {{48,18,59},{50,21,67},{51,24,74},{52,27,81},{53,30,88},{54,33,95},{55,36,102},{56,39,109},{57,42,115},{58,45,121},{59,47,128},{60,50,134},{61,53,139},{62,56,145},{63,59,151},{63,62,156},{64,64,162},{65,67,167},{65,70,172},{66,73,177},{66,75,181},{67,78,186},{68,81,191},{68,84,195},{68,86,199},{69,89,203},{69,92,207},{69,94,211},{70,97,214},{70,100,218},{70,102,221},{70,105,224},{70,107,227},{71,110,230},{71,113,233},{71,115,235},{71,118,238},{71,120,240},{71,123,242},{70,125,244},{70,128,246},{70,130,248},{70,133,250},{70,135,251},{69,138,252},{69,140,253},{68,143,254},{67,145,254},{66,148,255},{65,150,255},{64,153,255},{62,155,254},{61,158,254},{59,160,253},{58,163,252},{56,165,251},{55,168,250},{53,171,248},{51,173,247},{49,175,245},{47,178,244},{46,180,242},{44,183,240},{42,185,238},{40,188,235},{39,190,233},{37,192,231},{35,195,228},{34,197,226},{32,199,223},{31,201,221},{30,203,218},{28,205,216},{27,208,213},{26,210,210},{26,212,208},{25,213,205},{24,215,202},{24,217,200},{24,219,197},{24,221,194},{24,222,192},{24,224,189},{25,226,187},{25,227,185},{26,228,182},{28,230,180},{29,231,178},{31,233,175},{32,234,172},{34,235,170},{37,236,167},{39,238,164},{42,239,161},{44,240,158},{47,241,155},{50,242,152},{53,243,148},{56,244,145},{60,245,142},{63,246,138},{67,247,135},{70,248,132},{74,248,128},{78,249,125},{82,250,122},{85,250,118},{89,251,115},{93,252,111},{97,252,108},{101,253,105},{105,253,102},{109,254,98},{113,254,95},{117,254,92},{121,254,89},{125,255,86},{128,255,83},{132,255,81},{136,255,78},{139,255,75},{143,255,73},{146,255,71},{150,254,68},{153,254,66},{156,254,64},{159,253,63},{161,253,61},{164,252,60},{167,252,58},{169,251,57},{172,251,56},{175,250,55},{177,249,54},{180,248,54},{183,247,53},{185,246,53},{188,245,52},{190,244,52},{193,243,52},{195,241,52},{198,240,52},{200,239,52},{203,237,52},{205,236,52},{208,234,52},{210,233,53},{212,231,53},{215,229,53},{217,228,54},{219,226,54},{221,224,55},{223,223,55},{225,221,55},{227,219,56},{229,217,56},{231,215,57},{233,213,57},{235,211,57},{236,209,58},{238,207,58},{239,205,58},{241,203,58},{242,201,58},{244,199,58},{245,197,58},{246,195,58},{247,193,58},{248,190,57},{249,188,57},{250,186,57},{251,184,56},{251,182,55},{252,179,54},{252,177,54},{253,174,53},{253,172,52},{254,169,51},{254,167,50},{254,164,49},{254,161,48},{254,158,47},{254,155,45},{254,153,44},{254,150,43},{254,147,42},{254,144,41},{253,141,39},{253,138,38},{252,135,37},{252,132,35},{251,129,34},{251,126,33},{250,123,31},{249,120,30},{249,117,29},{248,114,28},{247,111,26},{246,108,25},{245,105,24},{244,102,23},{243,99,21},{242,96,20},{241,93,19},{240,91,18},{239,88,17},{237,85,16},{236,83,15},{235,80,14},{234,78,13},{232,75,12},{231,73,12},{229,71,11},{228,69,10},{226,67,10},{225,65,9},{223,63,8},{221,61,8},{220,59,7},{218,57,7},{216,55,6},{214,53,6},{212,51,5},{210,49,5},{208,47,5},{206,45,4},{204,43,4},{202,42,4},{200,40,3},{197,38,3},{195,37,3},{193,35,2},{190,33,2},{188,32,2},{185,30,2},{183,29,2},{180,27,1},{178,26,1},{175,24,1},{172,23,1},{169,22,1},{167,20,1},{164,19,1},{161,18,1},{158,16,1},{155,15,1},{152,14,1},{149,13,1},{146,11,1},{142,10,1},{139,9,2},{136,8,2},{133,7,2},{129,6,2},{126,5,2},{122,4,3}};
    std::vector<std::pair<float, QString> > turbo_grad;
    for (int i=0;i<256;++i) {
        QColor c(turbo_srgb_bytes[i][0], turbo_srgb_bytes[i][1], turbo_srgb_bytes[i][2]);
        turbo_grad.push_back(std::pair<float, QString>(i/256.f, c.name()));
    }
    // for now without interpolation
    mTurboColors.clear();
    for (unsigned int i=0;i<256;++i)
        mTurboColors.push_back(QColor(turbo_grad[i].second));

}

// colors
QColor Colors::colorFromValue(const float value,
                              const float min_value, const float max_value,
                              const bool reverse, const bool black_white)
{
    float rval = value;
    rval = std::max(min_value, rval);
    rval = std::min(max_value, rval);
    if (reverse)
        rval = max_value - rval;
    float rel_value;
    QColor col;
    if (min_value < max_value) {
        // default: high values -> red (h=0), low values to blue (h=high)
        rel_value = 1 - (rval - min_value) / (max_value - min_value);
        if (black_white) {
            int c = (1.-rel_value)*255;
            col = QColor(c,c,c);
        } else
            col=  QColor::fromHsvF(0.66666666666*rel_value, 0.95, 0.95).rgb();
    } else {
        col = Qt::white;
    }
    return col;
}

QColor Colors::colorFromValue(const float value, const GridViewType view_type, const float min_value, const float max_value)
{
    if (view_type==GridViewGray || view_type==GridViewGrayReverse)
        return colorFromValue(value, min_value, max_value, view_type==GridViewGrayReverse, true);

    if (view_type==GridViewRainbow || view_type==GridViewRainbowReverse)
        return colorFromValue(value, min_value, max_value, view_type==GridViewRainbowReverse, false);

    if (view_type == GridViewGreens || view_type==GridViewBlues || view_type==GridViewReds) {
        float rval = value;
        rval = std::max(min_value, rval);
        rval = std::min(max_value, rval);
        float rel_value = (max_value!=min_value)?(rval - min_value) / (max_value - min_value): 0;
        int r,g,b;
        switch (view_type) {
        case GridViewGreens:  // 11,111,19
            r=220 - rel_value*(220-11); g=220-rel_value*(220-111); b=220-rel_value*(220-19); break;
        case GridViewBlues: //15,67,138
            r=220 - rel_value*(220-15); g=220-rel_value*(220-67); b=220-rel_value*(220-138); break;
        case GridViewReds: //219,31,72
            r=240 - rel_value*(220-219); g=240-rel_value*(220-31); b=240-rel_value*(220-72); break;
        default: r=g=b=0;
        }
        return QColor(r,g,b);

    }
    if (view_type == GridViewHeat) {
        float rval = value;
        rval = std::max(min_value, rval);
        rval = std::min(max_value, rval);
        float rel_value = 1 - (rval - min_value) / (max_value - min_value);
        int g=255, b=0;
        if (rel_value < 0.5)
            g = rel_value*2.f * 255;
        if (rel_value>0.5)
            b = (rel_value-0.5)*2.f * 255;
        return QColor(255,g,b);
    }
    if (view_type == GridViewTurbo) {
        // google turbo palette
        float rval = value;
        rval = std::max(min_value, rval);
        rval = std::min(max_value, rval);
        float rel_value = (max_value!=min_value)?(rval - min_value) / (max_value - min_value): 0.f;
        int i_col = std::min(static_cast<int>(rel_value * 256), 255);
        return mTurboColors[i_col];
    }

    return colorFromPalette(value, view_type);


}
