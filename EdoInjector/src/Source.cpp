#include <edo_injector.h>
#include <iostream>

int main(int argc, char** argv)
{
	try
	{
		Edo::Process proc;
		proc.Open(TEXT("Morrowind.exe"), PROCESS_ALL_ACCESS);

		proc.InjectLibrary(TEXT("EdoScanner.dll"));

		std::cout << "Injected!" << std::endl;
	}
	catch (Edo::EdoError& err)
	{
		std::cout << err.what() << std::endl;
	}
}