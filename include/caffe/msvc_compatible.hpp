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
#include <signal.h>

#define snprintf _snprintf_s
#define getpid _getpid
#define __builtin_popcount __popcnt
#define __builtin_popcountl __popcnt64

typedef uint8_t mode_t;
inline int mkdir(const char *pathname, mode_t mode) {
  return _mkdir(pathname);
}

unsigned long win32_getlasterror();

typedef uint32_t uint;

#define SIGHUP SIGTERM

#define SA_RESTART 0x1

typedef struct {
	unsigned long sig[NSIG];
} sigset_t;

struct sigaction {
	void(*sa_handler)(int);
	sigset_t   sa_mask;
	int        sa_flags;
};

int sigaction(int signum, const struct sigaction *act,
struct sigaction *oldact);

int sigfillset(sigset_t *set);

#endif
#endif
