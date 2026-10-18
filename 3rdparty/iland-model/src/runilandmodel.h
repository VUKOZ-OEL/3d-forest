#ifndef ILANDMODEL_C_API_H
#define ILANDMODEL_C_API_H
#if defined(_WIN32)
#if defined(EXPORT_ILandModel)
#define ILANDMODEL_API __declspec(dllexport)
#else
#define ILANDMODEL_API __declspec(dllimport)
#endif
#else
#define ILANDMODEL_API __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
extern "C" {
#endif
/* path: UTF-8 project XML filename; years >= 0. Returns 0 or -1 on failure.
 * Calls have independent simulation contexts and may run concurrently.
 * Enable system.settings.isolateRunFiles in XML for automatic per-run folders;
 * otherwise configure distinct output, log and temporary paths concurrently. */
ILANDMODEL_API int runilandmodel(const char *path, int years);
#ifdef __cplusplus
}
#endif
#endif
