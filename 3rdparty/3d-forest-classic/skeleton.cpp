//
//  skeleton.cpp
//  3DForest
//
//  Created by Jan Trochta on 15.02.18.
//

#include <stdio.h>
#include "skeleton.h"
#include <pcl/octree/octree_search.h>
#include <pcl/common/common_headers.h>
#include <pcl/common/centroid.h>
#include <pcl/common/pca.h>
#include <pcl/search/kdtree.h>
#include <chrono>

namespace f3d
{

Skeleton::Skeleton(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud, float radius, int multiplication)
:m_ocs(0.05)
{
    //setTree(tree);
    m_treeCloud = cloud;
    setRadius(radius);
    setMultiplicator(multiplication);
    setOctree();
    setUsedPoints();
    m_usedPoint.resize(m_treeCloud->points.size(), false);
}
Skeleton::~Skeleton()
{
    
}
void Skeleton::setRadius ( float r)
{
    m_radius = r;
}
void Skeleton::setMultiplicator(int m)
{
    m_multiplicator = m;
}
void Skeleton::setPosition (pcl::PointXYZI p)
{
    m_position = p;
}
void Skeleton::compute()
{
    std::vector<int> treeSet;
    pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_voxels (new pcl::PointCloud<pcl::PointXYZI>);
    
    coverSets(cloud_voxels, treeSet);
    std::vector<std::shared_ptr<Segment>> components;
   // std::cout<< "tree sets: "<< treeSet.size()<< " pocet voxelu: " << cloud_voxels->points.size() << "\n";
    Segmentation(treeSet, cloud_voxels, components);
  //  std::cout<< "tree sets: "<< treeSet.size()<< " pocet bodu stromu: " << m_treeCloud->points.size() <<" veliksot components: " << components.size()<< "\n";
   // setIntensity(components, m_inliersSB);
    // for each components compute lenght
    computeDistances(components, cloud_voxels);
    correctSegments(components);
    computeDistances(components, cloud_voxels);
    // new stem
    setHighestVoxel(cloud_voxels);
    setStemTop(components);
    //
    //setStem(components);
    setBranches(components);
    //setIntensity(components, m_inliersSB);
    //std::vector<std::shared_ptr<Segment>> branches;
    mergeSegmentsIntoBranches(components, m_segments);
    //correctChildrens(m_segments);
   // std::cout<<"koncovy pocet vetvi :" << m_segments.size()<< "\n";
    computeDistances(m_segments, cloud_voxels);
    //setConnectionPoint();
    setIntensity(m_segments, m_inliersSB);
    setConnectionPoint();
}
void Skeleton::searchFromPosition()
{
    // get position
   // pcl::PointXYZI p = m_tree->getPosition();
    std::vector<int> indices;
   // pcl::PointCloud<pcl::PointXYZI>::Ptr input (new pcl::PointCloud<pcl::PointXYZI>);
    if(findPoints(m_position,0.50, indices))
    {
        // setCluster(indices);
       // std::cout<< " bocet bodu: "<< indices.size()<<"\n";
        pcl::ModelCoefficients::Ptr modelcoef (new pcl::ModelCoefficients);

        sampleConsensus(indices, modelcoef);
        modelcoef->values.at(2) =m_position.z;
        m_spheres.push_back(modelcoef);
    }
}
std::vector<std::shared_ptr<Segment>> Skeleton::getSegments()
{
    return m_segments;
}
pcl::PointCloud<pcl::PointXYZI>::Ptr Skeleton::getCloud()
{
    return m_treeCloud;
}
bool Skeleton::findPoints(pcl::PointXYZI p, float radius, std::vector<int> & pointID) // get indices of point in radius of point p
{
   // float resolution = 0.05;
    //float radius = 1.;
    std::vector<int> circle;

    std::vector<int> pointIDv;
    std::vector<float> pointSDv;
    // find all point in radius
    //std::cout<<"prvni hledani\n";
    if(m_ocs.radiusSearch(p, radius, pointIDv, pointSDv)>0)// find neighboring vox
    {
        for(int a=0; a < pointIDv.size(); a++)
        {
            if (m_usedPoint.at(pointIDv.at(a)) != true)
                pointID.push_back(pointIDv.at(a));
        }
        return true;
    }
    return false;
}
void Skeleton::sampleConsensus(std::vector<int> & pointID, pcl::ModelCoefficients::Ptr modelCoeff)
{
//    std::cout<<"SampleConsensusModelCircle3D\n";
//    float x=0,y=0,z=0;
//
//    pcl::PointCloud<pcl::PointXYZI>::Ptr input (new pcl::PointCloud<pcl::PointXYZI>);
//    for(int q=0; q < pointID.size(); q++)
//    {
//        input->points.push_back(m_treeCloud->points.at(pointID.at(q)));
//        x+=m_treeCloud->points.at(pointID.at(q)).x;
//        y+=m_treeCloud->points.at(pointID.at(q)).y;
//        z+=m_treeCloud->points.at(pointID.at(q)).z;
//    }
//    x/= pointID.size();
//    y/= pointID.size();
//    z/= pointID.size();
//    // normal estimation
//        pcl::PointCloud<pcl::PointNormal>::Ptr input_normals (new pcl::PointCloud<pcl::PointNormal>);
//
//        pcl::NormalEstimation< pcl::PointXYZI, pcl::PointNormal > ne;
//        ne.setInputCloud(input);
//       // ne.setViewPoint(x, y, z);
//        ne.setKSearch(5);
//        ne.compute(*input_normals);
//
////    std::vector<int> inliers;
////    pcl::SampleConsensusModelCylinder<pcl::PointXYZI, pcl::PointNormal>::Ptr model_s (new pcl::SampleConsensusModelCylinder<pcl::PointXYZI, pcl::PointNormal> (input));
////    model_s->setInputNormals(input_normals);
////    pcl::RandomSampleConsensus<pcl::PointXYZI> ransac (model_s);
////    ransac.setDistanceThreshold (.02);
////    ransac.setMaxIterations(1000);
////    ransac.setProbability(0.9);
////    ransac.refineModel();
////    ransac.computeModel();
////    ransac.getInliers(inliers);
////    Eigen::VectorXf coef;
////    ransac.getModelCoefficients(coef);
////
////    std::vector<float> values_;
////    values_.push_back (coef[0]);
////    values_.push_back (coef[1]);
////    values_.push_back (coef[2]);
////    values_.push_back (coef[4]);
////    values_.push_back (coef[5]);
////    values_.push_back (coef[6]);
////    values_.push_back (coef[3]);
////
////    modelCoeff->values = values_;
////
//    /// druha metoda
//
//    // normal estimation
////    pcl::PointCloud<pcl::PointNormal>::Ptr input_normals (new pcl::PointCloud<pcl::PointNormal>);
////
////    pcl::NormalEstimation< pcl::PointXYZI, pcl::PointNormal >::NormalEstimation ne;
////    ne.setInputCloud(input);
////    ne.setViewPoint(x, y, z);
////    ne.setKSearch(5);
////    ne.compute(*input_normals);
////
////    pcl::ModelCoefficients::Ptr coefficients_circle3d(new pcl::ModelCoefficients);
////
//    pcl::SACSegmentationFromNormals<pcl::PointXYZI, pcl::PointNormal> seg;
//    pcl::PointIndices::Ptr inliers_cylinder(new pcl::PointIndices);
////
////   // seg.setOptimizeCoefficients(true);
////   // seg.setModelType(pcl::SACMODEL_CIRCLE3D);
////   // seg.setMethodType(pcl::SAC_MLESAC);
////   // seg.setMaxIterations(1000);
////    //seg.setDistanceThreshold(0.02);
////
//    seg.setOptimizeCoefficients (true);
//    seg.setModelType (pcl::SACMODEL_CYLINDER);
//    seg.setMethodType (pcl::SAC_RANSAC);
//    seg.setNormalDistanceWeight (0.1);
//    seg.setMaxIterations (10000);
//    seg.setDistanceThreshold (0.02);
//    seg.setRadiusLimits (0, 1.1);
//
//    seg.setInputCloud(input);
//    seg.setInputNormals(input_normals);
//
//    // Obtain the cylinder inliers and coefficients
//    seg.segment(*inliers_cylinder, *modelCoeff);
////
////    //pcl::ModelCoefficients::Ptr modelcoef (new pcl::ModelCoefficients);
//
//
    
}
void Skeleton::setHighestVoxel(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_voxels )
{
 //   std::cout<< "setHighestVoxel\n";
    float z =-999999999999999999.0;
    int index;
    for(int q=0; q < cloud_voxels->points.size(); q++)
    {
        if(cloud_voxels->points.at(q).z > z)
        {
            z = cloud_voxels->points.at(q).z;
            index = q;
        }
    }
    m_highestVoxel = index;
}
void Skeleton::setOctree()
{
    m_ocs.setResolution(m_radius);
    m_ocs.setInputCloud (m_treeCloud);
    m_ocs.addPointsFromInputCloud ();
}
void Skeleton::setCluster(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud)
{
    // for all points find lowest point up to distance.
    float distance = 0.3;
    
    float minSize = std::numeric_limits< float>::max();
    // find lowest Z coord
    for (size_t i = 0; i < m_treeCloud->points.size(); i++)
    {
        if (m_treeCloud->points[i].z < minSize && m_usedPoint.at(i) == false)
            minSize = m_treeCloud->points[i].z;
    }
    //  najdi vsechny body ve vzdalenosti 0.+ od nejspodnejsiho bodu
    for (size_t i = 0; i < m_treeCloud->points.size(); i++)
    {
        if (m_treeCloud->points[i].z < (minSize + distance) && m_treeCloud->points[i].z >= minSize && m_usedPoint.at(i) == false)
        {
            cloud->push_back(m_treeCloud->points[i]);
            m_usedPoint.at(i) = true;
        }
    }
    //
}
void Skeleton::setUsedPoints()
{
    m_usedPoint.resize(m_treeCloud->points.size(), false);
}

void Skeleton::coverSets(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_voxel, std::vector<int>& treeSet )
{
    //float radius = .04;
    // set octree with given resolution (4 cm)
  //  pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_voxels (new pcl::PointCloud<pcl::PointXYZI>);
    createVoxels(cloud_voxel);
    m_usedCS.resize(cloud_voxel->points.size(), false);
    
    //small bal -each  coverset(m_inliersSB) have Id of points that belongs to it.
    
    std::vector<int>  coversetIDSB(m_treeCloud->points.size());
    m_inliersSB.resize(cloud_voxel->points.size());
    // small ball
    makeSmallBall(m_ocs, cloud_voxel, m_inliersSB, coversetIDSB, m_radius);
    // set highest voxel and its treeSet
    
    

   // std::cout<< "mala koule hotova\n";
    
    // big ball
    std::vector<std::vector<int>> inliersBB; // size of coversets each coverset contain point ids
    inliersBB.resize(cloud_voxel->points.size());
    std::vector<std::vector<int>> pointCSID(m_treeCloud->points.size()); // size of m_treepoint cloud, each point refers fo which of coverset it belongs

    // big ball
    makeBigBall(m_ocs, cloud_voxel, inliersBB,  m_radius*m_multiplicator);
   // std::cout<< "velka koule hotova\n";
    //set neighbors
    // zjiti sousedy a udeleat vzajemny vztah
    createNeighbors(inliersBB, coversetIDSB);
    
    //
    std::vector<std::vector<int>> treeSets;
    makeTreeSets(treeSets);
 //   std::cout<< "tree sets: "<< treeSets.size() << "\n";
    mergeTreeSets(treeSets, cloud_voxel,treeSet);
    
    
    // segmentation of tree sets into segments
    
   // setConnectionPoint();
    //m_tree->setSegments(branches);
}
void Skeleton::setConnectionPoint()
{
  //  std::cout<< "setConnectionPoint()\n";
    // for given segment find closet point to the parent
    for(int i=0; i< m_segments.size() ; i++)
    {
        // get cloud
        std::vector<int> indices = m_segments.at(i)->getInliers();
        pcl::PointCloud<pcl::PointXYZI>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZI>);
        cloud->points.resize(indices.size());
        //std::cout<< "velikost mracna: " << cloud->points.size() << "\n";
        //if(indices.size() < 4)
        // continue;
        
#pragma omp parallel for
        for(int q=0;q < indices.size();q++)
            cloud->points.at(q) = m_treeCloud->points.at(indices.at(q));
//std::cout<< "cloud of segment\n";
        // get parent cloud
        pcl::PointCloud<pcl::PointXYZI>::Ptr cloudParent (new pcl::PointCloud<pcl::PointXYZI>);
        
        if(m_segments.at(i)->getParent() == m_segments.at(i))
        {
            int indiceLow;
            float height =1000000000000;
            for(int r=0; r < cloud->points.size();r++)
            {
                if(height > cloud->points.at(r).z)
                {
                    height = cloud->points.at(r).z;
                    indiceLow = r;
                }
            }
            cloudParent->points.push_back(cloud->points.at(indiceLow));
        }
        else
        {
            std::vector<int> indicesParent = m_segments.at(i)->getParent()->getInliers();
            cloudParent->points.resize(indicesParent.size());
#pragma omp parallel for
            for(int q=0; q < indicesParent.size(); q++)
                cloudParent->points.at(q) = m_treeCloud->points.at(indicesParent.at(q));
        }

        //std::cout<< "cloud of parent segment\n";
        // search tree
        std::vector<int> pointIDv;
        std::vector<float> pointSDv;
        pcl::KdTreeFLANN<pcl::PointXYZI> k;
        k.setInputCloud (cloudParent);
        // pokud naleznu v dane vzalenosti min 3 body fituju jima kruznici
        int closestIndice;
        float distance = 10000000000;
        //for each point find closest point from parent
        for(int q=0; q < cloud->points.size();q++)
        {
            if(k.nearestKSearch(cloud->points.at(q), 1, pointIDv, pointSDv)>0)
            {
                for(int w=0; w < pointSDv.size();w++)
                {
                    if(distance > std::sqrt(pointSDv.at(w)))
                    {
                        distance = std::sqrt(pointSDv.at(w));
                        closestIndice = q;
                    }
                }
            }
        }
        //save point into segment
       // std::cout<< "save point into segment\n";
        m_segments.at(i)->setConnPoint(cloud->points.at(closestIndice));
    }
}
void Skeleton::createVoxels(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_)
{
    pcl::octree::OctreePointCloud<pcl::PointXYZI> oc (m_radius);
    oc.setInputCloud (m_treeCloud);
    oc.addPointsFromInputCloud ();
    // zjistit vsechny voxely
    std::vector<pcl::PointXYZI, Eigen::aligned_allocator<pcl::PointXYZI> > voxels;
    oc.getOccupiedVoxelCenters(voxels);
    
    cloud_->points.resize(voxels.size());
    
#pragma omp parallel for
    for(int r=0; r < voxels.size(); r++)
    {
        //std::vector<int> indices;
        //m_ocs.voxelSearch(voxels.at(r), indices);
        
        //if(indices.size() > 1)
            cloud_->points.at(r) = voxels.at(r);
    }
    cloud_->width = cloud_->points.size ();
    cloud_->height = 1;
    cloud_->is_dense = true;
}

