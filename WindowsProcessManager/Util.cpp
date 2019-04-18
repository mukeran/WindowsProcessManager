#include "Util.h"

wstring Util::formatTime(time_t snapshotTime)
{
	tm timeinfo;
	localtime_s(&timeinfo, &snapshotTime);
	wchar_t buffer[100];
	wcsftime(buffer, 98, L"%Y-%m-%d %H:%M:%S", &timeinfo);
	return wstring(buffer);
}

wstring Util::string2wstring(string str)

{
	wstring result;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	TCHAR* buffer = new TCHAR[len + 1];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';
	result.append(buffer);
	delete[] buffer;
	return result;
}