// persistence.cpp

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>
#include <cstring>

static constexpr const char* kRunSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static constexpr const char* kTestValueName = "OpenRogueTestEntry";
static constexpr const char* kMutexName = "Global\\OpenRogueRoguePoCMutex";

BOOL fnbEnsureRunEntry(const char* szPayloadPath)
{
	HKEY hKey = nullptr;
	LONG dRes = RegCreateKeyExA(
		HKEY_CURRENT_USER,
		kRunSubKey,
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_READ | KEY_WRITE,
		nullptr,
		&hKey,
		nullptr
	);

	if (ERROR_SUCCESS != dRes)
	{
		std::printf("[-] RegCreateKeyExA failed: %ld\n", dRes);
		return FALSE;
	}

	char szPath[MAX_PATH] = { 0 };
	DWORD nSize = sizeof(szPath);
	DWORD nType = 0;
	LONG res = RegQueryValueExA(
		hKey,
		kTestValueName,
		nullptr,
		&nType,
		reinterpret_cast<BYTE*>(szPath),
		&nSize
	);

	BOOL bWrite = (ERROR_SUCCESS != res) || (REG_SZ != nType) || (std::strcmp(szPath, szPayloadPath) != 0);
	if (bWrite)
	{
		LONG sRes = RegSetValueExA(
			hKey,
			kTestValueName,
			0,
			REG_SZ,
			reinterpret_cast<const BYTE*>(szPayloadPath),
			static_cast<DWORD>(std::strlen(szPayloadPath) + 1)
		);

		RegCloseKey(hKey);
		
		if (ERROR_SUCCESS != sRes)
		{
			std::printf("[-] RegSetValueExA failed: %ld\n", sRes);
			return FALSE;
		}
	}

	RegCloseKey(hKey);
	std::printf("[+] Run key intact: %s => %s\n", kTestValueName, szPath);

	return TRUE;
}

void fnCleanupRunEntry()
{
	HKEY hKey = nullptr;
	if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_CURRENT_USER, kRunSubKey, 0, KEY_SET_VALUE, &hKey))
	{
		LONG dRes = RegDeleteValueA(hKey, kTestValueName);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == dRes)
		{
			std::printf("[+] Cleaned up Run value: %s\n", kTestValueName);
		}
	}
}

BOOL fnbIsProcessRunning(const char* szTargetExeName)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnap)
		return FALSE;

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);
	BOOL bFound = FALSE;

	if (Process32First(hSnap, &pe))
	{
		do
		{
			if (_stricmp(pe.szExeFile, szTargetExeName) == 0)
			{
				bFound = TRUE;
				break;
			}
		} while (Process32Next(hSnap, &pe));
	}

	CloseHandle(hSnap);

	return bFound;
}

int main(int argc, char* argv[])
{
	bool bCleanUp = false;
	for (int i = 1; i < argc; ++i)
	{
		if (std::strcmp(argv[i], "--cleanup") == 0)
		{
			bCleanUp = true;
		}
	}

	if (bCleanUp)
	{
		fnCleanupRunEntry();
		return 0;
	}

	HANDLE hMutex = CreateMutex(nullptr, TRUE, kMutexName);
	if (hMutex != nullptr && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		std::printf("[-] Another instance is already running (mutex exists).\n");
		CloseHandle(hMutex);

		return 0;
	}

	char szSelfPath[MAX_PATH] = { 0 };
	GetModuleFileNameA(nullptr, szSelfPath, MAX_PATH);

	std::printf("Persistence");
	std::printf("Self PID: %lu | Path: %s\n", GetCurrentProcessId(), szSelfPath);

	for (int tick = 1; tick <= 5; ++tick)
	{
		std::printf("\n --- Tick %d/5 ---\n", tick);
		fnbEnsureRunEntry(szSelfPath);

		BOOL bExploreAlive = fnbIsProcessRunning("explorer.exe");
		std::printf("[*] explorer.exe present: %s\n", bExploreAlive ? "YES" : "NO");
		Sleep(1000);
	}

	std::printf("\nTip: Run with '--cleanup' to remove '%s' from HKCU Run.\n", kTestValueName);

	if (hMutex != nullptr)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}

	return 0;
}