void Skeleton::makeSmallBall(pcl::octree::OctreePointCloudSearch<pcl::PointXYZI> &octreeSearch, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_, std::vector<std::vector<int>> & inliersInCS, std::vector<int>& pointCSID, float radius)
{
    //radius = std::sqrt(radius*radius + radius*radius);
#pragma omp parallel for
    for(int q=0; q < cloud_->points.size(); q++)
    {
        //std::vector< int > ind;
        Eigen::Vector3f low (cloud_->points.at(q).x - radius/2, cloud_->points.at(q).y - radius/2, cloud_->points.at(q).z - radius/2);
        Eigen::Vector3f high(cloud_->points.at(q).x + radius/2, cloud_->points.at(q).y + radius/2, cloud_->points.at(q).z + radius/2);
        std::vector<int> pointID;
        std::vector<float> pointSD;
        
       //if(octreeSearch.radiusSearch(cloud_->points.at(q), radius, pointID, pointSD) > 0)
        if(octreeSearch.voxelSearch(cloud_->points.at(q), pointID)>0)
          // if(octreeSearch.boxSearch(low, high, pointID) > 0)
        {
            inliersInCS.at(q) = pointID;
            for(int w=0; w < pointID.size();w++)
            {
                pointCSID.at(pointID.at(w)) = q;
            }
        }
    }
}
void Skeleton::makeBigBall(pcl::octree::OctreePointCloudSearch<pcl::PointXYZI> &octreeSearch, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_, std::vector<std::vector<int>> & inliers, float radius)
{
    //radius = std::sqrt(radius*radius + radius*radius);
#pragma omp parallel for
    for(int q=0; q < cloud_->points.size(); q++)
    {
        std::vector<int> pointID;
        std::vector<float> pointSD;
        
        if(octreeSearch.radiusSearch(cloud_->points.at(q), radius, pointID, pointSD) > 0)
            inliers.at (q) = pointID;
    }
}
void Skeleton::createNeighbors(std::vector<std::vector<int>>& inliersBigBall, std::vector<int>& pointsWithCSID)
{
    // pro kazdy cover set  zjisti jeho sousedy z velke koule a dopln je do m_neighbors pod ID coversetu
    m_neighbors.clear();
    m_neighbors.resize(inliersBigBall.size());
    
#pragma omp parallel for
    for(int a=0; a < inliersBigBall.size(); a++) // pro kazdy voxel
    {
        std::vector<int> nei;
        std::vector<int> points = inliersBigBall.at(a); //zjistit inliersBB
        for(int s=0; s < points.size(); s++) // pro kazdy bod ziskat  ID  z inliers BB
        {
            if(pointsWithCSID.at(points.at(s)) != a ) // pokud to neni ten samy voxel
                nei.push_back(pointsWithCSID.at(points.at(s)));
        }
        if(nei.size()> 1)
        {
            std::sort(nei.begin(), nei.end());
            nei.erase(std::unique(nei.begin(), nei.end()), nei.end());
        }
        m_neighbors.at(a) = nei;
    }
 //   std::cout<< "neighbor hotovo\n";
    
    //make symetric relation of neighbors
    
    // for each cover set
    for(int y=0; y < m_neighbors.size(); y++)
    {
        // for each of neighbor
        //get all neighbors CS
        // if it has any neighbor
        for(int s=0; s < m_neighbors.at(y).size(); s++) // for each neighbor CS get ID
        {
            int c =m_neighbors.at(y).at(s) ;// ID of the neighbor
            bool used=false;
            // look for  neighbors in c CS
            for(int x=0; x < m_neighbors.at(c).size(); x++)
            {
                int v = m_neighbors.at(c).at(x); // ID of CS in neighbor(c)
                
                if( v == y)   // if there is a connection
                    used = true;
            }
            // if not connected in both neighbors
            if(used == false) // if in neighbor c is not y add
            {
                if(m_neighbors.at(c).size() ==0)
                {
                    std::vector<int> v;
                    v.push_back(y);
                    m_neighbors.at(c) = v;
                }
                else
                    m_neighbors.at(c).push_back(y);
            }
        }
    }
 //   std::cout<< "neighbor symetric hotovo\n";
}
void Skeleton::makeTreeSets(std::vector<std::vector<int>>& segments)
{
    // tree set  - one whole connected segment- TODO
    std::vector<bool> usedCover (m_neighbors.size(), false);
    int pocet =0;
    for(int q=0; q< m_neighbors.size(); q++) //for each coverset
    {
        // if not used save as new segment
        if(usedCover.at(q) == false)
        {
            std::vector<int> seg;
            seg.push_back(q);
            pocet++;
            usedCover.at(q) =true;
            
            for(int r=0; r < seg.size();r++) // for CS
            {
                int neiID =seg.at(r);       // CS ID
                bool hasUnusedNeighbor = false;
                
                for(int w=0; w < m_neighbors.at(neiID).size();w++)  // for all neighbors
                {
                    if(usedCover.at(m_neighbors.at(neiID).at(w)) == false) //  if not connected to the seg
                    {
                        seg.push_back(m_neighbors.at(neiID).at(w));
                        pocet++;// connect
                        usedCover.at(m_neighbors.at(neiID).at(w)) = true;
                        hasUnusedNeighbor = true;
                    }
                }
            }
            segments.push_back(seg);
        }
    }
    std::sort( segments.begin(), segments.end(), [ ]( const std::vector<int>& lhs, const std::vector<int>& rhs ){return lhs.size() > rhs.size();});
  //  std::cout<< "pocet segmentu: " << segments.size()<< " celkem pocet bodu: "<< pocet <<"\n"; // should be 1 for make whole conneceted tree
}
void Skeleton::mergeTreeSets(std::vector<std::vector<int>>& sets, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud, std::vector<int>& output)
{
    //predelat
    //vybrat ten nejvetsi
    
    //dokud neni jenom jeden set
    // nastavit prohledavaní
    // pro vsechny sety zjsitit minimální vzdalenost k nejvetsimu setu
    // nejblizsi priradit podle nejbližšího voxelu.
    // smazat set
    //opakovat
    //pcl::KdTreeFLANN<pcl::PointXYZI> search;
    pcl::octree::OctreePointCloudSearch<pcl::PointXYZI> search (m_radius);
    float radius = m_radius*m_multiplicator;
    //int mutiplicator =m_multiplicator;
    auto now = std::chrono::system_clock::now();
    time_t cnow = std::chrono::system_clock::to_time_t(now);
    
    int big=0;
    int bigSegmentID =0;
    for(int q=0; q < sets.size();q++)
    {
        if(sets.at(q).size() > big)
        {
            big = sets.at(q).size();
            bigSegmentID = q;
        }
    }
    
    while(sets.size() > 1)
    {
        //mutiplicator++;
        radius+=m_radius;
       // std::cout<< "pocet tree sets: "<<sets.size() << " radius: "<< radius  <<"\n";
        auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
        auto fraction = now - seconds;
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(fraction);
        //std::cout <<"uplynuly cas: " << milliseconds.count() << '\n';
        std::chrono::system_clock::to_time_t(now);
       // start = std::time(nullptr);
        // set octree search
        std::shared_ptr<std::vector<int> > indicesptr (new std::vector<int> (sets.at(bigSegmentID)));
       // std::cout<< "veliksot indices: "<< sets.at(bigSegmentID).size()<<"\n";
        search.setInputCloud (cloud, indicesptr);
        search.addPointsFromInputCloud ();
        
        ///nova verze - radius search
        for(int q=0; q < sets.size(); q++)
        {
            if(q ==bigSegmentID)
                continue;
            
            bool used =false;
            for(int e=0; e < sets.at(q).size(); e++)
            {
                int coverSetID = sets.at(q).at(e);
                std::vector<int> points;
                std::vector<float> dist;
                // pokud jsou v blizkosti body, udelat spojeni pro dany bod
                if(search.radiusSearch(cloud->points.at(coverSetID), radius, points, dist) > 0)
                {
                    used=true;
                    for(int r=0; r< points.size(); r++)
                    {
                        int voxelID1 = coverSetID;
                        int voxelID2 = points.at(r);
                        // set connection
                        m_neighbors.at(voxelID1).push_back(voxelID2);
                        m_neighbors.at(voxelID2).push_back(voxelID1);
                    }
                }
            }
            if(used==true)
            {
                // add point to the biggest set
                sets.at(bigSegmentID).insert(sets.at(bigSegmentID).end(), sets.at(q).begin(),sets.at(q).end());
                // erase sets at id
                sets.erase(sets.begin() + q);
                q--;
                
            }
        }
        
        ///
        
        
        
        
        
//        std::vector<int> voxelID1s(sets.size(),-1);
//        std::vector<int> voxelID2s(sets.size(), -1);
//        std::vector<float> distances(sets.size(), 10000000000);
//
//        for(int w=0; w < sets.size(); w++)
//        {
//            if(w == bigSegmentID)
//                continue;
//
//            float segmentDistance = 10000000;
//            int segmentVoxelID1, segmentVoxelID2;
//            for(int e =0; e < sets.at(w).size();e++)
//            {
//                int coverSetID = sets.at(w).at(e);
//                std::vector<int> points;
//                std::vector<float> dist;
//
//                if(search.nearestKSearch(cloud->points.at(coverSetID), 1, points, dist) >0)
//                {
////                    //save distance and voxels IDs
//                    int voxelID1 = coverSetID;
//                    int voxelID2 = points.at(0);
//                    int distance = std::sqrt(dist.at(0));
//                    if(distance < segmentDistance)
//                    {
//                        segmentDistance = distance;
//                        segmentVoxelID1 = voxelID1;
//                        segmentVoxelID2 = voxelID2;
//                    }
//                }
//           }
//            voxelID1s.at(w) = segmentVoxelID1;
//            voxelID2s.at(w) = segmentVoxelID2;
//            distances.at(w) = segmentDistance;
//        }
//        // find closest set
//        float closestd = 100000000000;
//        int id;
//        for(int z=0;z < sets.size();z++)
//        {
//            if(distances.at(z) < closestd)
//                closestd = distances.at(z);
//            id = z;
//        }
//        // set connection
//        m_neighbors.at(voxelID1s.at(id)).push_back(voxelID2s.at(id));
//        m_neighbors.at(voxelID2s.at(id)).push_back(voxelID1s.at(id));
//        // add point to the biggest set
//        sets.at(bigSegmentID).insert(sets.at(bigSegmentID).end(), sets.at(id).begin(),sets.at(id).end());
//        // erase sets at id
//        sets.erase(sets.begin() + id);
    }
    output = sets.at(0);
}
void Skeleton::Segmentation(std::vector<int> treeSet, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud, std::vector<std::shared_ptr<Segment>>& segments)
{
   // std::cout<< "Segmentation\n";//
    segments.clear();
    
    std::vector<bool> usedCS (treeSet.size(), false);
    // set the first segment as lowest CS of the whole treeset
    //int low = getLowestCS(treeSet, cloud); // get lowest ID
    std::vector<int> start;
    start.push_back(getLowestCS(treeSet, cloud));
    segments.push_back(std::shared_ptr<Segment>(new Segment(start, 0)));
    segments.at(0)->setParent(segments.at(0));
    segments.at(0)->setConnPoint(cloud->points.at(start.at(0)));

    for(int a=0; a < segments.size(); a++) // for each segment
    {
        //std::cout<< "pocet segmentu: " << segments.size()<<"\n";
        //int parentID = components.at(a)->getcu;
        std::vector<int> segmentCSs = segments.at(a)->getCoverSets(); // get all CS of the segment
        segments.at(a)->setCurrID(a); // set segment ID
        segments.at(a)->setOrder(-1); // set segment order
        
        while(segmentCSs.size()>0)
        {

                     // nalezene  CS nastavit jako použité
            for(int j=0; j < segmentCSs.size(); j++)
                m_usedCS.at(segmentCSs.at(j)) = true;
                    // definovat vektor vektoru pro casti
            std::vector<std::vector<int>> parts;
                   // std::cout<< "define components\n";
            if(defineComponents(segmentCSs, parts)> 0) //pokud je pocet častí vetší než nula
            {
                //std::cout<< "defineComponents: pocet parts: " << parts.size()<<"\n";
                if(parts.size()== 1) // only points of segment
                {
                    segmentCSs= parts.at(0); // definuj novy body pro hledani
                   // std::cout<< "velikost segmentCSs: " << segmentCSs.size()<< "\n";
                    segments.at(a)->addCoverSet(parts.at(0));
                    //std::cout<< "pridany coverSets\n";
                    for(int j=0; j < parts.at(0).size(); j++)
                        m_usedCS.at(parts.at(0).at(j)) = true;
                    //std::cout<< "nastaveno m_usedCS\n";
                }
                else // pokud jejich víc než jeden
                {
                            // two or more part - for each part make new segment
                    for(int i=0; i < parts.size(); i++)
                    {
                        for(int j=0; j < parts.at(i).size(); j++)
                            m_usedCS.at(parts.at(i).at(j)) = true;
                                // save new component
                                //TODO check if it has usable
                        int cur =segments.size(); // id of the new component
                        segments.push_back(std::shared_ptr<Segment>(new Segment(parts.at(i),cur )));
                        segments.at(a)->setChild(segments.at(cur)); // set child for current component
                                 // make new component
                        segments.at(cur)->setParent(segments.at(a)); // set parten for new component
                        segments.at(cur)->setConnPoint(cloud->points.at(parts.at(i).at(0)));
                    }
                    segmentCSs.clear(); // nejsou dalsi body k prohledani
                }//if parts size == 1 or more
            } // if found some points in define component
            else // nenalezeny  zadne pokracovani definecomponents
            {
               // std::cout<< "neni pokracovani\n";
                segmentCSs.clear();
            }
        } //end while
    } //end component
}
void Skeleton::correctSegments(std::vector<std::shared_ptr<Segment>>& input)
{
  //  std::cout<< "correctSegments\n";
  //  std::cout<< "veliksot input: " << input.size()<< "\n";

    std::vector<bool> usedCS (m_neighbors.size(), false);

    // pro kazdy output
    for(int a=0; a < input.size(); a++) // pro kazdou komponentu
    {
       // std::cout<< "komponenta " << a << " pocet cs: " << input.at(a) ->getCoverSets().size() << "\n";
        // set minimal size of component - at least has m_minCoversetSize coverset size
        /// if not add children and coversets to the parent
        // delete
        if(input.at(a) ->getCoverSets().size() < m_minCoversetSize )
        {
           // get children
            std::vector<std::shared_ptr<Segment>> children;
            input.at(a)->getChildrens(children);
            
            // set new parent to the children
            for (int v=0; v < children.size(); v++)
                children.at(v)->setParent(input.at(a)->getParent());
            
           // set new children to the parent
            std::vector<std::shared_ptr<Segment>> childrenP;
            input.at(a)->getParent()->getChildrens(childrenP);
            // add to the old children
            for (int m=0; m < childrenP.size(); m++)
            {
                if(childrenP.at(m) != input.at(a))
                    children.push_back(childrenP.at(m));
            }
            input.at(a)->getParent()->setChildrens(children);
            //set CS to the parent
            std::vector<int> cs = input.at(a)->getCoverSets();
            input.at(a)->getParent()->addCoverSet(cs);
            
            // remove
            input.erase(input.begin() + a);
            a=0;
            continue;
        }
        
//        // if no children and has only one coverset
//        if(input.at(a)->getLenght() == 0 && childrens.size() == 0  )   // only one cover set without children or
//        {
//            // get parent
//            // join cs from  input.at(a) to parent
//            // remove children from parent
//            // remove segment from input
//
//                std::vector<int> coverSetsCH = input.at(a)->getCoverSets();
//                input.at(a)->getParent()->addCoverSet(coverSetsCH);
//
//            // remove children from parent
//            std::vector<std::shared_ptr<Segment>> ch;
//            std::vector<std::shared_ptr<Segment>> ch_new;
//            input.at(a)->getParent()->getChildrens(ch);
//            for (int m=0; m < ch.size(); m++)
//            {
//                if(ch.at(m) != input.at(a))
//                    ch_new.push_back(ch.at(m));
//            }
//            input.at(a)->getParent()->setChildrens(ch_new);
//            input.erase(input.begin() + a);
//            a=0;
//        }
        else
        {
            input.at(a)->setCurrID(a);
            input.at(a)->setOrder(-1);
        }
    }
   // std::cout<< "veliksot input po korekci: " << input.size()<< "\n";
   //std::cout<< "veliksot output: " << output.size()<< "\n";
}
void Skeleton::setStem(std::vector<std::shared_ptr<Segment>>& components)
{
 //   std::cout<< "Set Stem\n";
    // pro kazdy segment ktery nema zadne dalsi deti
    // zjisti nejkratsi cestu k prvnimu componentu
    // vyber tu nejdelsi z nich
    float dist = 0;
    int order = 0;
    std::vector<int>stemIDs;
    m_usedSegment.resize (components.size(), false);
    
    for(int q=0; q < components.size(); q++)
    {
        if(components.at(q)->getChildsSize() > 0) // only ends
            continue;
        
        std::vector<int> stemIDs_tmp;
        float d = getShortestPath(components.at(q), components.at(0), components, stemIDs_tmp);
        if(  d > dist)
        {
            dist = d;
            stemIDs = stemIDs_tmp;
        }
    }
 //   std::cout<<"velikost stem: " << dist << "\n";
    
    for(int q=0; q < stemIDs.size(); q++)
    {
        m_usedSegment.at(stemIDs.at(q)) = true;
        components.at(stemIDs.at(q))->setOrder(order);
        components.at(stemIDs.at(q))->setBranchID(order);
    }
  //  std::cout<<"stem hotovo\n";
    
}
void Skeleton::setStemTop(std::vector<std::shared_ptr<Segment>>& components)
{
    // set as stem connection of top voxel with base.
    
   // std::cout<< "Set StemTop\n";
    // jdni nejvyssi voxel?
    //pro kazdy voxel zjisti z najdi s nejvyssi hodnotou
    // zjisti ve ktere componenete je.
    //spojit tuto komponentu s patou
    //float dist = 0;
    int order = 0;
    std::vector<int>stemIDs;
    bool find = false;
    m_usedSegment.resize (components.size(), false);
    
    for(int q=0; q < components.size(); q++)
    {
        if(find == true)
            break;
        std::vector<int> segmentCSs = components.at(q)->getCoverSets(); // get all CS of the segment
        for(int w=0; w < segmentCSs.size(); w++)
        {
            if( segmentCSs.at(w) == m_highestVoxel) // mame nejvyssi voxel a take jeho komponentu
            {
                //std::cout<<"nejvyssi voxel\n";
                // spočitat a nastavit jako knem
                std::vector<int> stemIDs;
                float d = getShortestPath(components.at(q), components.at(0), components, stemIDs);
               // std::cout<<"velikost stem: " << d << "\n";
                
                for(int q=0; q < stemIDs.size(); q++)
                {
                    m_usedSegment.at(stemIDs.at(q)) = true;
                    components.at(stemIDs.at(q))->setOrder(order);
                    components.at(stemIDs.at(q))->setBranchID(order);
                }
                find = true;
            }
            if(find == true)
                break;
        }
    }
}
void Skeleton::setBranches(std::vector<std::shared_ptr<Segment>>& components)
{
   // std::cout<< "Set Branches\n";
    // for each components
    //pokud neni pouzita
    // zjisit children
    // pokud ma children stejne branchID
    // spojit
    // pridat cihldern pro spojovany
    //if has no childen
    // if order is -1
    int branchID =100;
    std::vector<int> unsuedComponents;
    for(int q=0; q < components.size(); q++)
    {
        //std::cout<<"componenta:  " << q<< "\n";
        //pokud je použita
        if(m_usedSegment.at(q) == true)
        {
            // get order
            int order = components.at(q)->getOrder();
            int branchOrder = order + 1;
            
            //get children
            std::vector<std::shared_ptr<Segment>> childrens;
            components.at(q)->getChildrens(childrens);
           // std::cout<< "deti ";
            for(int w=0; w < childrens.size();w++)
            {
               // std::cout  << childrens.at(w)->getCurrID() << " ";
                if(childrens.at(w)->getOrder() != -1)
                    continue;
                //compute distance
                std::vector<int> usedSegments;
                float distance = getChildrenMaxLenght(components, childrens.at(w)->getCurrID(), usedSegments);
               // std::cout<<"delka vetve: " <<  distance << " componenta:  " << q<< "slozena z: ";
                // for each component of branch
                for(int e=0; e < usedSegments.size(); e++)
                {
                    //std::cout<< usedSegments.at(e) << " ";
                    // set order
                    components.at(usedSegments.at(e))->setOrder(branchOrder);
                    // set branchID
                    components.at(usedSegments.at(e))->setBranchID(branchID);
                    // set used components
                    m_usedSegment.at(usedSegments.at(e)) = true;
                }
               // std::cout<<"\n";
                branchID++;
            }// end for childrens
           // std::cout<<"\n";
        }// end if
        else
        {
            int parentID = components.at(q)->getParent()->getCurrID();
           // std::cout<< "componenta " << q << " ma rodice " << parentID << "\n";
            
        }
        //if(unsuedComponents.size()>0 && q == components.size()-2)
          // q = 0;
    }
    

    
    

//   // int branchID=1;
//    std::vector<int> usedSegmentsMax;
//  //  int order = 0;
//    for(int a=0; a< components.size(); a++)
//    {
//        // if component is not used
//        if(components.at(a)->getOrder()!=-1)
//        {
//            //get children
//            std::vector<std::shared_ptr<Segment>> childrens;
//            components.at(a)->getChildrens(childrens);
//            int parentOrder = components.at(a)->getOrder();
//            int currOrder = parentOrder+1;
//
//            for(int w=0; w < childrens.size();w++)
//            {
//                // if children is not used
//                if(m_usedSegment.at(childrens.at(w)->getCurrID()) == true)
//                    continue;
//                //compute distance
//                std::vector<int> usedSegments;
//                float distance = getChildrenMaxLenght(components, childrens.at(w)->getCurrID(), usedSegments);
//
//                // set as used
//                // set order
//                for(int q=0; q < usedSegments.size();q++)
//                {
//                    m_usedSegment.at(usedSegments.at(q)) = true;
//                    components.at(usedSegments.at(q))->setOrder(currOrder);
//                    components.at(usedSegments.at(q))->setBranchID(branchID);
//                }
//                branchID++;
//            }
//        }
//    }
}
float Skeleton::getChildrenMaxLenght(std::vector<std::shared_ptr<Segment>>& segment, int startID, std::vector<int> & usedComponent)
{
    usedComponent.push_back(startID);
    float lenght = segment.at(startID)->getLenght();
    std::vector<std::shared_ptr<Segment>> children;
    segment.at(startID)->getChildrens(children);
    // if segment has no children
    if(children.size() == 0)
        return lenght;
    
    float lenght_local = 0;
    std::vector<int> usedComps_local;
    
    for(int q=0; q < children.size(); q++)
    {
        int currID = children.at(q)->getCurrID();
        std::vector<int> usedComps;
        float dist = getChildrenMaxLenght(segment, currID, usedComps);
        if(dist > lenght_local)
        {
            lenght_local = dist;
            usedComps_local = usedComps;
        }
    }
    usedComponent.insert(usedComponent.end(), usedComps_local.begin(), usedComps_local.end());
    // merge lenght
    lenght += lenght_local;
    return lenght;

}
void Skeleton::mergeSegmentsIntoBranches(std::vector<std::shared_ptr<Segment>>& components, std::vector<std::shared_ptr<Segment>>& branches)
{
    
    // pro kazdou components
    // zjisti deti
    // pro kazde dite
    // pokud ma stejne branch ID
        // spojit do jednoho segmentu
        // spojit CS
        // spojit inliers
        // spojit deti - v tomto pripade se budou opakovat - opravit
    
    // nebo zalozit novou vetev
        // nastavit rodic
        // nastavit řád vetve
        // nastavit currID
        // nastavit jako dite do branches
    
   // std::cout<< "mergeSegmentsIntoBranches\n";
    branches.clear();
    branches.push_back(components.at(0));
    
    for(int i=0; i < branches.size(); i++)
    {
        // children
        std::vector<std::shared_ptr<Segment>> childs;
        branches.at(i)->getChildrens(childs);
        //std::cout<<"rodic vetve: " << i <<" " << branches.at(i)->getParent()<< "\n";
        //std::cout<< "branches ID " << i;
        branches.at(i)->removeChildrens();
        for(int q=0; q < childs.size(); q++)
        {
            //std::cout<<" ID ditete: " << childs.at(q)->getCurrID()<< " ";
            
            if(childs.at(q)->getBranchID() == branches.at(i)->getBranchID())
            {
                //std::cout<<"OK";
                // merge cs
                branches.at(i)->addCoverSet(childs.at(q)->getCoverSets());
                // merge inliers
                branches.at(i)->addInliers(childs.at(q)->getInliers());
                // get childs2
                std::vector<std::shared_ptr<Segment>> childs2;
                childs.at(q)->getChildrens(childs2);
                childs.insert(childs.end(), childs2.begin(), childs2.end());
            }
            else
            {
                //std::cout<<" X ";
                // set new  branch
                branches.push_back(components.at(childs.at(q)->getCurrID()));
                // set parent
                branches.at(branches.size()-1)->setParent(branches.at(i));
                //set order
                branches.at(branches.size()-1)->setOrder(branches.at(i)->getOrder()+1);
                //set currID
                branches.at(branches.size()-1)->setCurrID(branches.size()-1);
                // branch set children
                branches.at(i)->addChildren(branches.at(branches.size()-1));
               // std::cout<<" pro branch "<< i << " zapsano dite " << branches.size()-1<< "\n";
            }
        }
        //std::cout<<"\n";
    }
    // nastavit spravne děti
    // kontrola jetli neukazuje mimo
   // std::cout<<"veliksot branches: " << branches.size()<< "\n";
}
void Skeleton::setIntensity(std::vector<std::shared_ptr<Segment>>& branches, std::vector<std::vector<int>> inliers)
{
  //  std::cout<< "SetIntensity\n";
    int pocet_bodu=0;
    std::vector<bool> usedCS (inliers.size(), false);
        // pro kazdou vetev zjisti vsechny CS a jejich body
    std::srand(std::time(nullptr));
    for(int q=0; q < branches.size();q++)
    {
        int pocet = 0;
        int d =std::rand()%1000;
        std::vector<int> cs = branches.at(q)->getCoverSets();

        std::vector<int> in;
        std::sort(in.begin(), in.end());
         in.erase(std::unique(in.begin(), in.end()), in.end());
        
        for(int w=0;w< cs.size();w++)
        {
            int csID = cs.at(w);
            for(int e=0; e < inliers.at(csID).size(); e++)
            {
                int pointID = inliers.at(csID).at(e);
                in.push_back(pointID);
                m_treeCloud->points.at(pointID).intensity = branches.at(q)->getOrder();
                //m_treeCloud->points.at(pointID).intensity = d;
            }
        }
       // std::cout<< "pocet zdvojenych voxelu: " << pocet <<"  z celkem CS v branches: " << cs.size()<<  "\n";
       // std::sort(in.begin(), in.end());
       // in.erase(std::unique(in.begin(), in.end()), in.end());
        branches.at(q)->setInliers(in);
        pocet_bodu+= in.size();
    }
  //  std::cout<<" pocet bodu prirazeno celkem " << pocet_bodu<< "\n";
}
void Skeleton::computeDistances(std::vector<std::shared_ptr<Segment>>& branches, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_)
{
  //  std::cout<< "computeDistances\n";
    for(int h=0; h < branches.size(); h++)
        branches.at(h)->computeLenght(cloud_);
}
float Skeleton::getSegmentLenght(std::vector<std::shared_ptr<Segment>>& segment, int startID, int order, std::vector<int> & usedComponent)
{
    //
    std::shared_ptr<Segment> p = segment.at(startID);
    float lenght = 0;//p->getLenght();
    while ( p->getOrder() !=order )
    {
        lenght +=  p->getLenght();
        usedComponent.push_back(p->getCurrID());
        // get parent
        p = p->getParent();
    }
    return lenght;
    
    
    
    
    
    usedComponent.push_back(startID);
   // float lenght = segment.at(startID)->getLenght();
    for (int q=0; q < usedComponent.size(); q++)
    {
        std::shared_ptr<Segment> p =segment.at(usedComponent.at(q))->getParent();
        //std::cout<< "segmentID: " << segment.at(usedComponent.at(q))->getCurrID()<< " parentID:" <<segment.at(usedComponent.at(q))->getParent()->getCurrID() <<"\n";
        int parentID = p->getCurrID();
        int parentOrder =p->getOrder();
        if(parentOrder >= order || parentID ==usedComponent.at(q) || m_usedSegment.at(usedComponent.at(q)) == true)
            break;
        
        lenght+= segment.at(parentID)->getLenght();
        usedComponent.push_back(parentID);
    }
    return lenght;
}
float Skeleton::getShortestPath(std::shared_ptr<Segment> start, std::shared_ptr<Segment> end, std::vector<std::shared_ptr<Segment>>& segment, std::vector<int>& segmentIDs)
{
    //segmentIDs.push_back(start->getCurrID());
    if(start == end)
        return 0;
    std::shared_ptr<Segment> p = start;
    
   // std::cout<<"start: "<<start<< " end: " << end <<"\n";
    float lenght = 0;//p->getLenght();
    while ( p != end)
    {
       // std::cout<<"p: " << p<< "\n";
        lenght +=  p->getLenght();
        segmentIDs.push_back(p->getCurrID());
        // get parent
        p = p->getParent();
    }
    
    lenght +=  end->getLenght();
    segmentIDs.push_back(end->getCurrID());
    return lenght;
}

