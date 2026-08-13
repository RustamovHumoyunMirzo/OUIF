#pragma once

#if defined(_WIN32) && defined(OUIF_SHARED)
#if defined(OUIF_BUILDING_LIBRARY)
#define OUIF_API __declspec(dllexport)
#else
#define OUIF_API __declspec(dllimport)
#endif
#else
#define OUIF_API
#endif
