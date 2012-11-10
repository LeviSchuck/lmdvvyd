#pragma once
#define CORDITE_ENGINE
#if defined(_WIN32) || defined(WIN32) /* Win32 version */
#ifdef corditeEngine_EXPORTS
#  define DLL_EXPORT __declspec(dllexport)
#else
#  define DLL_EXPORT __declspec(dllimport)
#endif
#else
/* unix needs nothing */
#define DLL_EXPORT
#endif