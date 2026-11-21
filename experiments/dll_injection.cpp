// dll_injection.cpp

#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>

DWORD GetProcessIdByName(const wchar_t* processName)
{


	return 0;
}

int wmain(int argc, wchar_t* argv[])
{
	if (argc < 3)
	{
		printf("Usage   : %ls <Target process name> <DLL path>\n", argv[0]);
		printf("Example : %ls notepad.exe test_payload.dll\n", argv[0]);

		return 1;
	}

	const wchar_t* targetProcess = argv[1];
	const wchar_t* dllPath = argv[2];

	wchar_t fullDllPath[MAX_PATH];
	if (GetFullPathNameW(dllPath, MAX_PATH, fullDllPath, NULL) == 0)
	{
		printf("[-] Cannot resolve DLL path, ErrorCode: %lu\n", GetLastError());
		return 1;
	}


}