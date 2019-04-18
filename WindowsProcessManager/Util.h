#pragma once
#include <string>
#include <ctime>
#include <windows.h>
using namespace std;
namespace Util {
	wstring formatTime(time_t snapshotTime);
	wstring string2wstring(string str);
}