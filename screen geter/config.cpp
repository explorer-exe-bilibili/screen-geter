#include "config.h"
#include <stdio.h>
#include<string>
#include <codecvt>

// 定义链表头节点
config::Node* config::head = NULL;
const wchar_t* config::LogString;
std::wstring config::configpath;
using namespace std;

//读取和补全配置项
void config::init() {
	configpath = L"config.ini";
	FILE* file = _wfopen(configpath.c_str(), L"r");
	if (file == NULL) {
		add(TIME, L"10000");
		saveFile();
		readFile();
		printAllConfigItems();
	}
	else
	{
		fclose(file);
		readFile();
		wstring LogString = get(TIME);
		if (wcscmp(LogString.c_str(), L"err") == 0)add(TIME, L"10000");
		saveFile();
		printAllConfigItems();
	}
}
//获取配置项内容
std::wstring config::get(const std::wstring& name) {
	Node* current = head;

	// 遍历链表查找匹配的配置项
	while (current != NULL) {
		if (current->item.name == name) {
			// 找到匹配的配置项,返回其参数值
			return current->item.value;
		}
		current = current->next;
	}

	// 如果没有找到匹配的配置项,返回L"err"
	return L"err";
}
std::wstring config::getpath(const std::wstring& name)
{
	wstring path = get(name);
	if (path.find_first_of(L"\\") == 0) {
		path = path;
	}
	return path;
}
double config::getd(const std::wstring& name)
{
	Node* current = head;

	// 遍历链表查找匹配的配置项
	while (current != NULL) {
		if (current->item.name == name) {
			// 找到匹配的配置项,返回其参数值
			try
			{
				return stod(current->item.value);
			}
			catch (const std::exception& e)
			{
				return 0;
			}
		}
		current = current->next;
	}

	// 如果没有找到匹配的配置项,返回L"err"
	return -1;
}
int config::getint(const std::wstring& name)
{
	Node* current = head;

	// 遍历链表查找匹配的配置项
	while (current != NULL) {
		if (current->item.name == name) {
			// 找到匹配的配置项,返回其参数值
			try
			{
				return stoi(current->item.value);
			}
			catch (const std::exception& e)
			{
				return 0;
			}
		}
		current = current->next;
	}

	// 如果没有找到匹配的配置项,返回L"err"
	return -1;
}
bool endsWith(const std::wstring& str, const std::wstring& suffix) {
	if (suffix.length() > str.length()) {
		return false;
	}
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}
long long config::getscreen(const std::wstring& name)
{
	wstring strv = get(name);
	long double douv = stold(strv);
	if (douv> 1) {
		long long llv = douv;
		return llv;
	}
	else {
		if (name.back()=='x' || endsWith(name,L"width")) {
		}
		else if (name.back()=='y' || endsWith(name,L"height")) {
		}
	}
}
long long config::getwindow(const std::wstring& name)
{
	wstring strv = get(name);
	long double douv = stold(strv);
	if (douv> 1) {
		long long llv = douv;
		return llv;
	}
	else {
		if (name.back()=='x' || endsWith(name,L"width")) {
		}
		else if (name.back()=='y' || endsWith(name,L"height")) {
		}
	}
}
int config::turnUpSideDown(const std::wstring& name)
{
	bool t = !getint(name);
	replace(name, to_wstring(t));
	return t;
}
// 添加配置项到链表
void config::add(const std::wstring& name, const std::wstring& value) {
	Node* current = head;
	Node* prev = nullptr;

	while (current != nullptr) {
		if (current->item.name == name) {
			current->item.value = value;
			return;
		}
		prev = current;
		current = current->next;
	}

	ConfigItem newItem;
	newItem.name = name;
	newItem.value = value;

	Node* newNode = new Node;
	if (newNode == nullptr) {
		return;
	}

	newNode->item = newItem;
	newNode->next = head;
	head = newNode;
}
void config::add(const std::wstring& name, const int value)
{
	Node* current = head;
	Node* prev = nullptr;

	while (current != nullptr) {
		if (current->item.name == name) {
			current->item.value = to_wstring(value);
			return;
		}
		prev = current;
		current = current->next;
	}

	ConfigItem newItem;
	newItem.name = name;
	newItem.value = value;

	Node* newNode = new Node;
	if (newNode == nullptr) {
		return;
	}

	newNode->item = newItem;
	newNode->next = head;
	head = newNode;
}
// 读取配置文件并保存配置项到链表
void config::readFile() {
	FILE* file = _wfopen(configpath.c_str(), L"r, ccs=UNICODE");
	if (file == NULL) {
		return;
	}

	wchar_t line[256];
	while (fgetws(line, sizeof(line) / sizeof(wchar_t), file) != NULL) {
		wchar_t* trimLine = wcstok(line, L"\n"); // 移除换行符

		wchar_t* currentOption = wcstok(trimLine, L"=");
		if (currentOption != NULL) {
			wchar_t* value = wcstok(NULL, L"=");
			// 添加配置项到链表
			add(std::wstring(currentOption), std::wstring(value));
		}
	}

	fclose(file);
}
// 保存配置项到配置文件
void config::saveFile() {
	FILE* file = _wfopen(configpath.c_str(), L"w, ccs=UNICODE");
	if (file == NULL) {
		return;
	}

	Node* current = head;
	while (current != NULL) {
		fwprintf(file, L"%s=%s\n", current->item.name.c_str(), current->item.value.c_str());
		current = current->next;
	}

	fclose(file);
}
// 释放链表节点的内存
void config::cleanup() {
	Node* current = head;
	Node* next;

	while (current != NULL) {
		next = current->next;
		// 不需要手动释放 std::wstring 对象的内存
		delete current;
		current = next;
	}

	head = NULL;
}
// 删除配置项
void config::deleteItem(const std::wstring& name) {
	Node* prev = nullptr;
	Node* current = head;

	while (current != nullptr) {
		if (current->item.name == name) {
			// 找到匹配的配置项,删除它
			if (prev != nullptr) {
				prev->next = current->next;
			}
			else {
				head = current->next;
			}

			delete current;
			return;
		}

		prev = current;
		current = current->next;
	}

}
//替换配置项
void config::replace(const std::wstring& name, const std::wstring& value) {
	Node* prev = nullptr;
	Node* current = head;

	while (current != nullptr) {
		if (current->item.name == name) {
			current->item.value = value;
			saveFile();
			return;
		}

		prev = current;
		current = current->next;
	}

	// 如果没有找到匹配的配置项,则添加新的配置项
	add(name, value);
	saveFile();
}
void config::replace(const std::wstring& name, const int value)
{
	Node* prev = nullptr;
	Node* current = head;

	while (current != nullptr) {
		if (current->item.name == name) {
			saveFile();
			return;
		}

		prev = current;
		current = current->next;
	}

	// 如果没有找到匹配的配置项,则添加新的配置项
	add(name, value);
	saveFile();
}
//打印配置
void config::printAllConfigItems() {
	Node* current = head;
	while (current != NULL) {
		current = current->next;
	}
}