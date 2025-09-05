//
//  sortiment.h
//  3DForest043
//
//  Created by Jan Trochta on 13.08.18.
//

#ifndef sortiment_h
#define sortiment_h

#include <pcl/common/common_headers.h>
#include <pcl/ModelCoefficients.h>

namespace f3d
{

class Sortiment
{
    // class for store data about sortiment
    // contains vectors of cylinder with information about each cylinder and its quality
public:
    Sortiment();
    Sortiment(std::vector<pcl::ModelCoefficientsPtr> cylinders, int curve, int bb, int taper, int skew, int grade);
    ~Sortiment();
    //sets
    void setCylinders(std::vector<pcl::ModelCoefficientsPtr> cylinders);
    void setQualityGrade(int i);
    void setCurvatureGrade(int i);
    void setSkewnessGrade(int i);
    void setBranchBifurcationGrade(int i);
    void setTaperGrade(int i);
    //gets
    std::vector<pcl::ModelCoefficientsPtr> getCylinders();
    float getVolume();
    float getLength();
    int getGrade();
    float getBiggerEndDBH();
    float getSmallerEndDBH();
    float getMiddleDBH();
    
private:
    void computelenght();
    void computeVolume();
    void setBiggerEndDBH();
    void setSmallerEndDBH();
    void setMiddleDBH();
    float cylinderVolume(pcl::ModelCoefficients::Ptr c);
    float cylinderLength(pcl::ModelCoefficients::Ptr c);
    
    std::vector<pcl::ModelCoefficientsPtr> m_cylinders;
    int m_qualityGrade;
    int m_curvatureGrade;
    int m_skewnessGrade;
    int m_branchBifurcationGrade;
    int m_taperGrade;
    float m_volume;
    float m_length;
    float m_smallerEndDBH;
    float m_biggerEndDBH;
    float m_middleDBH;

};

}

#endif /* sortiment_h */
