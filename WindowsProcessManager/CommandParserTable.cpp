#include "CommandParserTable.h"
#include <regex>
using namespace std;


CommandParserTable::CommandParserTable()
{
}


CommandParserTable::~CommandParserTable()
{
}

void CommandParserTable::add(wstring short_name, wstring long_name = L"", bool hasArgument = false)
{
	if (short_name == L"") return;
	if (hasArgumentMappings.count(short_name) || mappings.count(short_name))
		throw DuplicateShortNameException();
	if (mappings.count(long_name) || hasArgumentMappings.count(long_name))
		throw DuplicateLongNameException();
	if (long_name != L"") {
		mappings[long_name] = short_name;
		rmappings[short_name] = long_name;
	}
	hasArgumentMappings[short_name] = hasArgument;
}

wstring CommandParserTable::queryLong(wstring short_name)
{
	if (rmappings.count(short_name))
		return rmappings[short_name];
	return L"";
}

wstring CommandParserTable::queryShort(wstring long_name)
{
	if (mappings.count(long_name))
		return mappings[long_name];
	return L"";
}

bool CommandParserTable::hasArgument(wstring name)
{
	if (!hasArgumentMappings.count(name)) {
		if (!mappings.count(name)) return false;
		else name = mappings[name];
	}
	return hasArgumentMappings[name];
}
