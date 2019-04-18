#include "Process.h"



Process::Process()
{
	*this = INVALID_PROCESS;
	isTerminated = false;
}

Process::Process(int pID, std::wstring pName) :pID(pID), pName(pName)
{
	isTerminated = false;
}


Process::~Process()
{
}

int Process::getPID() const
{
	return pID;
}

std::wstring Process::getPName() const
{
	return pName;
}

bool Process::terminate()
{
	HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pID);
	if (hProcess == INVALID_HANDLE_VALUE)
		return false;
	isTerminated = ::TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);
	return isTerminated;
}

bool Process::terminated()
{
	return isTerminated;
}

Process Process::start(std::wstring path, std::wstring cmd = L"")
{
	STARTUPINFO stStartUpInfo;
	::memset(&stStartUpInfo, 0, sizeof stStartUpInfo);
	stStartUpInfo.cb = sizeof stStartUpInfo;

	PROCESS_INFORMATION stProcessInfo;
	::memset(&stProcessInfo, 0, sizeof stProcessInfo);

	int dSize = (cmd.length() + 1) * sizeof(WCHAR);
	WCHAR* szCmd = (WCHAR*)malloc(dSize + sizeof(WCHAR));
	if (szCmd == NULL)
	{
		wprintf(L"在创建进程时，申请中间变量所需内存失败\n");
		return INVALID_PROCESS;
	}
	wmemcpy(szCmd, cmd.c_str(), dSize);
	bool bRet = false;

	try
	{
		bRet = ::CreateProcess(path.c_str(), szCmd, NULL, NULL, false, CREATE_NEW_CONSOLE, NULL, NULL, &stStartUpInfo, &stProcessInfo);
		if (bRet)
		{
			stProcessInfo.hProcess = NULL;
			stProcessInfo.hThread = NULL;
			free(szCmd);
			return Process(stProcessInfo.dwProcessId, getPNameByPID(stProcessInfo.dwProcessId));
		}
		else
			wprintf(L"创建进程失败\n");
	}
	catch (...) {
		wprintf(L"创建进程失败\n");
	}
	free(szCmd);
	return INVALID_PROCESS;
}

std::wstring Process::getPNameByPID(int pID)
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		wprintf(L"在尝试通过 pID 获取进程的名称时，获取进程列表失败\n");
		return L"ERROR";
	}
	bool bMore = Process32First(hSnapshot, &pe32);
	while (bMore)
	{
		if (pe32.th32ProcessID == pID)
			return pe32.szExeFile;
		bMore = Process32Next(hSnapshot, &pe32);
	}
	CloseHandle(hSnapshot);
	return L"ERROR";
}

bool Process::operator!=(const Process& rhs) const
{
	return pID != rhs.pID || pName != rhs.pName;
}

bool Process::operator==(const Process& rhs) const
{
	return pID == rhs.pID && pName == rhs.pName;
}
