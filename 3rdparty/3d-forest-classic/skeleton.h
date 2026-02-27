//
//  skeleton.h
//  3D_Forest
//
//  Created by Jan Trochta on 15.02.18.
//

#ifndef skeleton_h
#define skeleton_h

#include "segment.h"
#include <pcl/octree/octree_search.h>
#include <random>

namespace f3d
{

class Skeleton
{
public:
    Skeleton(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud, float radius, int multiplication);
    ~Skeleton();
    void compute();
    //void setTree(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud);
    void setRadius ( float r);
    void setMultiplicator(int m);
    void setPosition (pcl::PointXYZI p);
    std::vector<std::shared_ptr<Segment>> getSegments();
    pcl::PointCloud<pcl::PointXYZI>::Ptr getCloud();
    
protected:
    void searchFromPosition();
    bool findPoints(pcl::PointXYZI p,  float radius, std::vector<int>& pointID);
    void sampleConsensus(std::vector<int>& pointID, pcl::ModelCoefficients::Ptr modelCoeff);
    void setOctree();
    void setUsedPoints();
    void setCluster(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud);
    void coverSets(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_voxel,std::vector<int>& treeSet );
    void createVoxels(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_);
    int defineComponents(std::vector<int>& cut, std::vector<std::vector<int>>& segmentsCS);
    void getValidCS(std::vector<int>& input,std::vector<int>& output);
    void makeSmallBall(pcl::octree::OctreePointCloudSearch<pcl::PointXYZI> &octreeSearch, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_, std::vector<std::vector<int>> & inliersInCS, std::vector<int>& pointCSID, float radius);
    void makeBigBall(pcl::octree::OctreePointCloudSearch<pcl::PointXYZI> &octreeSearch, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_, std::vector<std::vector<int>> & inliers, float radius);
    void createNeighbors(std::vector<std::vector<int>>& inliersBigBall, std::vector<int>& pointsWithCSID);
    void makeTreeSets(std::vector<std::vector<int>>& segments);
    void mergeTreeSets(std::vector<std::vector<int>>& segments, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_,std::vector<int>& output);
    void Segmentation(std::vector<int> treeSet,pcl::PointCloud<pcl::PointXYZI>::Ptr cloud, std::vector<std::shared_ptr<Segment>>& components);
    void correctSegments(std::vector<std::shared_ptr<Segment>>& input);
    void setStem(std::vector<std::shared_ptr<Segment>>& components);
    void setStemTop(std::vector<std::shared_ptr<Segment>>& components);
    void setHighestVoxel(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_voxels );
    void setBranches(std::vector<std::shared_ptr<Segment>>& components);
    void mergeSegmentsIntoBranches(std::vector<std::shared_ptr<Segment>>& components, std::vector<std::shared_ptr<Segment>>& branches);
    float getShortestPath(std::shared_ptr<Segment> start, std::shared_ptr<Segment> end, std::vector<std::shared_ptr<Segment>>& segment, std::vector<int>& segmentIDs);
    float getSegmentLenght(std::vector<std::shared_ptr<Segment>>& segment, int startID, int endID, std::vector<int> & usedComponent);
    float getChildrenMaxLenght(std::vector<std::shared_ptr<Segment>>& segment, int startID, std::vector<int> & usedComponent);
    void setIntensity(std::vector<std::shared_ptr<Segment>>& branches, std::vector<std::vector<int>> inliers);// pouze pro zobrazeni
    int getLowestCS (std::vector<int>& segment, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_);
    int getHighestCS(std::vector<int>& segment, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_);
    void getNeighbors(std::vector<int>& component, std::vector<int>& cut );
    void computeDistances(std::vector<std::shared_ptr<Segment>>& branches, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_);
    void setConnectionPoint();
    void correctChildrens(std::vector<std::shared_ptr<Segment>>& branches);
    
    
    void createCylinder(std::vector<std::shared_ptr<Segment>>& segment, std::vector<std::vector<int>> inliers);
    Eigen::Vector3f computeMedian(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_segment);
    void getHalves(pcl::PointCloud<pcl::PointXYZI>::Ptr input,pcl::PointCloud<pcl::PointXYZI>::Ptr bottom,pcl::PointCloud<pcl::PointXYZI>::Ptr high);
    Eigen::Vector3f getInitialCylinderDir(pcl::PointCloud<pcl::PointXYZI>::Ptr input);
    float getDistance(Eigen::Vector3f axis , Eigen::Vector3f median, float radius, pcl::PointXYZI point);
    float getSquaredDistance(pcl::PointCloud<pcl::PointXYZI>::Ptr input, Eigen::Vector3f axis , float radius,Eigen::Vector3f median);
    float gFunction(pcl::PointCloud<pcl::PointXYZI>::Ptr input, Eigen::Vector3f axis , Eigen::Vector3f& PC, float& rSqr);
    
    void setCyliderInliers(pcl::ModelCoefficients::Ptr cylinder, std::vector<int> & indices);
    
    
    
    std::vector<bool> m_usedPoint;
    std::vector<bool> m_usedSegment;
    pcl::PointCloud<pcl::PointXYZI>::Ptr m_treeCloud;
    pcl::octree::OctreePointCloudSearch<pcl::PointXYZI> m_ocs;
    std::vector<pcl::ModelCoefficients::Ptr> m_spheres;
    std::vector<std::vector<int>> m_neighbors;
    std::vector<bool> m_usedCS;
    std::random_device m_rd;
    float m_radius;
    int m_multiplicator;
    std::vector<pcl::ModelCoefficients::Ptr> m_cylinders;
    pcl::PointXYZI m_position;
    std::vector<std::shared_ptr<Segment>> m_segments;
    std::vector<std::vector<int>> m_inliersSB; // each vector has vector of point indices in voxel
    int m_minCoversetSize = 3;
    int m_highestVoxel;
};

}

#endif /* skeleton_h */
