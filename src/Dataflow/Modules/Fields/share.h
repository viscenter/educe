#undef SCISHARE

#if defined(_WIN32) && !defined(BUILD_SCIRUN_STATIC)
#  ifdef BUILD_Dataflow_Modules_Fields
#    define SCISHARE __declspec(dllexport)
#  else
#    define SCISHARE __declspec(dllimport)
#  endif
#else
#  define SCISHARE
#endif

// FYI: This should fix the problem that Windows has a min() and max() macro
// As this should be the last header file that is included, this should take 
// care of the problem.

#if defined(min)
#undef min
#undef max
#endif
