#include <edo_embedded.h>

void Edo::ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		std::stringstream ss;
		ss << "DirectX API call failed. (CODE: " << std::to_string(hr) << ")" << std::endl;
		throw std::exception(ss.str().c_str());
	}
}