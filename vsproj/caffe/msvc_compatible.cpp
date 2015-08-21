#include "caffe/msvc_compatible.hpp"

#ifdef _MSC_VER

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#include <fcntl.h>

namespace caffe {
unsigned long win32_getlasterror() { return ::GetLastError(); }

class _win32_workaround {
public:
  _win32_workaround();
  ~_win32_workaround();
}win32_workaround;

_win32_workaround::_win32_workaround() {
  _set_fmode(_O_BINARY);
}

_win32_workaround::~_win32_workaround() {
}

}

#endif