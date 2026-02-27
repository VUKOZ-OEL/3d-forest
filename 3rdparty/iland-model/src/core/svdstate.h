#ifndef SVDSTATE_H
#define SVDSTATE_H
#include <QHash>
#include <QVector>

#include "grid.h"

class SVDStates;
struct SVDState
{
    SVDState(): composition(0), structure(0), function(0), dominant_species_index(-1), Id(0) { for (int i=0;i<5;++i) admixed_species_index[i]=-1; }
    int composition; // a kind of hash number combining all species (can be negative)
    int structure;
    int function;
    int dominant_species_index;
    int admixed_species_index[5];
    /// the unique Id of the state within the current simulation.
    int Id;
    /// get a string with the main species on the resource unit
    /// dominant species is uppercase, all other lowercase
    QString compositionString() const;
    /// a human readable string describing the state
    QString stateLabel() const;
    /// calculate neighborhood population, return total weight added to the vector of species
    float neighborhoodAnalysis(QVector<float> &v);
    /// link to the SVD container class
    static SVDStates *svd;
};

// functions for the hashing of states
inline bool operator==(const SVDState &s1, const SVDState &s2)
{
    // this does not include comparing the 'Id'!
    bool equal = s1.composition==s2.composition && s1.structure==s2.structure && s1.function==s2.function && s1.dominant_species_index==s2.dominant_species_index;
    if (!equal) return false;
    for (int i=0;i<5;++i)
        equal = equal && (s1.admixed_species_index[i]==s2.admixed_species_index[i]);
    return equal;
}

inline size_t qHash(const SVDState &key, size_t seed)
{
    size_t hash_value = qHash(key.composition, seed) ^ qHash(key.structure, seed) ^ qHash(key.function, seed);
    return hash_value;
}

class ResourceUnit; // forward
class SVDStates
{
public:
    SVDStates();
    enum EStructureClassification { Structure4m, // 0-4, 4-8, 8-12, ... + irregular
                                    Structure2m }; // 0-2, 2-4, 4-6, .... + irregular
    enum EFunctioningClassification { Functioning3Classes, // LAI 0-2, 2-4, >4
                                   Functioning5Classes}; // LAI 0-1, 1-2, 2-3, 3-4, >4

    /// calculate and returns the Id ofthe state that
    /// the resource unit is currently in
    int evaluateState(ResourceUnit *ru);
    /// access the state with the id 'index'
    const SVDState &state(int index) const { return mStates[index]; }
    /// return true if 'state' is a valid state Id
    bool isStateValid(int state) const { return state>=0 && state<mStates.size(); }
    /// return the number of states
    int count() const { return mStates.size(); }

    /// evaluate the species composition in the neighborhood of the cell
    /// this is executed in parallel.
    void evaluateNeighborhood(ResourceUnit *ru);

    /// get a string with the main species on the resource unit
    /// dominant species is uppercase, all other lowercase
    QString compositionString(int index) { return mCompositionString[index]; }

    /// create a human readable string representation of the string
    QString stateLabel(int index);

private:
    EStructureClassification mStructureClassification;
    EFunctioningClassification mFunctioningClassification;

    inline void executeNeighborhood(QVector<float> &vec, QPoint center_point, QVector<QPoint> &list, const Grid<ResourceUnit*> &grid);
    QString createCompositionString(const SVDState &s);
    QVector<SVDState> mStates;
    QVector<QString> mCompositionString;
    QHash<SVDState, int> mStateLookup;
};
#endif // SVDSTATE_H
