#ifndef _PHANTOM_API_H
#define _PHANTOM_API_H

#ifdef PHANTOM_PLATFORM_WINDOWS
#ifdef _PHANTOM_INTERNAL
#define PHANTOM_API __declspec(dllexport)
#else
#define PHANTOM_API __declspec(dllimport)
#endif // _PHANTOM_INTERNAL
#elif defined PHANTOM_PLATFORM_LINUX
#ifdef _PHANTOM_INTERNAL
#define PHANTOM_API __attribute__((visibility("default")))
#else
#define PHANTOM_API
#endif
#endif // PHANTOM_PLATFORM_XXXXXX

#ifndef _UINT
#define _UINT
typedef unsigned int uint;
#endif // _UINT

// Forward declarations and typedefs
struct PAppData;
struct PAppRequest;
struct PDeviceManager;
struct PDisplayInfo;
struct PGraphicalAppRequest;
struct PGraphicalDevice;
struct PGraphicalDisplayRequest;
struct PWindowData;
struct PWindowRequest;

typedef struct PAppData PAppData;
typedef struct PAppRequest PAppRequest;
typedef struct PDeviceManager PDeviceManager;
typedef struct PDisplayInfo PDisplayInfo;
typedef struct PWindowData PWindowData;
typedef struct PWindowRequest PWindowRequest;
typedef struct PGraphicalAppRequest PGraphicalAppRequest;
typedef struct PGraphicalDevice PGraphicalDevice;
typedef struct PGraphicalDisplayRequest PGraphicalDisplayRequest;

#endif // _PHANTOM_API_H
