#ifndef _EDOWADO_PROCESS_HPP
#define _EDOWADO_PROCESS_HPP

#include <Edo.h>

#include <tlhelp32.h>
#include <memory>
#include <assert.h>
#include <Aclapi.h>

#include <error.hpp>
#include <edo_typedef.hpp>

namespace Edo
{
	class EDO_API Process
	{
		public:
			Process();
			Process(const std::wstring& procName, DWORD desiredAccess);
			Process(HANDLE handle);
			~Process();

			//Delete the copy constructor and copy assignment operator.
			Process(const Process&) = delete;
			Process Process::operator=(const Process&) = delete;

			//Returns true if a process has been opened.
			bool IsOpen() const;

			//Returns true if a process has not yet been opened.
			bool IsClosed() const;

			//Attempts to open a process.
			void Open(const std::wstring& procName, DWORD desiredAccess);

			//Closes a process.
			void Close();

			//Attempts to retrieve the main window of a process.
			const HWND GetMainWindow() const;

			//Returns the wrapped process handle.
			const HANDLE GetHandle() const;

			//Returns the base address of the process.
			const DWORD GetBaseAddress();

			//Attempts to find the base address of the module with specified name. 
			const DWORD GetModuleBaseAddress(const std::wstring& moduleName);

			//Attempts to inject a code library into the target process.
			bool InjectLibrary(const std::wstring& filename);

			//Adds the first offset to the base address and gets the address pointed to by that.
			//Repeats for every offset except the last one which is just added to the final address.
			DWORD OffsetPointer(const DWORD baseAddress, const std::vector<unsigned int>& offsets);

			//Attempts to scan the process memory and find the byte pattern specified by patternMask and format.
			//Use the following format when defining a format string: "xxx??xxx" where 'x' is a static byte and '?' is a dynamic byte.
			DWORD FindBytePattern(const std::string& pattern, const std::string& format) const;
			
			//Check whether the buffer contains the specified pattern or not.
			DWORD CompareBytePattern(const ByteBuffer& buffer, const std::string& pattern, const std::string& format) const;

			//Attempts to read the number of bytes specified by nrOfBytes and return them in raw format.
			void ReadBytes(const DWORD address, ByteBuffer& buffer, const DWORD nrOfBytes) const;

			//Attempts to write the number of bytes specified by nrOfBytes.
			void WriteBytes(const DWORD address, const ByteBuffer& buffer, const DWORD nrOfBytes) const;


			//TEMPLATE FUNCTIONS

			//Attempts to read a value of the specified type from the memory location specified by the passed in address.
			template<typename T>
			T ReadValue(const DWORD address) const
			{
				DWORD size = 0;
				T buffer = T();

				BOOL result = ReadProcessMemory(this->m_ProcHandle, (LPCVOID)address, &buffer, sizeof(T), &size);
				if (result == FALSE)
					throw OSError("Could not read memory of process.");

				return buffer;
			}

			//Attempts to write the value passed in into the memory location specified by address.
			template<typename T>
			void WriteValue(const DWORD address, const T value) const
			{
				DWORD size = 0;
				BOOL result = WriteProcessMemory(this->m_ProcHandle, (LPVOID)address, &value, sizeof(T), &size);
				if (result == FALSE)
					throw OSError(TEXT("Could not write to memory of process."));
			}

		private:

			//Callback for use with EnumWindows (used in GetMainWindow!).
			static BOOL CALLBACK FindWindowByPID(HWND window, LPARAM lParam);
			HANDLE OpenSecureProcess(DWORD pid, DWORD accessmask);
			bool m_isInitialized;
			HANDLE m_ProcHandle;
			std::map<std::wstring, MODULEENTRY32> m_ModuleInfo;
			std::unique_ptr<PROCESSENTRY32> m_pProcInfo;
			std::unique_ptr<SYSTEM_INFO> m_pSysInfo;
	};
}

//#ifndef _EDOWADO_PROCESS_HPP
#endif