void Skeleton::getNeighbors(std::vector<int>& component, std::vector<int>& cut )
{
    for(int q=0; q < component.size(); q++)
    {
        cut.insert(cut.end(),m_neighbors.at(component.at(q)).begin(), m_neighbors.at(component.at(q)).end());
    }
    // remove duplicate numbers
    if(cut.size()> 1)
    {
        std::sort(cut.begin(), cut.end());
        cut.erase(std::unique(cut.begin(), cut.end()), cut.end());
    }
    
}
int Skeleton::defineComponents(std::vector<int>& seedCS, std::vector<std::vector<int>>& outputParts)
{
    // zjistit jestli spolu sousedi CS v cut a pripadne ktere, pro dalsi to same a ulozit

    // pro cut zjistit pouzitelne sousedy -N1
    
    //pro N1 zjsiti sousedy - N2
    
    // pro kazdy N1 zjisit jestli je v sousedstvi dalsi bod z n1 pokud ano spojit do jednnoho partu
   // std::cout<< "velikost seeds: " << seedCS.size() << "\n";
    std::vector<int> nei1;
    std::vector<int> n1;
    getNeighbors(seedCS, nei1); // zjistit sousedy
    getValidCS(nei1, n1);
    std::vector<bool> usedCS ( n1.size(), false);
   // std::cout<< "n1: ";
   // for(int b=0; b < n1.size();b++) // pro vsechny sousedy
    //    std::cout<< n1.at(b) << " ";
   // std::cout<< "\n ";
    for(int a=0; a < n1.size();a++) // pro vsechny sousedy
    {
        if(usedCS.at(a) == true)
            continue;
        std::vector<int> part;
        part.push_back(n1.at(a));
        usedCS.at(a) = true;
        
        
        for(int q=0; q < part.size(); q++) // pro kazdy bod najdi dasli z n1 ktery je s nim spojeny
        {
            //std::cout<< "ID hledane " << part.at(q)<< "\n";
            // zjisti sousedy pro dany cs
            std::vector<int> singleCS;
            singleCS.push_back(part.at(q));
            std::vector<int> nei2;
            std::vector<int> n2;
            getNeighbors(singleCS, nei2); // zjistit sousedy
            getValidCS(nei2, n2);
         //   std::cout<< "n2: ";
            for(int w=0; w < n2.size(); w++)
            {
               // std::cout<< n2.at(w) << " ";
                
                // get csID
                int csIDn2 = n2.at(w);
                for(int e=0; e < n1.size(); e++)
                {
                    if(usedCS.at(e) == true)
                        continue;
                    int csIDn1 = n1.at(e);
                   // std::cout<< "csIDn1: " << csIDn1 << " csIDn2: " << csIDn2 << " used csIDn1: " << usedCS.at(e) <<"\n";
                    if(csIDn1 == csIDn2) // společny soused
                    {
                        part.push_back(csIDn1);
                        usedCS.at(e) = true;
                    }
                }
            }
            //std::cout<< "\n ";
        }
        outputParts.push_back(part);
    }
   // std::cout<< "veliksot outputParts: " << outputParts.size() << "\n";
    if(outputParts.size() > 1)
        std::sort( outputParts.begin(), outputParts.end(), [ ]( const std::vector<int>& lhs, const std::vector<int>& rhs ){return lhs.size() > rhs.size();});
    return outputParts.size();
}
void Skeleton::getValidCS(std::vector<int>& input,std::vector<int>& output)
{
    for(int q=0; q < input.size(); q++)
    {
        if(m_usedCS.at(input.at(q)) == false)
            output.push_back(input.at(q));
    }
}
int Skeleton::getLowestCS(std::vector<int>& segment, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_)
{
    float height = 100000000;
    int ID;
    for(int q=0; q< segment.size(); q++)
    {
        // get CS ID
        int csID = segment.at(q);
        //get z of the csID
        float h = cloud_->points.at(csID).z;
        if(h < height)
        {
            height = h;
            ID = csID;
        }
    }
    return ID;
}
int Skeleton::getHighestCS(std::vector<int>& segment, pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_)
{
    float height = -100000000;
    int ID;
    for(int q=0; q< segment.size(); q++)
    {
        // get CS ID
        int csID = segment.at(q);
        //get z of the csID
        float h = cloud_->points.at(csID).z;
        if(h > height)
        {
            height = h;
            ID = csID;
        }
    }
    return ID;
}


