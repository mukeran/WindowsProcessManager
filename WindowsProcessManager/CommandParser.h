#pragma once
#include <string>
#include <regex>
#include <vector>
#include <map>
#include <exception>
#include "Util.h"
#include "CommandParserTable.h"
using namespace std;
class ParseFailedException : public exception {
	virtual const char* what() const { return "The parse of command has failed"; }
};
class NoSuchOptionException : public exception {
	virtual const char* what() const { return "No such option"; }
};
class CommandParser
{
private:
	wstring command;
	vector<wstring> argv;
	map<wstring, wstring> options[2];
	vector<wstring> arguments;
	CommandParserTable table;
	void parse();
public:
	CommandParser(wstring command);
	CommandParser(int argc, wchar_t* argv[]);
	CommandParser(int argc, char* argv[]);
	~CommandParser();
	void setTable(CommandParserTable table);
	void matchOptions();
	bool hasOption(wstring name, bool isLongName = false);
	wstring getOption(wstring name, bool isLongName = false);
	wstring getArgument(unsigned int ord);
	wstring getArgv(unsigned int ord);
	unsigned int getArgc();
};

