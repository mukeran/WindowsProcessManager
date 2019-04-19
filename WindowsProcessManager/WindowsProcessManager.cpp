#include <cstdio>
#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include <windows.h>
#include <TlHelp32.h>
#include "Util.h"
#include "LinkList.h"
#include "Process.h"
#include "CommandParser.h"
using namespace std;
class WindowsProcessManager
{
private:
	LinkList<Process>* processList;
	time_t snapshotTime;
	void checkRefreshProcessList() {
		if (processList == NULL || time(0) - snapshotTime > 300) {
			wprintf(L"由于没有获取过进程列表，或者离上次的获取时间太长，自动重新获取列表\n");
			getProcessList();
		}
	}
	Process queryProcess(int pID) {
		checkRefreshProcessList();
		auto equalPID = [](const Process & a, const Process & b) -> bool
		{
			return a.getPID() == b.getPID();
		};
		int pos = processList->find(Process(pID, L""), equalPID);
		return !pos ? INVALID_PROCESS : processList->get(pos);
	}
	vector<Process>* queryProcess(wstring pName) {
		checkRefreshProcessList();
		auto equalPName = [](const Process & a, const Process & b) -> bool
		{
			return a.getPName() == b.getPName();
		};
		vector<Process>* list = new vector<Process>();
		processList->find(Process(-1, pName), list, equalPName);
		return list;
	}
	void getProcessList() {
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(pe32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE)
		{
			wprintf(L"获取进程列表失败\n");
			return;
		}
		if(processList != NULL) delete processList;
		processList = new LinkList<Process>();
		snapshotTime = time(0);
		bool bMore = Process32First(hSnapshot, &pe32);
		while (bMore)
		{
			processList->insert(Process(pe32.th32ProcessID, pe32.szExeFile));
			bMore = Process32Next(hSnapshot, &pe32);
		}
		CloseHandle(hSnapshot);
	}
	void startProcess(wstring path, wstring cmd) {
		checkRefreshProcessList();
		Process p = Process::start(path, cmd);
		if (p != INVALID_PROCESS) {
			processList->insert(p);
			wprintf(L"创建进程成功 PID=%d 进程名=%s\n", p.getPID(), p.getPName().c_str());
		}
	}
	void showProcessList() {
		wprintf(L"最近的进程列表快照时间：%s\n", Util::formatTime(snapshotTime).c_str());
		auto func = [](const Process & val) { wprintf(L"PID: %6d 进程名: %s\n", val.getPID(), val.getPName().c_str()); };
		processList->map(func);
		wprintf(L"最近的进程列表快照时间：%s\n", Util::formatTime(snapshotTime).c_str());
	}
	void terminateProcess(Process& p) {
		if (p != INVALID_PROCESS) {
			bool result = p.terminate();
			if (result) {
				wprintf(L"终止进程成功 PID=%d 进程名=%s\n", p.getPID(), p.getPName().c_str());
				processList->remove(p);
			}
			else wprintf(L"终止进程失败 PID=%d 进程名=%s\n", p.getPID(), p.getPName().c_str());
		}
		else wprintf(L"terminateProcess 传入无效进程\n");
	}
	void terminateProcess(int pid) {
		Process p = queryProcess(pid);
		if (p != INVALID_PROCESS) terminateProcess(p);
		else wprintf(L"没有此进程 PID=%d\n", pid);
	}
	void terminateProcess(wstring pname) {
		vector<Process>* list = queryProcess(pname);
		if(list->size() == 0)
			wprintf(L"没有此进程 进程名=%s\n", pname.c_str());
		else if (list->size() == 1)
			terminateProcess(list->front());
		else {
			wprintf(L"这里有多项名称为 %s 的进程:\n", pname.c_str());
			vector<Process>::iterator it = list->begin();
			while(it != list->end()) {
				wprintf(L"PID: %6d 进程名: %s\n", it->getPID(), it->getPName().c_str());
				++it;
			}
			wprintf(L"需要全部终止吗 [y/N]:");
			wstring line;
			getline(wcin, line);
			while (line != L"y" && line != L"Y" && line != L"n" && line != L"N" && line != L"") {
				wprintf(L"你的输入有误，请重新输入 [y/N]:");
				getline(wcin, line);
			}
			if (line == L"y" || line == L"Y") {
				vector<Process>::iterator it = list->begin();
				int countTerminated = 0;
				while (it != list->end()) {
					terminateProcess(*it);
					if (it->terminated())
						++countTerminated;
					++it;
				}
				wprintf(L"完成终止任务，成功 %d 个，失败 %d 个\n", countTerminated, list->size() - countTerminated);
			}
			else wprintf(L"已取消终止\n");
		}
		delete list;
	}
public:
	const wstring VERSION = L"v0";
	WindowsProcessManager() {
		processList = NULL;
		snapshotTime = 0;
	}
	~WindowsProcessManager() {}
	void interactive() {
		wprintf(L"欢迎使用 WindowsProcessManager 交互模式，如果您不知道如何使用，请键入 help 命令\n");
		while (true) {
			wprintf(L"WindowsProcessManager>");
			wstring line;
			getline(wcin, line);
			CommandParser* cp;
			try {
				cp = new CommandParser(line);
			}
			catch (ParseFailedException e) {
				continue;
			}
			wstring cmd = cp->getArgv(0);
			if (cmd == L"s" || cmd == L"start")
			{
				cp->matchOptions();
				if (cp->hasOption(L"h") || cp->hasOption(L"help", true)) {
					wprintf(L"start - 创建进程\n"
						L"start              进入创建向导\n"
						L"start <path> [cmd] 运行 path 处的程序，可选调用 cmd 参数\n");
				}
				else if (cp->getArgc() == 1) {
					wstring path, cmd;
					wprintf(L"WindowsProcessManager>start>请输入调用程序的路径 (path)>");
					getline(wcin, path);
					wprintf(L"WindowsProcessManager>start>请输入调用程序的参数 (cmd) 可为空>");
					getline(wcin, cmd);
					startProcess(path, cmd);
				}
				else if (cp->getArgc() > 3)
					wprintf(L"您输入的命令有误，请检查\n");
				else {
					wstring path = cp->getArgument(0);
					wstring cmd = cp->getArgument(1);
					startProcess(path, cmd);
				}
			}
			else if (cmd == L"t" || cmd == L"terminate")
			{
				CommandParserTable cpt;
				cpt.add(L"i", L"id", true);
				cpt.add(L"n", L"name", true);
				cp->setTable(cpt);
				try {
					cp->matchOptions();
				}
				catch (ParseFailedException e) {
					continue;
				}
				if (cp->getArgc() == 1) {
					wstring method;
					wprintf(L"WindowsProcessManager>terminate>请输入查找进程的方式 [id/name]>");
					getline(wcin, method);
					while (method != L"id" && method != L"name") {
						wprintf(L"WindowsProcessManager>terminate>您的输入有误，请检查 [id/name]>");
						getline(wcin, method);
					}
					if (method == L"id") {
						int pid;
						wprintf(L"WindowsProcessManager>terminate>请输入 PID>");
						wstring line;
						getline(wcin, line);
						wstringstream wss(line);
						wss >> pid;
						terminateProcess(pid);
					}
					else {
						wstring pname;
						wprintf(L"WindowsProcessManager>terminate>请输入进程名>");
						getline(wcin, pname);
						terminateProcess(pname);
					}
				}
				else if (cp->hasOption(L"i")) {
					wstringstream wss(cp->getOption(L"i"));
					int pid;
					wss >> pid;
					terminateProcess(pid);
				}
				else if (cp->hasOption(L"n")) {
					wstring pname = cp->getOption(L"n");
					terminateProcess(pname);
				}
				else if (cp->hasOption(L"h") || cp->hasOption(L"help", true)) {
					wprintf(L"terminate - 终止进程\n"
						L"terminate                  进入终止向导\n"
						L"terminate -i/--id <id>     通过 PID 来终止进程\n"
						L"terminate -n/--name <name> 通过进程名称来终止进程\n"
						L"terminate -h/--help        显示该帮助\n");
				}
				else
					wprintf(L"未知参数，请检查\n");
			}
			else if (cmd == L"q" || cmd == L"query")
			{
				CommandParserTable cpt;
				cpt.add(L"i", L"id", true);
				cpt.add(L"n", L"name", true);
				cp->setTable(cpt);
				try {
					cp->matchOptions();
				}
				catch (ParseFailedException e) {
					continue;
				}
				if (cp->getArgc() == 1) {
					wstring method;
					wprintf(L"WindowsProcessManager>query>请输入查找进程的方式 [id/name]>");
					getline(wcin, method);
					while (method != L"id" && method != L"name") {
						wprintf(L"WindowsProcessManager>query>您的输入有误，请检查 [id/name]>");
						getline(wcin, method);
					}
					if (method == L"id") {
						int pid;
						wprintf(L"WindowsProcessManager>query>请输入 PID>");
						wstring line;
						getline(wcin, line);
						wstringstream wss(line);
						wss >> pid;
						Process p = queryProcess(pid);
						if (p != INVALID_PROCESS)
							wprintf(L"找到进程 PID=%d 进程名=%s\n", p.getPID(), p.getPName().c_str());
						else wprintf(L"没有此进程 PID=%d\n", pid);
					}
					else {
						wstring pname;
						wprintf(L"WindowsProcessManager>query>请输入进程名>");
						getline(wcin, pname);
						vector<Process>* list = queryProcess(pname);
						if (list->size() == 1)
							wprintf(L"找到进程 PID=%d 进程名=%s\n", list->front().getPID(), list->front().getPName().c_str());
						else if (list->size() > 1) {
							wprintf(L"这里有 %d 项名称为 %s 的进程:\n", list->size(), pname.c_str());
							vector<Process>::iterator it = list->begin();
							while (it != list->end()) {
								wprintf(L"PID: %6d 进程名: %s\n", it->getPID(), it->getPName().c_str());
								++it;
							}
						}
						else wprintf(L"没有此进程 进程名=%s\n", pname.c_str());
						delete list;
					}
				}
				else if (cp->hasOption(L"i")) {
					wstringstream wss(cp->getOption(L"i"));
					int pid;
					wss >> pid;
					Process p = queryProcess(pid);
					if (p != INVALID_PROCESS)
						wprintf(L"找到进程 PID=%d 进程名=%s\n", p.getPID(), p.getPName().c_str());
					else wprintf(L"没有此进程 PID=%d\n", pid);
				}
				else if (cp->hasOption(L"n")) {
					wstring pname = cp->getOption(L"n");
					vector<Process>* list = queryProcess(pname);
					if (list->size() == 1)
						wprintf(L"找到进程 PID=%d 进程名=%s\n", list->front().getPID(), list->front().getPName().c_str());
					else if (list->size() > 1) {
						wprintf(L"这里有 %d 项名称为 %s 的进程:\n", list->size(), pname.c_str());
						vector<Process>::iterator it = list->begin();
						while (it != list->end()) {
							wprintf(L"PID: %6d 进程名: %s\n", it->getPID(), it->getPName().c_str());
							++it;
						}
					}
					else wprintf(L"没有此进程 进程名=%s\n", pname.c_str());
					delete list;
				}
				else if (cp->hasOption(L"h") || cp->hasOption(L"help", true)) {
					wprintf(L"query - 查找进程\n"
						L"query                  进入终止向导\n"
						L"query -i/--id <id>     通过 PID 来查询进程\n"
						L"query -n/--name <name> 通过进程名称来查询进程\n"
						L"query -h/--help        显示该帮助\n");
				}
				else
					wprintf(L"未知参数，请检查\n");
			}
			else if (cmd == L"l" || cmd == L"list")
			{
				try {
					cp->matchOptions();
				}
				catch (ParseFailedException e) {
					continue;
				}
				if (cp->getArgc() == 1) {
					checkRefreshProcessList();
					showProcessList();
				}
				else if (cp->hasOption(L"r") || cp->hasOption(L"refresh", true)) {
					getProcessList();
					showProcessList();
				}
				else if (cp->hasOption(L"h") || cp->hasOption(L"help", true)) {
					wprintf(L"list - 显示进程列表\n"
						L"list              显示进程列表\n"
						L"list -r/--refresh 刷新列表显示\n"
						L"list -t/--time    显示上次进行进程快照的时间\n"
						L"list -h/--help    显示该帮助\n");
				}
				else if (cp->hasOption(L"t") || cp->hasOption(L"time", true)) wprintf(L"最近的进程列表快照时间：%s\n", Util::formatTime(snapshotTime).c_str());
				else wprintf(L"未知参数，请检查\n");
			}
			else if (cmd == L"h" || cmd == L"help")
			{
				wprintf(L"WindowsProcessManager - 简易的 Windows 进程管理器 %s by mukeran\n"
					L"\n"
					L"子命令:\n"
					L"s/start     - 创建进程\n"
					L"t/terminate - 终止进程\n"
					L"q/query     - 查询进程\n"
					L"l/list      - 列出进程\n"
					L"h/help      - 显示本帮助\n"
					L"exit/quit   - 退出程序\n"
					L"\n"
					L"可以通过 <subcommand> --help 来查看相应命令的帮助\n"
					L"由于作业的要求，任务列表快照不会及时更新，会默认间隔 5 分钟更新，而之后在本程序中的操作将会在快照上体现，可以通过 list --refresh 强制更新\n", VERSION.c_str());
			}
			else if (cmd == L"v" || cmd == L"version") wprintf(L"WindowsProcessManager %s\n", VERSION.c_str());
			else if (cmd == L"quit" || cmd == L"exit") exit(0);
			else wprintf(L"未知的命令，请重试\n");
		}
	}
	void command(int argc, char* argv[]) {
		CommandParser cp(argc, argv);
		try {
			cp.matchOptions();
		}
		catch (ParseFailedException e) {
			exit(1);
		}
		if (cp.hasOption(L"i") || cp.hasOption(L"interactive", true)) interactive();
		else if (cp.hasOption(L"v") || cp.hasOption(L"version", true)) wprintf(L"WindowsProcessManager %s\n", VERSION.c_str());
		else if (cp.hasOption(L"h") || cp.hasOption(L"help", true)) {
			wprintf(L"WindowsProcessManager - 简易的 Windows 进程管理器 %s by mukeran\n"
				L"\n"
				L"WindowsProcessManager [-i/--interactive] - 进入交互模式\n"
				L"WindowsProcessManager -v/--version       - 查看版本信息\n"
				L"WindowsProcessManager -h/--help          - 查看当前帮助\n"
				L"WindowsProcessManager <subcommand>       - 执行 subcommand 子命令，未实现\n"
				L"\n"
				L"子命令:\n"
				L"(当前为空，请使用交互模式)\n"
				L"\n"
				L"当前为命令行模式\n", VERSION.c_str());
		}
		else {
			wprintf(L"未知的指令\n");
			exit(1);
		}
	}
};
int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "chs");
	WindowsProcessManager program;
	if (argc == 1) program.interactive();
	else program.command(argc, argv);
	return 0;
}
