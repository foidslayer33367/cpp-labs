#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdexcept>

template <typename T>
class SinglyLinkedList {
    struct Node { T value; Node* next; };
    Node* head = nullptr;
    Node* tail = nullptr;
    std::size_t count = 0;

    void unlink(Node* node, Node* previous) noexcept {
        // Если удаляем первый или последний узел, обновляем границу списка.
        if (previous) previous->next = node->next;
        else head = node->next;
        if (tail == node) tail = previous;
        delete node;
        --count;
    }

public:
    SinglyLinkedList() = default;
    SinglyLinkedList(const SinglyLinkedList&) = delete;
    SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;
    ~SinglyLinkedList() { clear(); }

    std::size_t size() const noexcept { return count; }
    bool empty() const noexcept { return head == nullptr; }

    void push_front(const T& value) {
        head = new Node{value, head};
        if (!tail) tail = head;
        ++count;
    }
    void push_back(const T& value) {
        Node* added = new Node{value, nullptr};
        if (tail) tail->next = added;
        else head = added;
        tail = added;
        ++count;
    }
    void pop_front() {
        if (empty()) throw std::underflow_error("Пустой односвязный список");
        unlink(head, nullptr);
    }
    void pop_back() {
        if (empty()) throw std::underflow_error("Пустой односвязный список");
        if (head == tail) {
            unlink(head, nullptr);
            return;
        }
        // У хвоста нет ссылки назад, поэтому ищем его предшественника от головы.
        Node* previous = head;
        while (previous->next != tail) previous = previous->next;
        unlink(tail, previous);
    }

    const T* find(const T& value) const {
        for (const Node* node = head; node; node = node->next) {
            if (node->value == value) return &node->value;
        }
        return nullptr;
    }
    bool remove(const T& value) {
        Node* previous = nullptr;
        for (Node* node = head; node; previous = node, node = node->next) {
            if (node->value == value) {
                unlink(node, previous);
                return true;
            }
        }
        return false;
    }
    void print(std::ostream& out = std::cout) const {
        out << '[';
        for (const Node* node = head; node; node = node->next) {
            if (node != head) out << ", ";
            out << node->value;
        }
        out << "]\n";
    }
    void clear() noexcept {
        while (head) unlink(head, nullptr);
    }
};

template <typename T>
class DoublyLinkedList {
    struct Node { T value; Node* prev; Node* next; };
    Node* head = nullptr;
    Node* tail = nullptr;
    std::size_t count = 0;

    void unlink(Node* node) noexcept {
        // Соединяем соседей с двух сторон и поправляем head или tail на границе.
        if (node->prev) node->prev->next = node->next;
        else head = node->next;
        if (node->next) node->next->prev = node->prev;
        else tail = node->prev;
        delete node;
        --count;
    }

public:
    class const_iterator {
        friend class DoublyLinkedList<T>;
        const DoublyLinkedList* owner = nullptr;
        Node* node = nullptr;
        const_iterator(const DoublyLinkedList* list, Node* current)
            : owner(list), node(current) {}

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator() = default;
        reference operator*() const { return node->value; }
        pointer operator->() const { return &node->value; }
        const_iterator& operator++() { node = node->next; return *this; }
        const_iterator& operator--() {
            // Для --end() текущего узла ещё нет, поэтому берём хвост.
            node = node ? node->prev : owner->tail;
            return *this;
        }
        const_iterator operator++(int) { auto old = *this; ++*this; return old; }
        const_iterator operator--(int) { auto old = *this; --*this; return old; }
        bool operator==(const_iterator other) const {
            return owner == other.owner && node == other.node;
        }
        bool operator!=(const_iterator other) const { return !(*this == other); }
    };

    using reverse_iterator = std::reverse_iterator<const_iterator>;

    DoublyLinkedList() = default;
    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;
    ~DoublyLinkedList() { clear(); }

    std::size_t size() const noexcept { return count; }
    bool empty() const noexcept { return head == nullptr; }
    const_iterator begin() const noexcept { return {this, head}; }
    const_iterator end() const noexcept { return {this, nullptr}; }
    reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

