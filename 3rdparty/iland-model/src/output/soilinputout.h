#ifndef SOILINPUTOUT_H
#define SOILINPUTOUT_H
#include "output.h"

class SoilInputOut : public Output
{
public:
    SoilInputOut();
    virtual void exec();
    virtual void setup();

};

#endif // SOILINPUTOUT_H
