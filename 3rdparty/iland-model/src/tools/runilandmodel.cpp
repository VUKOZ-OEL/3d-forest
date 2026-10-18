#include "global.h"
#include "model.h"
#include "modelcontroller.h"
#include "runilandmodel.h"

extern "C"
{
ILANDMODEL_API int runilandmodel(const char *path, int years)
{
    if (!path || !*path || years < 0)
    {
        qWarning() << "ERROR: invalid run iland-model parameters";
        return -1;
    }

    try
    {
        SimulationContext context;
        ModelController iLandModel(context);

        QString xmlName(path);

        iLandModel.setFileName(xmlName);
        if (iLandModel.hasError())
        {
            return -1;
        }

        iLandModel.create();
        if (iLandModel.hasError())
        {
            return -1;
        }

        iLandModel.run(years);
        if (iLandModel.hasError())
        {
            return -1;
        }
    }
    catch (const IException &error)
    {
        qWarning() << error.message();
        return -1;
    }
    catch (const std::exception &error)
    {
        qWarning() << error.what();
        return -1;
    }
    catch (...)
    {
        // do not let exceptions cross the C boundary
        return -1;
    }

    return 0;
}
}
