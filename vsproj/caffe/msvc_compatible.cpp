#include "caffe/msvc_compatible.hpp"

#ifdef _MSC_VER

#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#include <fcntl.h>

unsigned long win32_getlasterror() { return ::GetLastError(); }

struct sigaction _sigaction_buffer[NSIG];

class _win32_workaround {
public:
	_win32_workaround();
	~_win32_workaround();
}win32_workaround;

_win32_workaround::_win32_workaround() {
	_set_fmode(_O_BINARY);
	memset(_sigaction_buffer, 0, sizeof(_sigaction_buffer));
}

_win32_workaround::~_win32_workaround() {
}

int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact)
{
	void *rc = signal(signum, act->sa_handler);
	if (rc != SIG_ERR)
	{
		if (oldact)
			memcpy_s(oldact, sizeof(struct sigaction), &_sigaction_buffer[signum], sizeof(struct sigaction));
		memcpy_s(&_sigaction_buffer[signum], sizeof(struct sigaction), act, sizeof(struct sigaction));
		return 0;
	}
	return -1;
}

int sigfillset(sigset_t* set)
{
	memset(set, 1, sizeof(sigset_t));

	return 0;
}
#endif