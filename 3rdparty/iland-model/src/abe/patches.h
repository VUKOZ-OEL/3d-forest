#ifndef ABE_PATCHES_H
#define ABE_PATCHES_H

#include "patch.h"
#include "grid.h"
#include "tree.h"
#include "scriptgrid.h"

#include <QObject>



namespace ABE {

class FMStand; // forward

class Patches : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<Patch*> list READ list WRITE setList)
    Q_PROPERTY(QRectF rectangle READ rectangle)
public:
    explicit Patches(QObject *parent = nullptr);
    ~Patches() override { clear(); }
    void setup(FMStand *stand);

    // main functions
    /// return the patch the given tree is on
    int patch(const Tree *tree) const { return patch(tree->positionIndex()); }
    int patch(QPoint pos) const {
        QPoint p = QPoint(pos.x() / cPxPerHeight - mStandOffset.x(),
                          pos.y() / cPxPerHeight - mStandOffset.y() );
        if (!mLocalStandGrid.isIndexValid(p))
            throw IException(QString("Invalid access to Patches: ix: %1,iy: %2.").arg(pos.x()).arg(pos.y()));
        return mLocalStandGrid.constValueAtIndex(p);
    }

    // properties
    FMStand* stand() const { return mStand; }
    Grid<short int> &grid()  { return mLocalStandGrid; }
    QList<Patch*> list() const { return mPatches; }
    void setList(QList<Patch*> l) { mPatches = l; updateGrid(); }
    QRectF rectangle() const { return mStandRect; }

    /// get patch from a tree (static)
    static int getPatch(QPoint position_lif);
public slots:
    /// re-create the internal stand grid from the list of patches
    void updateGrid();

    /// create a new patch that surrounds existing patches (with id `patchId`) in up to `grow_by` distance
    int createExtendedPatch(short patchId, short newPatchId, int grow_by=1);
    // query patches

    /// get the average value of the Light Influence Field (4m) on the patch `patch`
    double lif(Patch *patch);

    // create patches
    void createRandomPatches(int n);
    void clear();
    bool createPatch(double x, double y, QString shape_string, int id=-1);
    QList<Patch *> createStrips(double width, bool horizontal);
    QList<Patch *> createRegular(int size, int spacing);
    QList<Patch *> createFromGrid(ScriptGrid *grid);
signals:
private:
    /// get or
    Patch *getPatch(QList<Patch *> &list, int patch_id, bool create_on_miss=false);
    FMStand *mStand;
    QList<Patch*> mPatches;
    QRectF mStandRect; ///< metric rect of the stand
    QPoint mStandOffset; ///< offset of the stand on the 10m grid
    Grid<short int> mLocalStandGrid;

};


} // namespace ABE

#endif // ABE_PATCHES_H
