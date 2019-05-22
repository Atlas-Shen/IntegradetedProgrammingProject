#pragma once

#include <utility>
#include <cstddef>

//single linked list implementation, limited functions ver.
//it can also be used as a stack or a queue
template <typename T>
class List {

	struct Node {
		T element;
		Node *next;

		Node(const T &element, Node *node) : element(element), next(node) {}

		Node(T &&element, Node *node) : element(std::move(element)), next(node) {}
	};

public:

	//const iterator, actually. For convenience, use iterator
	class Iterator {
		
	public:
        Iterator() : current(nullptr) {}

		//make sure current is not nullptr
        const T &element() const {
            return current->element;
		}

        bool isValid() const {
            return current != nullptr;
		}

		void next() {
            current = current->next;
		}

	private:
        Node *current;

        Iterator(Node *p) : current(p) {}

		friend class List<T>;
	};

	//constructors and destructor
	List();
	List(const List &);
	List(List &&) noexcept;
	~List();
	List &operator=(const List &);
	List &operator=(List &&) noexcept;

	//capacity
    bool isEmpty() const;
    unsigned size() const;

	//element access
    bool doesContain(const T &) const;
    const T &front() const; //make sure the list is not empty
    T &front();
    const T &back() const; //make sure the list is not empty
    T &back();
    Iterator iterator() const;

	//modifiers
    void addFront(const T &);
    void addFront(T &&);
    void removeFront(); //make sure the list is not empty
    void addBack(const T &);
    void addBack(T &&);
    void removeFirstOf(const T &);
    void clear();

private:
    Node *head;
    Node *tail;
    unsigned mSize;

	void free();
};

template <typename T>
List<T>::List() : head(nullptr), tail(nullptr), mSize(0) {}

template <typename T>
List<T>::List(const List &list) : head(nullptr), tail(nullptr), mSize(list.mSize) {
    Node **piter = &head;
    Node *iter = list.head;
	while (iter != nullptr) {
        *piter = new Node(iter->element, nullptr);
        tail = *piter;
        piter = &(*piter)->next;
		iter = iter->next;
	}
}

template <typename T>
List<T>::List(List &&list) noexcept : head(list.head), tail(list.tail), mSize(list.mSize) {
    list.head = nullptr;
    list.tail = nullptr;
    list.mSize = 0;
}

template <typename T>
List<T>::~List() {
	free();
}

template <typename T>
List<T>& List<T>::operator=(const List &list) {
	List copy = list;
	std::swap(*this, copy);
	return *this;
}

template <typename T>
List<T>& List<T>::operator=(List &&list) noexcept {
    std::swap(head, list.head);
    std::swap(tail, list.tail);
    std::swap(mSize, list.mSize);
	return *this;
}

template <typename T>
bool List<T>::isEmpty() const {
    return mSize == 0;
}

template <typename T>
unsigned List<T>::size() const {
    return mSize;
}

template <typename T>
bool List<T>::doesContain(const T &element) const {
    Node *iter = head;
	while (iter != nullptr) {
		if (iter->element == element)
			return true;
		iter = iter->next;
	}
	return false;
}

template <typename T>
const T &List<T>::front() const {
    return head->element;
}

template <typename T>
T &List<T>::front() {
    return head->element;
}

template <typename T>
const T &List<T>::back() const {
    return tail->element;
}

template <typename T>
T &List<T>::back() {
    return tail->element;
}

template <typename T>
typename List<T>::Iterator List<T>::iterator() const {
    return Iterator(head);
}

template <typename T>
void List<T>::addFront(const T &element) {
    head = new Node(element, head);
    ++mSize;
    if (mSize == 1)
        tail = head;
}

template <typename T>
void List<T>::addFront(T &&element) {
    head = new Node(std::move(element), head);
    ++mSize;
    if (mSize == 1)
        tail = head;
}

template <typename T>
void List<T>::removeFront() {
    Node *nodeToDelete = head;
    head = head->next;
    delete nodeToDelete;
    --mSize;
    if (head == nullptr)
        tail = nullptr;
}

template <typename T>
void List<T>::addBack(const T &element) {
    if (tail == nullptr) {
        tail = new Node(element, nullptr);
        head = tail;
	}
	else {
        tail->next = new Node(element, nullptr);
        tail = tail->next;
	}
    ++mSize;
}

template <typename T>
void List<T>::addBack(T &&element) {
    if (tail == nullptr) {
        tail = new Node(std::move(element), nullptr);
        head = tail;
	}
	else {
        tail->next = new Node(std::move(element), nullptr);
        tail = tail->next;
	}
    ++mSize;
}

template <typename T>
void List<T>::removeFirstOf(const T &element) {
    Node **piter = &head;
    while (*piter != nullptr) {
        if ((*piter)->element == element) {
            Node *nodeToDelete = *piter;
            *piter = (*piter)->next;
            delete nodeToDelete;
            --mSize;
            if (*piter == nullptr)
                tail = reinterpret_cast<Node *>(reinterpret_cast<char *>(piter) - reinterpret_cast<size_t>(&(reinterpret_cast<Node *>(0)->next)));
			break;
		}
        piter = &(*piter)->next;
	}
}

template <typename T>
void List<T>::clear() {
    if (mSize != 0) {
		free();
        head = nullptr;
        tail = nullptr;
        mSize = 0;
	}
}

template <typename T>
void List<T>::free() {
    Node *iter = head;
	while (iter != nullptr) {
        Node *nodeToDelete = iter;
		iter = iter->next;
        delete nodeToDelete;
	}
}
