#include "process.hpp"


namespace Edo
{
	Process::Process()
	{
		this->m_isInitialized = false;
		this->m_ProcHandle = INVALID_HANDLE_VALUE;
		this->m_pProcInfo = nullptr;
		this->m_pSysInfo = nullptr;
		this->m_ModuleInfo = std::map<std::wstring, MODULEENTRY32>();
	}
	Process::Process(const std::wstring& procName, DWORD desiredAccess) : Process()
	{
		this->Open(procName, desiredAccess);
	}
	Process::~Process()
	{
		this->Close();
	}
	void Process::Open(const std::wstring& procName, DWORD desiredAccess)
	{
		HANDLE processSnap;
		PROCESSENTRY32 pe32;

		//Check if a process has already been opened.
		if (this->IsOpen())
			throw InvalidObjectError("A process has already been opened");

		//Try to create a snapshot of all open processes.
		processSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (processSnap == INVALID_HANDLE_VALUE || processSnap == NULL)
			throw OSError("Failed to create process snapshot.");

		//Set the size of PE32, not sure why this is necessary but whatever.
		pe32.dwSize = sizeof(PROCESSENTRY32);

		//Try to fetch the first process from the snapshot.
		if (!Process32First(processSnap, &pe32))
		{
			CloseHandle(processSnap);
			throw OSError("Failed to fetch first process from process snapshot");
		}

		//Start walking the snapshot looking for our target.
		do
		{
			//Check if the process matches our target name.
			if (wcscmp(pe32.szExeFile, procName.c_str()) == 0)
			{
				//Try to obtain a handle to the process.
				HANDLE process = OpenSecureProcess(pe32.th32ProcessID, desiredAccess);
				if (process == INVALID_HANDLE_VALUE)
				{
					CloseHandle(processSnap);
					throw OSError("Could not obtain a handle to the target.");
				}

				//Handle was obtained. Set some information.
				this->m_ProcHandle = process;
				this->m_pProcInfo = std::make_unique<PROCESSENTRY32>(pe32);

				break;
			}
		} while (Process32Next(processSnap, &pe32));

		//Close the process snapshot handle.
		CloseHandle(processSnap);

		//Check if the process was located or not.
		if (m_ProcHandle == INVALID_HANDLE_VALUE || m_pProcInfo == nullptr)
			throw NotFoundError("A process that matches the target name could not be found");

		//Get system information.
		this->m_pSysInfo.reset(new SYSTEM_INFO());
		GetNativeSystemInfo(m_pSysInfo.get());

		//Get information about the process architecture.
		BOOL isWoW = FALSE;
		BOOL result = IsWow64Process(m_ProcHandle, &isWoW);

		//Check if we could find out whether or not the process is running in Wow64
		if (result == FALSE)
			throw OSError("Unable to fetch architecture information about process.");

		//Check if we're running on Wow64 and if we're running 64bit OS or not.
		if (isWoW == FALSE && m_pSysInfo->wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			throw UnsupportedError("The architecture of the target process is unsupported. (Normally this means a 64bit process on a 64bit OS)");

		//Successfully opened.
		this->m_isInitialized = true;
	}
	void Process::Close()
	{
		//Check if object has been opened.
		if (this->IsOpen())
		//Close the current open process handle if we have one.
		if (m_ProcHandle != INVALID_HANDLE_VALUE && m_ProcHandle != NULL && m_ProcHandle != nullptr)
			CloseHandle(m_ProcHandle);

		//Reset all members to initial values.
		m_ProcHandle = INVALID_HANDLE_VALUE;
		m_ModuleInfo.clear();
		m_pProcInfo.reset(nullptr);
		m_pSysInfo.reset(nullptr);

		//Set the initialized flag.
		m_isInitialized = false;
	}
	const HWND Process::GetMainWindow() const
	{
		//Check if process has been initialized yet.
		if (this->IsClosed())
			throw InvalidObjectError("No process has been opened yet");

		FindWindowByPIDParam param;
		param.processID = m_pProcInfo->th32ProcessID;
		param.window = nullptr;

		//EnumWindows returns false when enumeration was canceled. Also check if the procedure set the error flag to success.
		if (!EnumWindows(&Edo::Process::FindWindowByPID, (LPARAM)&param) && (GetLastError() == ERROR_SUCCESS))
		{
			assert(param.window != nullptr);
			return param.window;
		}

		//Should this really throw?
		throw EdoError("No window could be associated with the target process.");
	}
	bool Process::IsOpen() const
	{
		return m_isInitialized;
	}
	bool Process::IsClosed() const
	{
		return !this->IsOpen();
	}
	const HANDLE Process::GetHandle() const
	{
		//Check if process has been initialized yet.
		if (this->IsClosed())
			throw InvalidObjectError("No process has been opened yet");

		return m_ProcHandle;
	}
	const DWORD Process::GetBaseAddress()
	{
		//Check if process has been initialized yet.
		if (this->IsClosed())
			throw InvalidObjectError("No process has been opened yet");

		return this->GetModuleBaseAddress(this->m_pProcInfo->szExeFile);
	}
	const DWORD Process::GetModuleBaseAddress(const std::wstring& moduleName)
	{
		HANDLE moduleSnapshot = INVALID_HANDLE_VALUE;
		MODULEENTRY32 me32;

		//Check if process has been initialized yet.
		if (this->IsClosed())
			throw InvalidObjectError("No process has been opened yet");

		//Check if the module info has already been loaded once.
		std::map<std::wstring, MODULEENTRY32>::iterator found = this->m_ModuleInfo.find(moduleName);
		if (found != this->m_ModuleInfo.end())
			return reinterpret_cast<DWORD>(found->second.modBaseAddr);

		//Try to create a snapshot of loaded modules.
		moduleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_pProcInfo->th32ProcessID);
		if (moduleSnapshot == INVALID_HANDLE_VALUE || moduleSnapshot == NULL)
		{
			printf("%u", GetLastError());
			throw OSError("Could not create module snapshot.");
		}

		me32.dwSize = sizeof(MODULEENTRY32);

		//Open the first module.
		if (!Module32First(moduleSnapshot, &me32))
		{
			CloseHandle(moduleSnapshot);
			throw OSError("Could not fetch the first module from module snapshot.");
		}

		do 
		{
			//Compare the name of the module to try and find a match.
			if (wcscmp(me32.szModule, moduleName.c_str()) == 0)
			{
				this->m_ModuleInfo.insert(std::pair<std::wstring, MODULEENTRY32>(moduleName, me32));
				CloseHandle(moduleSnapshot);

				return reinterpret_cast<DWORD>(me32.modBaseAddr);
			}
		}
		while (Module32Next(moduleSnapshot, &me32));

		CloseHandle(moduleSnapshot);
		throw NotFoundError("A module that matches the target name could not be located.");
	}
	bool Process::InjectLibrary(const std::wstring& filename)
	{
		TCHAR fullPath[MAX_PATH];

		//Try to get pathname.
		DWORD success = GetFullPathName(filename.c_str(), MAX_PATH, fullPath, NULL);
		if (success == 0)
			throw OSError("Could not locate a file with that name.");

		//Allocate memory for the library filepath in the target process.
		size_t len = wcslen(fullPath) + 1;
		size_t byteLen = len * sizeof(TCHAR);
		LPVOID targetAddr = VirtualAllocEx(this->GetHandle(), NULL, byteLen, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (targetAddr == NULL)
			throw OSError("Could not allocate memory in target process.");

		//Write the library into target memory space.
		DWORD bytesRead = 0;
		BOOL ret = WriteProcessMemory(this->GetHandle(), targetAddr, fullPath, byteLen, &bytesRead);
		if (ret == 0)
			throw OSError("Could not write filepath into target memory space.");

		//Retrieve address of LoadLibraryW function.
		LPTHREAD_START_ROUTINE funcAddr = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "LoadLibraryW");

		//Start initialization thread in remote process.
		HANDLE threadHandle = CreateRemoteThread(this->GetHandle(), NULL, NULL, funcAddr, targetAddr, NULL, NULL);
		if (threadHandle == NULL)
			throw OSError("Could not create new thread.");

		//Wait for thread initialization to complete.
		WaitForSingleObject(threadHandle, INFINITE);

		return true;
	}
	DWORD Process::OffsetPointer(const DWORD baseAddress, const std::vector<unsigned int>& offsets)
	{
		//Sanity check.
		if (this->IsClosed())
			throw InvalidObjectError("No process has been opened yet");

		DWORD address = baseAddress;

		//For every offset, add it to the current address and read what it points to.
		for (unsigned int i = 0; i < (offsets.size() - 1); i++)
		{
			address += offsets[i];
			address = ReadValue<DWORD>(address);
		}

		return address + offsets.back();
	}
	DWORD Process::FindBytePattern(const std::string& pattern, const std::string& format) const
	{
		//Check if process has been initialized yet.
		if (this->IsClosed())
			throw InvalidObjectError("No process has been opened yet");

		//Define some variables we will need.
		std::vector<BYTE> buffer = std::vector<BYTE>();
		DWORD startAddress = reinterpret_cast<DWORD>(this->m_pSysInfo->lpMinimumApplicationAddress);
		DWORD endAddress = reinterpret_cast<DWORD>(this->m_pSysInfo->lpMaximumApplicationAddress);
		DWORD protectMask = PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY | PAGE_WRITECOPY;
		DWORD oldProtect = 0;
		MEMORY_BASIC_INFORMATION mem = { 0 };

		//Check if the pattern and format matches each other.
		auto nrOfX = std::count(format.begin(), format.end(), 'x');
		if (nrOfX != pattern.size())
			throw AssertionError("Mismatch between the pattern and the format. Ensure that the number of bytes in the pattern matches the number of x in the format.");

		for (DWORD address = startAddress; address < endAddress;)
		{
			//Query the current memory region for information.
			if (VirtualQueryEx(m_ProcHandle, reinterpret_cast<LPVOID>(address), &mem, sizeof(mem)) == NULL)
			{
				//If the function failed with error code INVALID_PARAMETER, we've hit the end of the address space available to the process.
				if (GetLastError() == 87)
					break;
			}

			//If memory is commit and isn't too protected.
			if (mem.State == MEM_COMMIT && (mem.Protect & protectMask))
			{
				//If memory is guarded.
				if ((mem.Protect & PAGE_GUARD) == PAGE_GUARD)
					VirtualProtectEx(m_ProcHandle, mem.BaseAddress, mem.RegionSize, (mem.Protect &~PAGE_GUARD), &oldProtect);

				//Read.
				this->ReadBytes(reinterpret_cast<DWORD>(mem.BaseAddress), buffer, mem.RegionSize);

				//Check whether or not the Pattern was found inside the buffer.
				DWORD found = this->CompareBytePattern(buffer, pattern, format);

				if (found != -1)
					return address + found;
			}

			//Update variables.
			address += mem.RegionSize;
		}

		//Pattern was not found.
		throw NotFoundError("Byte pattern could not be located.");
	}
	DWORD Process::CompareBytePattern(const ByteBuffer& buffer, const std::string& pattern, const std::string& format) const
	{
		//Check if the pattern and format matches each other.
		auto nrOfX = std::count(format.begin(), format.end(), 'x');
		if (nrOfX != pattern.size())
			throw AssertionError("Mismatch between the pattern and the format. Ensure that the number of bytes in the pattern matches the number of x in the format.");

		//We already know it wont match if the buffer is too small.
		if (buffer.size() < format.size())
			return -1;

		//Loop through the entire buffer.
		for (DWORD i = 0; i < buffer.size() - format.size(); i++)
		{
			DWORD staticBytes = 0;
			bool match = false;

			//For every iteration of the first loop, try to loop through the entire format string and see if it matches.
			for (DWORD k = 0; k < format.size(); k++)
			{
				if (format.at(k) == 'x')
				{
					if (buffer.at(i + k) == (unsigned char)pattern.at(staticBytes))
					{
						//This byte matches, update the staticBytes counter.
						staticBytes++;

						if (staticBytes == pattern.size())
							match = true;
					}
					else
					{
						break;
					}
				}
			}

			//Loop finished, check if we found a match.
			if (match)
				return i;
		}

		//No match was found
		return -1;
	}
	void Process::ReadBytes(const DWORD address, ByteBuffer& buffer, const DWORD nrOfBytes) const
	{
		//Check if process has been initialized yet.
		if (this->IsClosed())
			throw InvalidObjectError("No process has been opened yet");

		buffer.resize(nrOfBytes);
		DWORD size = 0;

		BOOL result = ReadProcessMemory(this->m_ProcHandle, (LPCVOID)address, buffer.data(), nrOfBytes, &size);
		if (result == FALSE)
			throw OSError("Could not read memory of process.");
	}