    void push_front(const T& value) {
        Node* added = new Node{value, nullptr, head};
        if (head) head->prev = added;
        else tail = added;
        head = added;
        ++count;
    }
    void push_back(const T& value) {
        Node* added = new Node{value, tail, nullptr};
        if (tail) tail->next = added;
        else head = added;
        tail = added;
        ++count;
    }
    void pop_front() {
        if (empty()) throw std::underflow_error("Пустой двусвязный список");
        unlink(head);
    }
    void pop_back() {
        if (empty()) throw std::underflow_error("Пустой двусвязный список");
        unlink(tail);
    }

    const_iterator find(const T& value) const {
        for (auto it = begin(); it != end(); ++it) {
            if (*it == value) return it;
        }
        return end();
    }

    void insert_after(const_iterator pos, const T& value) {
        if (pos.owner != this || pos == end())
            throw std::invalid_argument("Для вставки нужен узел этого списка");
        Node* node = pos.node;
        Node* added = new Node{value, node, node->next};
        if (node->next) node->next->prev = added;
        else tail = added;
        node->next = added;
        ++count;
    }
    void erase(const_iterator pos) {
        if (pos.owner != this || pos == end())
            throw std::invalid_argument("Для удаления нужен узел этого списка");
        unlink(pos.node);
    }
    bool remove(const T& value) {
        auto pos = find(value);
        if (pos == end()) return false;
        erase(pos);
        return true;
    }

    void removeEverySecond() noexcept {
        Node* kept = head;
        while (kept && kept->next) {
            // Удаляем соседа справа: это позиции 2, 4, 6 исходного списка.
            unlink(kept->next);
            kept = kept->next;
        }
    }

    void print(std::ostream& out = std::cout) const {
        out << '[';
        for (auto it = begin(); it != end(); ++it) {
            if (it != begin()) out << ", ";
            out << *it;
        }
        out << "]\n";
    }
    void printReverse(std::ostream& out = std::cout) const {
        out << '[';
        for (auto it = rbegin(); it != rend(); ++it) {
            if (it != rbegin()) out << ", ";
            out << *it;
        }
        out << "]\n";
    }
    void clear() noexcept {
        while (head) unlink(head);
    }
};

template <typename List>
double medianPopBack(int n) {
    std::array<double, 5> samples{};
    for (double& sample : samples) {
        List list;
        for (int i = 0; i < n; ++i) list.push_back(i);
        const auto start = std::chrono::steady_clock::now();
        while (!list.empty()) list.pop_back();
        const auto end = std::chrono::steady_clock::now();
        sample = std::chrono::duration<double, std::milli>(end - start).count();
    }
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

int main() {
    std::cout << std::boolalpha;
    SinglyLinkedList<int> single;
    single.push_back(20);
    single.push_front(10);
    single.push_back(30);
    single.push_back(40);
    std::cout << "Односвязный: ";
    single.print();
    std::cout << "Найден 30: " << (single.find(30) != nullptr) << '\n';
    single.remove(20);
    single.pop_front();
    single.pop_back();
    std::cout << "После удалений: ";
    single.print();
    single.clear();
    std::cout << "Размер после clear: " << single.size() << '\n';

    DoublyLinkedList<int> dual;
    for (int i = 2; i <= 5; ++i) dual.push_back(i);
    dual.push_front(1);
    dual.insert_after(dual.find(5), 6);
    std::cout << "\nДвусвязный: ";
    dual.print();
    dual.removeEverySecond();
    std::cout << "После removeEverySecond: ";
    dual.print();
    std::cout << "Через reverse_iterator: ";
    dual.printReverse();
    dual.insert_after(dual.find(3), 4);
    dual.erase(dual.find(4));
    dual.pop_back();
    dual.push_back(7);
    dual.pop_front();
    std::cout << "После вставок и удалений: ";
    dual.print();
    dual.clear();
    try {
        dual.pop_back();
    } catch (const std::underflow_error& error) {
        std::cout << "Проверка пустого списка: " << error.what() << '\n';
    }

    std::cout << "\nУдаление всех элементов с конца, медиана 5 серий, мс\n"
              << "Размер | Односвязный | Двусвязный\n" << std::fixed << std::setprecision(3);
    for (int n : {2000, 4000, 8000}) {
        const double singleMs = medianPopBack<SinglyLinkedList<int>>(n);
        const double dualMs = medianPopBack<DoublyLinkedList<int>>(n);
        std::cout << std::setw(6) << n << " | " << std::setw(11) << singleMs
                  << " | " << std::setw(10) << dualMs << '\n';
    }
    return 0;
}
