//
//  sortiment.cpp
//  3DForest043
//
//  Created by Jan Trochta on 13.08.18.
//

#include "sortiment.h"

namespace f3d
{

Sortiment::Sortiment()
{
    
}

Sortiment::Sortiment(std::vector<pcl::ModelCoefficientsPtr> cylinders, int curve, int bb, int taper, int skew, int grade)
{
    setCylinders(cylinders);
    setCurvatureGrade(curve);
    setBranchBifurcationGrade(bb);
    setTaperGrade(taper);
    setSkewnessGrade(skew);
    setQualityGrade(grade);
    
}
Sortiment::~Sortiment()
{
    
}
//sets
void Sortiment::setCylinders(std::vector<pcl::ModelCoefficientsPtr> cylinders)
{
    m_cylinders.clear();
    m_cylinders = cylinders;
    setSmallerEndDBH();
    setBiggerEndDBH();
    setMiddleDBH();
    computeVolume();
    computelenght();
}
void Sortiment::setQualityGrade(int i)
{
    m_qualityGrade = i;
}
void Sortiment::setCurvatureGrade(int i)
{
    m_curvatureGrade = i;
}
void Sortiment::setSkewnessGrade(int i)
{
    m_skewnessGrade = i;
}
void Sortiment::setBranchBifurcationGrade(int i)
{
    m_branchBifurcationGrade = i;
}
void Sortiment::setTaperGrade(int i)
{
    m_taperGrade = i;
}
//gets
std::vector<pcl::ModelCoefficientsPtr> Sortiment::getCylinders()
{
    return m_cylinders;
}
float Sortiment::getVolume()
{
    return m_volume;
}
float Sortiment::getLength()
{
    return m_length;
}
int Sortiment::getGrade()
{
    return m_qualityGrade;
}
float Sortiment::getBiggerEndDBH()
{
    return m_biggerEndDBH;
}
float Sortiment::getSmallerEndDBH()
{
    return m_smallerEndDBH;
}
float Sortiment::getMiddleDBH()
{
    return m_middleDBH;
}
void Sortiment::computelenght()
{
    m_length = 0;
    for(int q=0;q< m_cylinders.size();q++)
    {
        m_length+=cylinderLength(m_cylinders.at(q));
    }
}
void Sortiment::computeVolume()
{
    m_volume =0;
    for(int q=0;q< m_cylinders.size();q++)
    {
        m_volume+=cylinderVolume(m_cylinders.at(q));
    }
}
void Sortiment::setBiggerEndDBH()
{
    // first and last cylinder and compare its size
    m_biggerEndDBH = m_cylinders.at(0)->values.at(6);
    if(m_cylinders.at(m_cylinders.size()-1)->values.at(6) > m_biggerEndDBH)
        m_biggerEndDBH = m_cylinders.at(m_cylinders.size()-1)->values.at(6);
}
void Sortiment::setSmallerEndDBH()
{
    m_smallerEndDBH = m_cylinders.at(0)->values.at(6);
    if(m_cylinders.at(m_cylinders.size()-1)->values.at(6) < m_smallerEndDBH)
        m_smallerEndDBH = m_cylinders.at(m_cylinders.size()-1)->values.at(6);
}
void Sortiment::setMiddleDBH()
{
    m_middleDBH = (m_biggerEndDBH + m_smallerEndDBH)/2;
}
float Sortiment::cylinderVolume(pcl::ModelCoefficients::Ptr c)
{
    float v = 0;
    float volume = 0;
    v = cylinderLength(c);
    volume =(M_PI * (c->values.at(6) * c->values.at(6)) * v);
    return volume;
}
float Sortiment::cylinderLength(pcl::ModelCoefficients::Ptr c)
{
    float lenght= std::sqrt(
                  (c->values.at(3) * c->values.at(3)) +
                  (c->values.at(4) * c->values.at(4)) +
                  (c->values.at(5) * c->values.at(5)) );
    if(std::isnan(lenght))
        return 0;
    
    return lenght;
}

}
