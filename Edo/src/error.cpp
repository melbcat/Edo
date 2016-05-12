#include "error.hpp"


namespace Edo
{
	//EdowadoError methods.
	EdoError::EdoError(const std::string& msg)
	{
		this->m_Message = msg;
	}

	//Override std::exception::what()
	const char* EdoError::what() const
	{
		return this->m_Message.c_str();
	}

	//OSError methods.
	OSError::OSError(const std::string& msg) : EdoError(msg) {}

	//NotFoundError methods.
	NotFoundError::NotFoundError(const std::string& msg) : EdoError(msg) {}

	//AssertionError methods.
	AssertionError::AssertionError(const std::string& msg) : EdoError(msg) {}

	//UnsupportedError methods.
	UnsupportedError::UnsupportedError(const std::string& msg) : EdoError(msg) {}

	//InvalidObjectError methods.
	InvalidObjectError::InvalidObjectError(const std::string& msg) : EdoError(msg) {}
}
