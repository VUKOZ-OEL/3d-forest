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
#include "randomgenerator.h"
#include <QtGlobal>
#include "../3rdparty/MersenneTwister.h"

#include <QMutex>

class RGenerators
{
public:
    RGenerators() {}
    void seed();
    void seed(unsigned int oneSeed);
    inline unsigned int random_function(const int type) {   if (type==0) return WELLRNG512();
                                                            if (type==1) return xorshf96();
                                                            if (type==2) return fastrand();
                                                            return 0;
                                                        }
private:
    // see  http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf
    // for details on the WellRNG512 algorithm
    inline unsigned long WELLRNG512(void);
    /* initialize state to random bits  */
    unsigned long state[16];
    /* init should also reset this to 0 */
    unsigned int index;
    /* return 32 bit random number      */

    int g_seed;
    inline unsigned int fastrand()
    {
        g_seed = (214013*g_seed+2531011);
        return g_seed;
    }
    unsigned long x,y,z;
    unsigned long xorshf96(void);

};

inline unsigned long RGenerators::WELLRNG512(void)
{
unsigned long a, b, c, d;
a  = state[index];
c  = state[(index+13)&15];
b  = a^c^(a<<16)^(c<<15);
c  = state[(index+9)&15];
c ^= (c>>11);
a  = state[index] = b^c;
d  = a^((a<<5)&0xDA442D24UL);
index = (index + 15)&15;
a  = state[index];
state[index] = a^b^d^(a<<2)^(b<<18)^(c<<28);
return state[index];
}
// The Marsaglia's xorshf generator:
// see: http://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c and
// http://www.cse.yorku.ca/~oz/marsaglia-rng.html
//unsigned long x=123456789, y=362436069, z=521288629;
unsigned long RGenerators::xorshf96(void) {          //period 2^96-1
unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

   t = x;
   x = y;
   y = z;
   z = t ^ x ^ y;

  return z;
}


inline void RGenerators::seed()
{
    srand ( time(NULL) );
    for (int i=0;i<16;i++)
        state[i] = std::rand();
    index = 0;
    // inits for the xorshift algorithm...
    x=123456789, y=362436069, z=521288629;
    // inits for the fast rand....
    g_seed = std::rand();
}

inline void RGenerators::seed(unsigned int oneSeed)
{
    srand ( oneSeed );
    for (int i=0;i<16;i++)
        state[i] = std::rand();
    index = 0;
    x=123456789, y=362436069, z=521288629;
    g_seed = oneSeed;
}

// static variables
unsigned int RandomGenerator::mBuffer[RANDOMGENERATORSIZE+5];
int RandomGenerator::mIndex = 0;
int RandomGenerator::mRotationCount = RANDOMGENERATORROTATIONS + 1;
int RandomGenerator::mRefillCounter = 0;
RandomGenerator::ERandomGenerators RandomGenerator::mGeneratorType = RandomGenerator::ergFastRandom;



/// fill the internal buffer with random numbers choosen from the defined random generator
QMutex random_generator_refill_mutex;

void RandomGenerator::refill() {

    QMutexLocker lock(&random_generator_refill_mutex); // serialize access
    if (mRotationCount<RANDOMGENERATORROTATIONS) // another thread might already succeeded in refilling....
        return;

    RGenerators gen;
    gen.seed(mBuffer[RANDOMGENERATORSIZE+4]); // use the last value as seed for the next round....
    switch (mGeneratorType) {
    case ergMersenneTwister: {
        MTRand mersenne;
        // qDebug() << "refill random numbers. seed" <<mBuffer[RANDOMGENERATORSIZE+4];
        mersenne.seed(mBuffer[RANDOMGENERATORSIZE+4]);
        //mersenne.seed(); // use random number seed from mersenne twister (hash of time and clock)
        for (int i=0;i<RANDOMGENERATORSIZE+5;++i)
            mBuffer[i] = mersenne.randInt();
        break;
    }
    case ergWellRNG512: {

        for (int i=0;i<RANDOMGENERATORSIZE+5;++i)
            mBuffer[i] = gen.random_function(0);
        break;
    }
    case ergXORShift96: {
        for (int i=0;i<RANDOMGENERATORSIZE+5;++i)
            mBuffer[i] = gen.random_function(1);
        break;
    }
    case ergFastRandom: {
        for (int i=0;i<RANDOMGENERATORSIZE+5;++i)
            mBuffer[i] = gen.random_function(2);
        break;
    }
    } // switch


    mIndex = 0; // reset the index
    mRotationCount=0;
    mRefillCounter++;
}

void RandomGenerator::seed(const unsigned oneSeed)
{
    if (oneSeed==0) {
        srand ( time(NULL) );
        mBuffer[RANDOMGENERATORSIZE+4] = std::rand();
    } else {
        mBuffer[RANDOMGENERATORSIZE+4] = oneSeed; // set a specific seed as seed for the next round
    }
}
