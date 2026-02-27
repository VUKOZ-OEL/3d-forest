//
//  segment.hpp
//  3D_Forest
//
//  Created by Jan Trochta on 15.05.18.
//

#ifndef segment_h
#define segment_h

#include <stdio.h>
#include <pcl/common/common_headers.h>
#include "sortiment.h"
#include "stred.h"

namespace f3d
{

class Segment // segment as single part of tree containts reference to coverset ( int), parentID, childrenIDs
{
public:
    Segment();
    Segment(std::vector<int> cs, int curr);
    ~Segment();
    void setCoverSet(std::vector<int> cs);
    void addCoverSet(std::vector<int> cs);
    void setParent (std::shared_ptr<Segment> p);
    void setChild(std::shared_ptr<Segment> childId);
    void setChildrens(std::vector<std::shared_ptr<Segment>> & cs);
    void addChildren(std::shared_ptr<Segment>& cs);
    std::shared_ptr<Segment> getParent();
    std::vector<int> getCoverSets();
    void getChildrens(std::vector<std::shared_ptr<Segment>> & childs);
    void removeChildrens();
    void setLenght(float length);
    float getLenght();
    void setOrder(int order);
    int getOrder();
    void setCurrID(int i);
    int getCurrID();
    void setBranchID(int i);
    int getBranchID();
    void setInliers(std::vector<int> inliers);
    void addInliers(std::vector<int> inliers);
    std::vector<int> getInliers();
    void computeLenght(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud);
    int getChildsSize();
    void setCylinders(std::vector<pcl::ModelCoefficients::Ptr> cylinders);
    void addCylinder(pcl::ModelCoefficients::Ptr cylinder);
    std::vector<pcl::ModelCoefficients::Ptr> getCylinders();
    void setConnPoint();
    void setConnPoint(pcl::PointXYZI p);
    pcl::PointXYZI getConnPoint();
    void setParentCylinderPoint(pcl::PointXYZI i);
    pcl::PointXYZI getParentCP();
    void setProfile(std::vector<stred> p);
    std::vector<stred>& getProfile();
    void setTotalVolume();
    void setHroubiVolume();
    void setHroubiLength();
    float getTotalVolume();
    float getHroubiVolume();
    float getHroubiLength();
    void setSortiment(std::shared_ptr<Sortiment> sort);
    void setSortiments(std::vector<std::shared_ptr<Sortiment> > sortiments);
    std::vector<std::shared_ptr<Sortiment> > & getSortiments();
    void clearSortiments();
    

protected:
    
    float cylinderVolume(pcl::ModelCoefficients::Ptr c);
    std::vector<int> m_coversets;
    std::vector<int> m_inliers;
    std::shared_ptr<Segment> m_parent;
    int m_currentID;
    std::vector<std::shared_ptr<Segment>> m_childs;
    float m_lenght;
    float m_hroubiLength;
    int m_order;
    int m_branchID;
    std::vector<pcl::ModelCoefficients::Ptr> m_cylinders;
    pcl::PointXYZI m_connectionPoint;
    pcl::PointXYZI m_connectionPointParent;
    std::vector<stred> m_profile;
    float m_totalVolume=0;
    float m_hroubiVolume=0;
    std::vector<std::shared_ptr<Sortiment> > m_sortiments;
    
};

}

#endif /* segment_hpp */
