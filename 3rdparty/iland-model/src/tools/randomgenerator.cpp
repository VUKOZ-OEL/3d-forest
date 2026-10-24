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
#include <MersenneTwister.h>

#include <QMutex>
#include <random>

class RGenerators
{
public:
    RGenerators() {}
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

    unsigned int g_seed;
    unsigned int legacySeed;
    unsigned int legacyRand() {
        // Preserve MSVC's historical initialization sequence without CRT state.
        legacySeed = legacySeed * 214013u + 2531011u;
        return (legacySeed >> 16) & 0x7fffu;
    }
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


inline void RGenerators::seed(unsigned int oneSeed)
{
    legacySeed = oneSeed;
    for (int i=0;i<16;i++)
        state[i] = legacyRand();
    g_seed = legacyRand();
    index = 0;
    x=123456789, y=362436069, z=521288629;
}

// static variables
RandomGenerator::RunType_mBuffer &RandomGenerator::runState_mBuffer()
{
    struct State { RunType_mBuffer value = {}; };
    return SimulationContext::state<State>().value;
}
RandomGenerator::RunType_mIndex &RandomGenerator::runState_mIndex()
{
    struct State { RunType_mIndex value = 0; };
    return SimulationContext::state<State>().value;
}
RandomGenerator::RunType_mRotationCount &RandomGenerator::runState_mRotationCount()
{
    struct State { RunType_mRotationCount value = RANDOMGENERATORROTATIONS + 1; };
    return SimulationContext::state<State>().value;
}
RandomGenerator::RunType_mRefillCounter &RandomGenerator::runState_mRefillCounter()
{
    struct State { RunType_mRefillCounter value = 0; };
    return SimulationContext::state<State>().value;
}
RandomGenerator::RunType_mGeneratorType &RandomGenerator::runState_mGeneratorType()
{
    struct State { RunType_mGeneratorType value = RandomGenerator::ergFastRandom; };
    return SimulationContext::state<State>().value;
}



/// fill the internal buffer with random numbers choosen from the defined random generator
void RandomGenerator::refill() {
    std::lock_guard<std::recursive_mutex> lock(mutex());
    if (runState_mRotationCount()<RANDOMGENERATORROTATIONS) // another thread might already succeeded in refilling....
        return;
    runState_mIndex() = 0; // reset the index
    runState_mRotationCount()=0;
    runState_mRefillCounter()++;

    RGenerators gen;
    gen.seed(runState_mBuffer()[RANDOMGENERATORSIZE+4]); // use the last value as seed for the next round....
    switch (runState_mGeneratorType()) {
    case ergMersenneTwister: {
        MTRand mersenne(runState_mBuffer()[RANDOMGENERATORSIZE+4]);
        // qDebug() << "refill random numbers. seed" <<mBuffer[RANDOMGENERATORSIZE+4];
        //mersenne.seed(); // use random number seed from mersenne twister (hash of time and clock)
        for (int i=0;i<RANDOMGENERATORSIZE+5;++i)
            runState_mBuffer()[i] = mersenne.randInt();
        break;
    }
    case ergWellRNG512: {

        for (int i=0;i<RANDOMGENERATORSIZE+5;++i)
            runState_mBuffer()[i] = gen.random_function(0);
        break;
    }
    case ergXORShift96: {
        for (int i=0;i<RANDOMGENERATORSIZE+5;++i)
            runState_mBuffer()[i] = gen.random_function(1);
        break;
    }
    case ergFastRandom: {
        for (int i=0;i<RANDOMGENERATORSIZE+5;++i)
            runState_mBuffer()[i] = gen.random_function(2);
        break;
    }
    } // switch


}

void RandomGenerator::seed(const unsigned oneSeed)
{
    std::lock_guard<std::recursive_mutex> lock(mutex());
    if (oneSeed==0) {
        std::random_device entropy;
        runState_mBuffer()[RANDOMGENERATORSIZE+4] = entropy();
    } else {
        runState_mBuffer()[RANDOMGENERATORSIZE+4] = oneSeed; // set a specific seed as seed for the next round
    }
}
