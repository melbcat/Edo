#ifndef _EDOWADO_EDOWADO_TYPEDEF_HPP
#define _EDOWADO_EDOWADO_TYPEDEF_HPP

#include <Edo.h>
#include <functional>

namespace Edo
{
	struct FindWindowByPIDParam
	{
		int processID;
		HWND window;
	};

	using ByteBuffer = std::vector<unsigned char>;
	using TimeMilliseconds = std::chrono::milliseconds;
	using Clock = std::chrono::high_resolution_clock;
	using Timestamp = std::chrono::time_point<Clock>;
}

#endif