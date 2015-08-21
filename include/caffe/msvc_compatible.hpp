#ifndef MSVC_COMPATIBLE
#define MSVC_COMPATIBLE

#ifndef _MSC_VER
#define DEFINE_FORCE_LINK_SYMBOL(classname, token)
#else
#define DEFINE_FORCE_LINK_SYMBOL(classname, token) \
  int g_caffe_force_link_##token##_##classname = 0
#endif

#ifdef _MSC_VER
#include <intrin.h>
#include <process.h>
#include <stdarg.h>
#include <stdint.h>
#include <direct.h>
#include <io.h>

#define snprintf _snprintf_s
#define getpid _getpid
#define __builtin_popcount __popcnt
#define __builtin_popcountl __popcnt64

typedef uint8_t mode_t;
inline int mkdir(const char *pathname, mode_t mode) {
  return _mkdir(pathname);
}

namespace caffe{
unsigned long win32_getlasterror();
}

typedef uint32_t uint;

#endif
#endif
