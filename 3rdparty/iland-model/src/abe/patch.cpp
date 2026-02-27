#include "patch.h"
#include "fmtreelist.h"
#include "patches.h"

namespace ABE {

Patch::Patch(Patches* patches, int id, QObject *parent)
    : QObject{parent}, mPatches{patches}, mPatchId{id}
{
    mArea = 0.;
    mScore = 0.;
}

void Patch::update()
{
    mArea = mCells.size() * cHeightPixelArea / 10000.;
    mRect= QRectF();

    // lets ignore teh rectangle for the moment
}


} // namespace ABE