	BOOL CALLBACK Process::FindWindowByPID(HWND window, LPARAM lParam)
	{
		FindWindowByPIDParam* parameters = reinterpret_cast<FindWindowByPIDParam*>(lParam);
		DWORD windowPID;

		//Get the process ID of the currently enumerated window.
		GetWindowThreadProcessId(window, &windowPID);

		//Check for match
		if (windowPID == parameters->processID)
		{
			//Set return parameter.
			parameters->window = window;

			//Set the success flag.
			SetLastError(ERROR_SUCCESS);

			//Stop enumeration.
			return FALSE;
		}

		//Continue enumeration.
		return TRUE;
	}
	HANDLE Process::OpenSecureProcess(DWORD pid, DWORD accessmask)
	{
		if (this->IsOpen())
			throw InvalidObjectError("A process has already been opened");

		PACL pAcl;
		PSECURITY_DESCRIPTOR pSecDesc;
		HANDLE process;
		DWORD result;

		//Try to open the process normally.
		process = OpenProcess(accessmask, false, pid);
		if (process != INVALID_HANDLE_VALUE && process != NULL)
		{
			return process;
		}

		//A secure process has been encountered.
		//Get the DACL security info of the current process.
		result = GetSecurityInfo(GetCurrentProcess(), SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pAcl, NULL, &pSecDesc);
		if (result != ERROR_SUCCESS)
			return INVALID_HANDLE_VALUE;

		//Try to open the process with write_dac access only.
		process = OpenProcess(WRITE_DAC, NULL, pid);
		if (process == NULL || process == INVALID_HANDLE_VALUE)
		{
			LocalFree(pSecDesc);
			return INVALID_HANDLE_VALUE;
		}

		//It succeeded.
		//Write the new security info into the target.
		result = SetSecurityInfo(process, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION, NULL, NULL, pAcl, NULL);
		if (result != ERROR_SUCCESS)
		{
			LocalFree(pSecDesc);
			return INVALID_HANDLE_VALUE;
		}

		//Success. Close some used variables.
		CloseHandle(process);
		LocalFree(pSecDesc);

		//Try to open the process with the original rights.
		process = OpenProcess(accessmask, false, pid);
		if (process == NULL)
			return INVALID_HANDLE_VALUE;

		//Process is either a valid handle or INVALID_HANDLE_VALUE
		return process;
	}
}
