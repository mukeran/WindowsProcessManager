#pragma once
#include <set>
#include <string>
#include <map>
#include <exception>
using namespace std;
class DuplicateShortNameException : public exception {
	virtual const char* what() const { return "The short name has been registered"; }
};
class DuplicateLongNameException : public exception {
	virtual const char* what() const { return "The long name has been registered"; }
};
class CommandParserTable
{
private:
	map<wstring, wstring> mappings;
	map<wstring, wstring> rmappings;
	map<wstring, bool> hasArgumentMappings;
public:
	CommandParserTable();
	~CommandParserTable();
	void add(wstring short_name, wstring long_name, bool hasArgument);
	wstring queryLong(wstring short_name);
	wstring queryShort(wstring long_name);
	bool hasArgument(wstring name);
};

