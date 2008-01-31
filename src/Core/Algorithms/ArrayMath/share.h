#undef SCISHARE

#if defined(_WIN32) && !defined(BUILD_SCIRUN_STATIC)
#ifdef BUILD_Core_Algorithms_ArrayMath
#define SCISHARE __declspec(dllexport)
#else
#define SCISHARE __declspec(dllimport)
#endif
#else
#define SCISHARE
#endif

#if defined(min)
#undef min
#undef max
#endif
