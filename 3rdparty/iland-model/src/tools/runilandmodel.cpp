#include "global.h"
#include "model.h"
#include "modelcontroller.h"

#if defined(_WIN32)
    #if defined(EXPORT_ILandModel)   // CMake already sets EXPORT_ILandModel=1
      #define ILANDMODEL_API __declspec(dllexport)
    #else
      #define ILANDMODEL_API __declspec(dllimport)
    #endif
#else
    #define ILANDMODEL_API __attribute__((visibility("default")))
#endif

extern "C"
{
ILANDMODEL_API int runilandmodel(const char *path, int years)
{
    try
    {
        static std::unique_ptr<QCoreApplication> app;

        if (!QCoreApplication::instance())
        {
            int argc = 0;
            app = std::make_unique<QCoreApplication>(argc, nullptr);
        }

        ModelController iLandModel;
        GlobalSettings::instance()->setModelController(&iLandModel);

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
    catch (...)
    {
        // do not let exceptions cross the C boundary
        return -1;
    }

    return 0;
}
}