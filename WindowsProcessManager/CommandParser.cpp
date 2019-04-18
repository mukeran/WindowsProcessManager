#include "CommandParser.h"



void CommandParser::parse()
{
	int len = command.length(), i = 0;
	while (i < len) {
		while (command[i] == ' ' && i < len) ++i;
		if (i == len) break;
		if (command[i] == L'\"') {
			int start = i++;
			while (((command[i] != L'\"') || (i > 0 && command[i] == L'\"' && command[i - 1] == L'\\')) && i < len) ++i;
			if (i == len) {
				wprintf(L"您输入的命令 \" 不匹配，请检查\n");
				throw ParseFailedException();
			}
			if (i != len - 1 && command[i] == L'\"' && command[i + 1] == L'\"') {
				wprintf(L"您输入的命令 \" 语法有误，请检查\n");
				throw ParseFailedException();
			}
			argv.push_back(command.substr(start + 1, i - start - 1));
			++i;
		}
		else {
			int start = i;
			int inQuote = 0;
			while (i < len)
			{
				if (command[i] == L'\"' && command[i - 1] != L'\\')
					++inQuote;
				if (inQuote == 3) {
					wprintf(L"您输入的命令 \" 语法有误，请检查\n");
					throw ParseFailedException();
				}
				if ((inQuote == 0 || inQuote == 2) && command[i] == L' ')
					break;
				++i;
			}
			if (inQuote == 1) {
				wprintf(L"您输入的命令 \" 不匹配，请检查\n");
				throw ParseFailedException();
			}
			argv.push_back(command.substr(start, i - start));
		}
	}
}

CommandParser::CommandParser(wstring command) :command(command)
{
	parse();
}

CommandParser::CommandParser(int argc, wchar_t* argv[])
{
	for (int i = 0; i < argc; ++i)
		this->argv.push_back(argv[i]);
}

CommandParser::CommandParser(int argc, char* argv[])
{
	for (int i = 0; i < argc; ++i)
		this->argv.push_back(Util::string2wstring(string(argv[i])));
}


CommandParser::~CommandParser()
{
}

void CommandParser::setTable(CommandParserTable table)
{
	this->table = table;
}

void CommandParser::matchOptions()
{
	int argc = argv.size();
	for (int i = 1; i < argc; ++i) {
		if (argv[i].length() == 1 && argv[i][0] == L'-') {
			wprintf(L"您输入的命令有误，参数中出现了单个 '-'，请检查\n");
			throw ParseFailedException();
		}
		else if (argv[i].length() == 2 && argv[i] == L"--") {
			wprintf(L"您输入的命令有误，参数中出现了单个 '--'，请检查\n");
			throw ParseFailedException();
		}
		else if (argv[i][0] != L'-')
			arguments.push_back(argv[i]);
		else {
			wstring short_name, long_name;
			if (argv[i][1] != L'-') {
				short_name = argv[i].substr(1);
				long_name = table.queryLong(short_name);
			}
			else {
				long_name = argv[i].substr(2);
				short_name = table.queryShort(long_name);
			}
			if (!table.hasArgument(short_name)) {
				if (short_name != L"") options[0][short_name] = L"";
				if (long_name != L"") options[1][long_name] = L"";
			}
			else {
				if (i == argc - 1) {
					wprintf(L"参数 %s 缺少字段，请检查\n", argv[i].c_str());
					throw ParseFailedException();
				}
				if (short_name != L"") options[0][short_name] = argv[i + 1];
				if (long_name != L"") options[1][long_name] = argv[i + 1];
				++i;
			}
		}
	}
}

bool CommandParser::hasOption(wstring name, bool isLongName)
{
	return options[isLongName].count(name);
}

wstring CommandParser::getOption(wstring name, bool isLongName)
{
	if (!options[isLongName].count(name))
		throw NoSuchOptionException();
	return options[isLongName][name];
}

wstring CommandParser::getArgument(unsigned int ord)
{
	if (ord >= arguments.size())
		return L"";
	return arguments[ord];
}

wstring CommandParser::getArgv(unsigned int ord)
{
	if (ord >= argv.size())
		return L"";
	return argv[ord];
}

unsigned int CommandParser::getArgc()
{
	return argv.size();
}
