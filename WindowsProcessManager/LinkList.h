#pragma once
#include <exception>
#include <string>
#include <vector>
#define NULL 0
class InvalidPosException : public std::exception
{
	virtual const char* what() const { return "Invalid position"; }
};
template <typename T>
bool equal(const T& a, const T& b)
{
	return a == b;
}
template <typename T>
class LinkList
{
private:
	struct Node {
		Node* next;
		T val;
		Node() { next = NULL; }
		Node(T val) :val(val) { next = NULL; }
	};
	int len;
	Node* head, * tail;
public:
	LinkList() {
		head = tail = new Node();
		len = 0;
	}
	~LinkList() {
		if (head->next != NULL) {
			Node* cur = head->next;
			while (cur != NULL) {
				Node* tmp = cur->next;
				delete cur;
				cur = tmp;
			}
		}
		delete head;
	}
	int length() { return len; }
	void insert(T val) {
		++len;
		tail->next = new Node(val);
		tail = tail->next;
	}
	bool remove(T val) {
		Node* cur = head->next, * pre = head;
		while (cur != NULL) {
			if (cur->val == val)
				break;
			pre = cur;
			cur = cur->next;
		}
		if (cur == NULL)
			return false;
		pre->next = cur->next;
		if (cur == tail)
			tail = pre;
		--len;
		delete cur;
		return true;
	}
	bool removeByPos(int pos) {
		if (pos > len || pos <= 0)
			throw InvalidPosException();
		Node* cur = head->next, * pre = head;
		int cur_pos = 1;
		while (cur != NULL) {
			if (cur_pos == pos)
				break;
			++cur_pos;
			pre = cur;
			cur = cur->next;
		}
		pre->next = cur->next;
		if (cur == tail)
			tail = pre;
		--len;
		delete cur;
		return true;
	}
	int find(T val, bool(*cmp)(const T& a, const T& b) = equal) {
		Node* cur = head->next;
		int pos = 1;
		while (cur != NULL) {
			if ((*cmp)(cur->val, val))
				break;
			++pos;
			cur = cur->next;
		}
		if (cur != NULL)
			return pos;
		return 0;
	}
	void find(T val, std::vector<T>* list, bool(*cmp)(const T& a, const T& b) = equal) {
		Node* cur = head->next;
		int pos = 1;
		while (cur != NULL) {
			if ((*cmp)(cur->val, val))
				list->push_back(cur->val);
			++pos;
			cur = cur->next;
		}
	}
	T get(int pos) {
		if (pos > len || pos <= 0)
			throw InvalidPosException();
		Node * cur = head->next;
		int cur_pos = 1;
		while (cur != NULL) {
			if (cur_pos == pos)
				break;
			++cur_pos;
			cur = cur->next;
		}
		if (cur == NULL)
			throw std::exception();
		return cur->val;
	}
	void map(void(*func)(const T& val)) {
		Node* cur = head->next;
		while (cur != NULL) {
			func(cur->val);
			cur = cur->next;
		}
	}
};

