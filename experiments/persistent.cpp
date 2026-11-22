// persistent.cpp

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>
#include <cstring>

static constexpr const char* kRunSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static constexpr const char* kTestValueName = "OpenRogueTestEntry";
static constexpr const char* kMutexName = "Global\\OpenRogueRoguePoCMutex";

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
		
	}
}