float Skeleton::gFunction(pcl::PointCloud<pcl::PointXYZI>::Ptr input, Eigen::Vector3f axis , Eigen::Vector3f& PC, float& rSqr)
{
    //Eigen::Matrix3f I = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f P = Eigen::Matrix3f::Identity() - (axis * axis.transpose());
    Eigen::Matrix3f S;
    int n = input->points.size();
    S << 0, -1*axis[2], axis.y(), axis.z(), 0, -1*axis.x(), -1*axis.y(), axis.x(), 0;
    Eigen::Matrix3f A = Eigen::Matrix3f::Zero();
    Eigen::Vector3f B = Eigen::Vector3f::Zero();
    std::vector<Eigen::Vector3f> Y; // nevim jestli je vlastne potreba
    std::vector<float> sqrLength;
    float averageSqrLength = 0;
    
    for (int i = 0; i < n; i++)
    {
        Eigen::Vector3f point;
        point << input->points.at(i).x, input->points.at(i).y, input->points.at(i).z;
        Y.push_back(P*point);
        sqrLength.push_back(Y.at(i).dot(Y.at(i)));
        A += Y.at(i) * Y.at(i).transpose(); //  OuterProduct(Y[ i ] , Y[ i ]);
        B = B + (sqrLength.at(i) * Y.at(i));
        averageSqrLength += sqrLength.at(i);
    }
    A /= n; // To keep elements small when n is large.
    B /= n; // To keep elements small when n is large.
    averageSqrLength /= n;
    Eigen::Matrix3f Ahat = (-1 * S) * A * S;
    Eigen::Matrix3f Trace= Ahat * A;
    PC = (Ahat * B)/Trace.trace();
    float error = 0;
    
    rSqr = 0;
    for (int i = 0; i < n; i++)
    {
        float term = sqrLength.at(i) - averageSqrLength - 2 * PC.dot(Y.at(i));
        error += term * term;
        Eigen::Vector3f diff =PC - Y.at(i);
        rSqr+=diff.dot(diff);
    }
    error /= n; // For root−mean square error .
    rSqr /= n;
    return error ;
}
void Skeleton::createCylinder(std::vector<std::shared_ptr<Segment>>& segment, std::vector<std::vector<int>> inliers)
{
    // for given segment compute cylinders coef and save result into  output.
    
    // urcit hlavni smer vetve (segmentu)
    // od zacatku vzit cca 10 cm ve smeru vetve
    // nafittovat valec
    // zjistit odchylky od valce
    // ulozit valec a pokracovat dal.
    
    
    // get points of segment
    // compute cylinder for points
    std::cout<<"createCylinder\n";

    for(int q=0; q < segment.size() ;q++)//
    {
       // std::cout<<"segment cislo: " << q << "\n";
        
         // find cloud
        std::vector<int> cs = segment.at(q)->getCoverSets();
        pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_segment (new pcl::PointCloud<pcl::PointXYZI>);
        std::vector<int> points;
        for(int w=0;w< cs.size();w++)
        {
            int csID = cs.at(w);
            for(int e=0; e < inliers.at(csID).size(); e++)
            {
                int pointID = inliers.at(csID).at(e);
                points.push_back(pointID);
                cloud_segment->points.push_back(m_treeCloud->points.at(pointID));
            }
        }
        if(cloud_segment->points.size()< 5)
            continue;
        ///make pca
        pcl::PointCloud<pcl::PointXYZI> cloud_ ;
        pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_translated (new pcl::PointCloud<pcl::PointXYZI>);
        pcl::PCA<pcl::PointXYZI> pca;
        pca.setInputCloud(cloud_segment);
        pca.project(*cloud_segment, *cloud_translated);
        //compute median and shift cloud
        
//        Eigen::Vector3f average = computeMedian(cloud_segment);
//
//        for (int i = 0; i < cloud_segment->points.size(); i++)
//        {
//            pcl::PointXYZI bod;
//            bod.x = cloud_segment->points.at(i).x - average.x();
//            bod.y = cloud_segment->points.at(i).y - average.y();
//            bod.z = cloud_segment->points.at(i).z - average.z();
//            cloud_translated->points.push_back(bod);
//        }
            Eigen::Vector3f W = getInitialCylinderDir(cloud_translated);
        //std::cout << "axis: "<< axis.x()<< " "<< axis.y()<< " "<< axis.z()<< "\n ";
        Eigen::Vector3f C;
        //float rr;
        float rSqr;
        float er = gFunction(cloud_translated, W, C, rSqr);
       // std::cout << "center: "<<cC.x()<< " "<< cC.y()<< " "<< cC.z()<< "\n ";
       // std::cout << "INTIAL radius: "<<std::sqrt(rSqr)<<" error: " <<  er <<"\n ";
        
        if(er >  5.15 || std::sqrt(rSqr) > 5 )
        {
            // Choose imax and jmax as desired
            // want for sampling W vectors on the hemisphere .
            float  minError = 1000000000000000000;
            W = Eigen::Vector3f::Zero();
            C = Eigen::Vector3f::Zero();
            rSqr = 0;
            
            int jmax = 6;
            int imax =jmax;
            for (int j = 0; j <= jmax; j++)
            {
                //std::cout<<"j: " << j << "\n";
                
                float phi = M_PI/2 * j / jmax; // in [0,pi/2]
                float csphi = cos(phi);
                float snphi = sin(phi);
                for (int i = 0; i < imax; i++)
                {
                    float theta = M_PI * i / imax; // in [0,2∗pi)
                    float cstheta = std::cos(theta);
                    float sntheta = std::sin(theta);
                    
                    Eigen::Vector3f currentW (cstheta * snphi , sntheta * snphi , csphi );
                    Eigen::Vector3f currentC;
                    float currentRSqr;
                    float error = gFunction(cloud_translated, currentW, currentC, currentRSqr);
                    
                    if (error < minError && std::sqrt(currentRSqr) < 5)
                    {
                        minError = error ;
                        W = currentW ;
                        C = currentC;
                        rSqr = currentRSqr ;
                        std::cout << "radius: "<<std::sqrt(rSqr)<<" error: " <<  minError <<"\n ";
                    }
                }
            }
        }
//
//        Eigen::Vector3f center;
//        float squaredRadius;
//        float  error = gFunction(cloud_translated, axis, center, squaredRadius);
        
        
        pcl::PointXYZI proj_min,proj_max, minp, maxp, proj_minp, proj_maxp;
        pcl::getMinMax3D (*cloud_translated, proj_min, proj_max);
        
        proj_minp.x =proj_min.x;
        proj_minp.y =(proj_max.y + proj_min.y)/2;
        proj_minp.z =(proj_max.z + proj_min.z)/2;
        
        proj_maxp.x =proj_max.x;
        proj_maxp.y =(proj_max.y + proj_min.y)/2;
        proj_maxp.z =(proj_max.z + proj_min.z)/2;
        
        pca.reconstruct (proj_minp, minp);
        pca.reconstruct (proj_maxp, maxp);
        Eigen::Vector3f vect = Eigen::Vector3f(maxp.x - minp.x,maxp.y - minp.y,maxp.z - minp.z);
        
        
        
//
//
        pcl::PointXYZI pointC, pC, pointW, pW;
        
        
        pointC.x =C.x();
        pointC.y =C.y();
        pointC.z =C.z();
        
        pointW.x =C.x() + W.x();
        pointW.y =C.y() + W.y();
        pointW.z =C.z() + W.z();
       // Eigen::Vector3f vect = Eigen::Vector3f(maxp.x - minp.x,maxp.y - minp.y,maxp.z - minp.z);

        
        pca.reconstruct (pointC, pC);
        pca.reconstruct (pointW, pW);
        float radius= std::sqrt(rSqr);
        pcl::ModelCoefficients::Ptr coeff (new pcl::ModelCoefficients);
        
        //sampleConsensus(points, coeff);
        // fit cylinder
        
        coeff->values.push_back(minp.x);
        coeff->values.push_back(minp.y);
        coeff->values.push_back(minp.z);
        coeff->values.push_back(vect.x());
        coeff->values.push_back(vect.y());
        coeff->values.push_back(vect.z());
        coeff->values.push_back(radius);
        
        //m_tree->setCylinder(coeff);
        m_cylinders.push_back(coeff);
    }
}
Eigen::Vector3f Skeleton::computeMedian(pcl::PointCloud<pcl::PointXYZI>::Ptr cloud_segment)
{
    Eigen::Vector4f xyzi;
    pcl::compute3DCentroid (*cloud_segment, xyzi);
    Eigen::Vector3f xyz;
    xyz << xyzi.x(), xyzi.y(),xyzi.z();
    return xyz;
}
void Skeleton::getHalves(pcl::PointCloud<pcl::PointXYZI>::Ptr input,pcl::PointCloud<pcl::PointXYZI>::Ptr bottom,pcl::PointCloud<pcl::PointXYZI>::Ptr high)
{
    Eigen::Vector3f median = computeMedian(input);
    for(int i=0; i < input->points.size(); i++)
    {
        if(input->points.at(i).z < median.z())
            bottom->points.push_back(input->points.at(i));
        else
            high->points.push_back(input->points.at(i));
    }
}
Eigen::Vector3f Skeleton::getInitialCylinderDir(pcl::PointCloud<pcl::PointXYZI>::Ptr input)
{
    pcl::PointCloud<pcl::PointXYZI>::Ptr bottom (new pcl::PointCloud<pcl::PointXYZI>);
    pcl::PointCloud<pcl::PointXYZI>::Ptr high (new pcl::PointCloud<pcl::PointXYZI>);
    
    getHalves(input, bottom, high);
    Eigen::Vector3f b = computeMedian(bottom);
    Eigen::Vector3f h = computeMedian(high);
    Eigen::Vector3f axis;
    axis.x() = b.x() - h.x();
    axis.y() = b.y() - h.y();
    axis.z() = b.z() - h.z();
    return axis;
}
float Skeleton::getSquaredDistance(pcl::PointCloud<pcl::PointXYZI>::Ptr input, Eigen::Vector3f axis , float radius, Eigen::Vector3f median)
{
    float dist=0;
    for(int i=0; i < input->points.size(); i++)
    {
        float d = getDistance(axis, median, radius, input->points.at(i));
        dist+= d;
    }
    return dist/input->points.size();
}
float  Skeleton::getDistance(Eigen::Vector3f axis , Eigen::Vector3f median, float radius, pcl::PointXYZI point)
{
    // obecna rovnice roviny = point.x * cylinder->values.at(3) + point.y * cylinder->values.at(4) - point.z * cylinder->values.at(5) + d = 0
    float d = -point.x * axis.x() - point.y * axis.y() - point.z * axis.z();
    // dosadit parametrickou primku do roviny
    float delenec =  -d - axis.x() * median.x() - axis.y() * median.y() - axis.z() * median.z();
    
    float delitel = ( axis.x() *  axis.x()) + (axis.y() * axis.y()) + (axis.z() * axis.z());
  //  std::cout<< "delenec: " << delenec << " delitel: " << delitel <<"\n";
    if (delitel == 0)
        return 10000000;
    
    float t = delenec/delitel;
    // SOURADNICE SPOLECNEHO BODU
    float x = median.x() + t * axis.x();
    float y = median.y() + t * axis.y();
    float z = median.z() + t * axis.z();
   // std::cout<< "bod Y.x: " << x << " bod Y.y: " << y <<" bod Y.z: " << z<<"\n";
    // distance to model
    float distance = std::sqrt( (x-point.x)*(x-point.x) + (y-point.y)*(y-point.y) + (z-point.z)*(z-point.z) - radius);
    return distance;
    
}
void Skeleton::correctChildrens(std::vector<std::shared_ptr<Segment>>& branches)
{
   //  std::cout<< "correctChildrens\n";
    // pro kazdy branches
    // zjisti children currID
    // ulozit do vectoru, odstrnit duplicity, mensi nez currid
    // vymenit children
    for(int q=0; q < branches.size();q++)
    {
        std::vector<int> cIDs;
        std::vector<std::shared_ptr<Segment>> childs;
        branches.at(q)->getChildrens(childs);
        for(int w=0; w < childs.size(); w++)
        {
            cIDs.push_back(childs.at(w)->getCurrID());
            std::cout<< "pro vetev: " << q <<" currID childs " <<childs.at(w)->getCurrID()<<"\n";
        }
        // sort and remove duplicity
        if(cIDs.size()> 1)
        {
            std::sort(cIDs.begin(), cIDs.end());
            cIDs.erase(std::unique(cIDs.begin(), cIDs.end()), cIDs.end());
            std::vector<std::shared_ptr<Segment>> new_childs;
            std::cout<<"nove deti: ";
            for(int e=0; e < cIDs.size(); e++)
            {
                new_childs.push_back(branches.at(cIDs.at(e)));
                std::cout<<cIDs.at(e) << " ";
            }
            std::cout<<"\n";
            branches.at(q)->setChildrens(new_childs);
        }
    }
}

}
