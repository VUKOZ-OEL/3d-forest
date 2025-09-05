//
//  segment.cpp
//  3D_Forest
//
//  Created by Jan Trochta on 15.05.18.
//

#include "segment.h"

namespace f3d
{

// Segment
Segment::Segment()
{
    
}
Segment::Segment(std::vector<int> cs, int currID)
{
    setCoverSet(cs);
    setCurrID(currID);
    //setParent(parent);
    //setOrder(order);
}
Segment::~Segment()
{
    
}
void Segment::setCoverSet(std::vector<int> cs)
{
    m_coversets=cs;
}
void Segment::setParent (std::shared_ptr<Segment> p)
{
    m_parent=p;
}
void Segment::setChild(std::shared_ptr<Segment> childId)
{
    m_childs.push_back(childId);
}
std::shared_ptr<Segment> Segment::getParent()
{
    return m_parent;
}
std::vector<int> Segment::getCoverSets()
{
    return m_coversets;
}
void Segment::addCoverSet(std::vector<int> cs)
{
    m_coversets.insert(m_coversets.end(), cs.begin(), cs.end());
}
void Segment::getChildrens(std::vector<std::shared_ptr<Segment>> & childs)
{
    childs = m_childs;
}
void Segment::setLenght(float length)
{
    m_lenght = length;
}
float Segment::getLenght()
{
    return m_lenght;
}
void Segment::setOrder(int order)
{
    m_order = order;
}
int Segment::getOrder()
{
    return m_order;
}
void Segment::computeLenght (pcl::PointCloud<pcl::PointXYZI>::Ptr cloud)
{
    m_lenght=0;
    for(int q=0; q < m_coversets.size();q++)
    {
        for(int w=0;w < m_coversets.size();w++)
        {
            if(w==q)
                continue;
            int low = m_coversets.at(q);
            int high = m_coversets.at(w);
            float x = cloud->points.at(low).x - cloud->points.at(high).x;
            float y = cloud->points.at(low).y - cloud->points.at(high).y;
            float z = cloud->points.at(low).z - cloud->points.at(high).z;
            float dist = std::sqrt(x*x + y*y + z*z);
            if(dist > m_lenght)
            {
                m_lenght = dist;
            }
        }
    }
}
int Segment::getChildsSize()
{
    return m_childs.size();
}
void Segment::setCurrID(int i)
{
    m_currentID = i;
}
int Segment::getCurrID()
{
    return m_currentID;
}
void Segment::setBranchID(int i)
{
    m_branchID = i;
}
int Segment::getBranchID()
{
    return m_branchID;
}
void Segment::setChildrens(std::vector<std::shared_ptr<Segment>> & cs)
{
    m_childs = cs;
}
void Segment::addChildren(std::shared_ptr<Segment>& cs)
{
    m_childs.push_back(cs);
}
void Segment::removeChildrens()
{
    m_childs.clear();
}
void Segment::setInliers(std::vector<int> inliers)
{
    m_inliers= inliers;
}
void Segment::addInliers(std::vector<int> inliers)
{
    m_inliers.insert(m_inliers.end(), inliers.begin(), inliers.end());
}
std::vector<int> Segment::getInliers()
{
    return m_inliers;
}
void Segment::setCylinders(std::vector<pcl::ModelCoefficients::Ptr> cylinders)
{
    m_cylinders = cylinders;
}
void Segment::addCylinder(pcl::ModelCoefficients::Ptr cylinder)
{
    m_cylinders.push_back(cylinder);
}
std::vector<pcl::ModelCoefficients::Ptr> Segment::getCylinders()
{
    return m_cylinders;
}
void Segment::setConnPoint()
{
    //find point that is closet to the parent of if stem to the position
    
    
}
void Segment::setConnPoint(pcl::PointXYZI p)
{
    m_connectionPoint = p;
}
pcl::PointXYZI Segment::getConnPoint()
{
    return m_connectionPoint;
}
void Segment::setParentCylinderPoint(pcl::PointXYZI i)
{
    m_connectionPointParent = i;
}
pcl::PointXYZI Segment::getParentCP()
{
    return m_connectionPointParent;
}
void Segment::setProfile(std::vector<stred> p)
{
    m_profile = p;
}
std::vector<stred>& Segment::getProfile()
{
    return m_profile;
}
void Segment::setTotalVolume()
{
    for(int q=0;q < m_cylinders.size();q++)
    {
        m_totalVolume+=cylinderVolume(m_cylinders.at(q));
    }
}
void Segment::setHroubiVolume()
{
    for(int q=0;q < m_cylinders.size();q++)
    {
        if(m_cylinders.at(q)->values.at(6) > 0.034)
            m_hroubiVolume+=cylinderVolume(m_cylinders.at(q));
    }
}
void Segment::setHroubiLength()
{
    m_hroubiLength = 0;
    float v=0;
    for(int q=0;q < m_cylinders.size();q++)
    {
        if(m_cylinders.at(q)->values.at(6) > 0.034)
        {

            v = std::sqrt(
                          (m_cylinders.at(q)->values.at(3) * m_cylinders.at(q)->values.at(3)) +
                          (m_cylinders.at(q)->values.at(4) * m_cylinders.at(q)->values.at(4)) +
                          (m_cylinders.at(q)->values.at(5) * m_cylinders.at(q)->values.at(5)) );
            
            if(std::isnan(v))
            {
                v=0;
            }
            m_hroubiLength+=v;
        }
    }
}
float Segment::getTotalVolume()
{
    return m_totalVolume;
}
float Segment::getHroubiVolume()
{
    return m_hroubiVolume;
}
float Segment::getHroubiLength()
{
    return m_hroubiLength;
}
float Segment::cylinderVolume(pcl::ModelCoefficients::Ptr c)
{
    float v = 0;
    float volume = 0;
    v = std::sqrt(
                  (c->values.at(3) * c->values.at(3)) +
                  (c->values.at(4) * c->values.at(4)) +
                  (c->values.at(5) * c->values.at(5)) );
    
    volume =(M_PI * (c->values.at(6) * c->values.at(6)) * v);
    
    if(std::isnan(v))
    {
        v=0;
        volume = 0;
    }
    return volume;
}
void Segment::setSortiment(std::shared_ptr<Sortiment> sort)
{
    m_sortiments.push_back(sort);
}
void Segment::setSortiments(std::vector<std::shared_ptr<Sortiment>> sortiments)
{
    m_sortiments = sortiments;
}
std::vector<std::shared_ptr<Sortiment>> & Segment::getSortiments()
{
    return m_sortiments;
}
void Segment::clearSortiments()
{
    m_sortiments.clear();
}

}
