#include "caffe/msvc_compatible.hpp"

#ifdef _MSC_VER

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <Wincrypt.h>

#include <fcntl.h>

#include <assert.h>

namespace caffe {

HCRYPTPROV h_cryptprov;

int win32_genrandom(uint32_t len, void *buffer) {
  return ::CryptGenRandom(h_cryptprov, len, static_cast<unsigned char *>(buffer));
}

unsigned long win32_getlasterror() { return ::GetLastError(); }

class _win32_workaround {
public:
	_win32_workaround();
	~_win32_workaround();
}win32_workaround;

_win32_workaround::_win32_workaround() {
  _set_fmode(_O_BINARY);
  assert(::CryptAcquireContext(&h_cryptprov, nullptr, nullptr, PROV_RSA_FULL, 0));
}

_win32_workaround::~_win32_workaround() {
  assert(::CryptReleaseContext(h_cryptprov, 0));
}

}

#endif