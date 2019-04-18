#pragma once
#include <string>
#include <windows.h>
#include <TlHelp32.h>
#define INVALID_PROCESS Process(-1, L"")
class Process
{
private:
	int pID;
	std::wstring pName;
	bool isTerminated;
public:
	Process();
	Process(int pID, std::wstring pName);
	~Process();
	int getPID() const;
	std::wstring getPName() const;
	bool terminate();
	bool terminated();
	static Process start(std::wstring path, std::wstring cmd);
	static std::wstring getPNameByPID(int pID);
	bool operator != (const Process& rhs) const;
	bool operator == (const Process& rhs) const;
};

