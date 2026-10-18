#ifndef ImageStamp_H
#define ImageStamp_H

#include <QImage>

class ImageStamp
{
public:
    ImageStamp() {}
    ImageStamp(const QString& filename) { load(filename); }
    bool load(const QString& filename);
    float getXY(const float x, const float y);
    float get(const float r, const float phi);
    void setScale(const float radius, const float intensity) { rScale=radius; hScale=intensity; }
private:
    QImage mImage;
    float rScale;
    float hScale;

};

#endif // ImageStamp_H
