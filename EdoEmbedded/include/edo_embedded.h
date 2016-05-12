#ifndef _EDO_EMBEDDED_H
#define _EDO_EMBEDDED_H

#include <windows.h>
#include <exception>
#include <cstdint>
#include <string>
#include <sstream>

namespace Edo
{
	void ThrowIfFailed(HRESULT hr);
}
#endif
