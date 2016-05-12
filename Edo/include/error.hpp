#ifndef _EDOWADO_ERROR_HPP
#define _EDOWADO_ERROR_HPP

#include <Edo.h>

namespace Edo
{
	class EDO_API EdoError //: public std::exception
	{
		public:
			EdoError(const std::string& msg);
			const char* what() const;

		protected:
			std::string m_Message;
	};

	//A class that represents an error caused by the underlying OS.
	class EDO_API OSError : public EdoError
	{
		public:
			OSError(const std::string& msg);
	};

	class EDO_API NotFoundError : public EdoError
	{
		public:
			NotFoundError(const std::string& msg);
	};

	class EDO_API AssertionError : public EdoError
	{
		public:
			AssertionError(const std::string& msg);
	};

	class EDO_API UnsupportedError : public EdoError
	{
		public:
			UnsupportedError(const std::string& msg);
	};

	class EDO_API InvalidObjectError : public EdoError
	{
		public:
			InvalidObjectError(const std::string& msg);
	};
}

//#ifndef _EDOWADO_ERROR_HPP
#endif