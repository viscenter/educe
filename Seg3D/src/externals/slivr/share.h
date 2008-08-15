#undef SLIVRSHARE

#if defined(_WIN32) 
#define SLIVRSHARE __declspec(dllexport)
#else
#define SLIVRSHARE
#endif
