#include "abe_global.h"

#include "patches.h"
#include "fmstand.h"
#include "forestmanagementengine.h"
#include "mapgrid.h"
#include "model.h"


namespace ABE {

Patches::Patches(QObject *parent)
    : QObject{parent}
{

}

void Patches::setup(FMStand *stand)
{
    mStand = stand;
    mStandRect = ForestManagementEngine::standGrid()->boundingBox(mStand->id());
    mLocalStandGrid.setup(mStandRect, cHeightSize);
    mStandOffset = ForestManagementEngine::standGrid()->grid().indexAt( mLocalStandGrid.metricRect().topLeft() );
    // mask stand grid with actual stand (out of stand to -1)
    GridRunner<int> runner(ForestManagementEngine::standGrid()->grid(), mStandRect);
    short int *p=mLocalStandGrid.begin();

    while (runner.next()) {
        if (*runner.current()!=mStand->id())
            *p = -1; // out of stand
        else
            *p = 0; // default
        ++p;
    }

}

void Patches::updateGrid()
{
    // reset to "no patch" = 0 (keep -1 flags)
    for (short int *p = mLocalStandGrid.begin(); p!=mLocalStandGrid.end(); ++p)
        *p = qMin(*p, ((short int)0));

    for(auto *p : mPatches) {
        p->update();
        for (auto idx : p->indices()) {
            mLocalStandGrid[idx] = p->id();
        }
    }
}

int Patches::createExtendedPatch(short patchId, short newPatchId, int grow_by)
{
    short int *neighbor[8];
    int n_extended = 0;
    Patch *patch=new Patch(this, newPatchId);
    GridRunner<short int> runner(mLocalStandGrid, mLocalStandGrid.rectangle());
    while (runner.next()) {
        if (*runner.current() == patchId) {
            runner.neighbors8(neighbor);
            for (int i=0;i<8;++i)
                if (neighbor[i]) {
                    if (*neighbor[i] == 0) {
                        *neighbor[i] = newPatchId;
                        patch->indices().push_back(mLocalStandGrid.index( mLocalStandGrid.indexOf(neighbor[i]) ));
                        ++n_extended;
                    }
                }
        }
    }
    if (n_extended>0) {
        patch->update();
        mPatches.push_back(patch);
    } else {
        delete patch;
    }

    return n_extended;

}

double Patches::lif(Patch *patch)
{
    float lif_value = 0.f;
    int n=0;
    for (auto idx : patch->indices()) {
        QRectF cell_rect = mLocalStandGrid.cellRect(mLocalStandGrid.indexOf(idx)).translated(mStandRect.topLeft());
        GridRunner<float> runner(GlobalSettings::instance()->model()->grid(), cell_rect);
        while (runner.next()) {
            lif_value += *runner.current();
            ++n;
        }
    }
    return n>0 ? lif_value / n : 0.;
}

int Patches::getPatch(QPoint position_lif)
{
    if (!GlobalSettings::instance()->model()->ABEngine()) return -1;
    int stand_id = ForestManagementEngine::standGrid()->standIDFromLIFCoord(position_lif);
    FMStand* stand = ForestManagementEngine::instance()->stand(stand_id);
    if (!stand) return -1;
    if (!stand->hasPatches()) return -1;
    return stand->patches()->patch(position_lif);
}

void Patches::createRandomPatches(int n)
{
    int i=0;
    int found=0;
    while (++i < 10*n) {
        Patch *p=new Patch(this, mPatches.length() + 1);
        QPoint pt = mLocalStandGrid.randomPosition();
        if (mLocalStandGrid[pt]!=0)
            continue;
        p->indices().push_back(mLocalStandGrid.index(pt));
        p->setRectangle(mLocalStandGrid.cellRect(pt));
        mPatches.push_back(p);
        if (++found >= n)
            break;
    }
    updateGrid();

}

void Patches::clear()
{
    qDeleteAll(mPatches);
    mPatches.clear();
}

bool Patches::createPatch(double x, double y, QString shape_string, int id)
{
    if (!ForestManagementEngine::standGrid()->grid().coordValid(x,y))
        return false;
    int s_id = ForestManagementEngine::standGrid()->grid().constValueAt(x,y);
    if (mStand->id() != s_id)
        return false;

}

QList<Patch*> Patches::createStrips(double width, bool horizontal)
{

    QList<Patch*> patches;

    for (short int* pg= mLocalStandGrid.begin(); pg!=mLocalStandGrid.end(); ++pg) {
        if (*pg != -1) {
            QPointF p = mLocalStandGrid.cellCenterPoint(pg);
            double dx = p.x() - mLocalStandGrid.metricRect().left();
            double dy = p.y() - mLocalStandGrid.metricRect().top();
            int strip = (horizontal ? dy : dx) / width + 1;
            getPatch(patches, strip, true)->indices().push_back(pg - mLocalStandGrid.begin());
        }
    }
    //updateGrid();
    return patches;

}

QList<Patch *> Patches::createRegular(int size, int spacing)
{
    QList<Patch*> patches;
    int box_size = size + spacing;
    int d_box = mLocalStandGrid.sizeY() / box_size;

    for (short int* pg= mLocalStandGrid.begin(); pg!=mLocalStandGrid.end(); ++pg) {
        if (*pg != -1) {
            QPoint p = mLocalStandGrid.indexOf(pg);
            int dx = p.x() / box_size;
            int dy = p.y() / box_size;
            int id = dy * d_box + dx + 1;
            if (p.x() % box_size < size &&
                p.y() % box_size < size)
                getPatch(patches, id, true)->indices().push_back(pg - mLocalStandGrid.begin());
        }
    }

    return patches;
}

QList<Patch *> Patches::createFromGrid(ScriptGrid *grid)
{
    QList<Patch*> patches;

    if (!grid || !grid->isCoordValid(mStandRect.x(), mStandRect.y()))
        return patches;

    GridRunner<double> runner(grid->grid(), mStandRect);
    short int *pg = mLocalStandGrid.begin();
    while (runner.next()) {
        short int v = *runner.current();
        if (*pg>-1 && v > 0.) {
            Patch *pt = getPatch(patches, v, true); // get / create patch
            pt->indices().push_back(pg - mLocalStandGrid.begin());
        }
        ++pg;
    }
    for (auto &p : patches)
        p->update();

    return patches;
}

Patch *Patches::getPatch(QList<Patch*> &list, int patch_id, bool create_on_miss)
{
    for (auto *p : list) {
        if (p->id() == patch_id)
            return p;
    }
    if (create_on_miss) {
        list.push_back(new Patch(this, patch_id));
        return list.last();
    } else {
        return nullptr;
    }
}

} // namespace ABE
