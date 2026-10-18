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
#ifndef COLORS_H
#define COLORS_H
#include <QObject>
#include <QColor>
#include "grid.h"
/** Colors: helper class for managing/selecting colors
 *
 * */
class DEM; // forward
class Colors: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList colors READ colors NOTIFY colorsChanged)
    Q_PROPERTY(QStringList labels READ labels NOTIFY colorsChanged)
    Q_PROPERTY(QStringList factorLabels READ factorLabels NOTIFY colorsChanged)
    Q_PROPERTY(int count READ colorCount NOTIFY colorsChanged)
    Q_PROPERTY(double minValue READ minValue WRITE setMinValue NOTIFY colorsChanged)
    Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue NOTIFY colorsChanged)
    Q_PROPERTY(bool autoScale READ autoScale WRITE setAutoScale NOTIFY colorsChanged)
    Q_PROPERTY(bool hasFactors READ hasFactors NOTIFY colorsChanged)
    Q_PROPERTY(QString caption READ caption NOTIFY colorsChanged)
    Q_PROPERTY(QString description READ description NOTIFY colorsChanged)
    Q_PROPERTY(double meterPerPixel READ meterPerPixel NOTIFY scaleChanged)



public:
    Colors(QWidget*parent=0);
    // properties
    QStringList colors() const {return mColors; }
    QStringList labels() const {return mLabels; }
    QStringList factorLabels() const {return mFactorLabels; }
    int colorCount() const { return mColors.count(); }
    double minValue() const {return mMinValue; }
    double maxValue() const {return mMaxValue; }
    void setMinValue(double val) { if(val==mMinValue) return;
        mNeedsPaletteUpdate=true; setPalette(mCurrentType, val, mMaxValue); mMinValue = val; }
    void setMaxValue(double val) { if(val==mMaxValue) return;
        mNeedsPaletteUpdate=true; setPalette(mCurrentType, mMinValue, val); mMaxValue = val; }
    bool hasFactors() const { return mHasFactors; }
    bool autoScale() const {return mAutoScale; }
    void setAutoScale(bool value) { if (value==mAutoScale) return; mAutoScale=value; mNeedsPaletteUpdate=true; setPalette(mCurrentType, mMinValue, mMaxValue);}
    QString caption() const {return mCaption; }
    QString description() const {return mDescription; }

    void setPalette(const GridViewType type, const float min_val, const float max_val);
    void setFactorLabels(QStringList labels);
    void setFactorColors(QStringList colors);
    void setCaption(QString caption, QString description=QString()) {
        if (mCaption==caption && mDescription==description) return;
        mCaption = caption; mDescription=description;mNeedsPaletteUpdate=true; }

    // scale
    double meterPerPixel() const {return mMeterPerPixel; }
    void setScale(double meter_per_pixel) { if(mMeterPerPixel==meter_per_pixel) return;
                                             mMeterPerPixel = meter_per_pixel; emit scaleChanged();}

    static QColor colorFromValue(const float value, const float min_value=0.f, const float max_value=1.f, const bool reverse=false, const bool black_white=false);
    static QColor colorFromValue(const float value, const GridViewType view_type, const float min_value=0.f, const float max_value=1.f);
    static QColor colorFromPalette(const int value, const GridViewType view_type);
    static QColor shadeColor(const QColor col, const QPointF &coordinates, const DEM *dem);
private:
    void setupFixedPalettes();
    static QVector<QColor> mBrewerDiv;
    static QVector<QColor> mBrewerQual;
    static QVector<QColor> mTerrainCol;
    static QVector<QColor> mCustomColors;
    static QVector<QColor> mTurboColors;
    QStringList mColors;
    QStringList mLabels;
    QStringList mFactorLabels;
    double mMinValue;
    double mMaxValue;
    GridViewType mCurrentType;
    bool mAutoScale;
    bool mHasFactors;
    bool mNeedsPaletteUpdate;
    QString mCaption;
    QString mDescription;
    double mMeterPerPixel;
signals:
    void colorsChanged();
    void scaleChanged();
};

#endif // COLORS_H
