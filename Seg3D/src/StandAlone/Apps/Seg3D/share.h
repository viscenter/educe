#undef SCISHARE

#if defined(_WIN32) && !defined(BUILD_SCIRUN_STATIC)
#ifdef BUILD_Painter
#define SCISHARE __declspec(dllexport)
#else
#define SCISHARE __declspec(dllimport)
#endif
#else
#define SCISHARE
#endif
