#include "caffe/msvc_compatible.hpp"

#ifdef _MSC_VER

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <Wincrypt.h>

#include <fcntl.h>

namespace caffe {
HCRYPTPROV h_cryptprov;
int win32_genrandom(uint32_t len, void *buffer) {
  return CryptGenRandom(h_cryptprov, len, static_cast<unsigned char *>(buffer));
}

_global_object_initializer::_global_object_initializer() {
  _set_fmode(_O_BINARY);
  CHECK(CryptAcquireContext(&h_cryptprov, nullptr, nullptr, PROV_RSA_FULL, 0));
}

_global_object_initializer::~_global_object_initializer() {
  CHECK(CryptReleaseContext(h_cryptprov, 0));
}

_global_object_initializer global_object_initializer;
}

#endif