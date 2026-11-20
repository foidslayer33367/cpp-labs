#include <cctype>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

std::size_t grownCapacity(std::size_t capacity) {
    if (capacity > std::numeric_limits<std::size_t>::max() / 2)
        throw std::length_error("Слишком большая ёмкость");
    return capacity == 0 ? 4 : capacity * 2;
}

template <class T>
class Deque {
    std::unique_ptr<std::optional<T>[]> data;
    std::size_t capacity = 0, first = 0, count = 0;

    std::size_t physical(std::size_t i) const {
        return i < capacity - first ? first + i : i - (capacity - first);
    }
    void requireElement() const {
        if (isEmpty()) throw std::underflow_error("Дек пуст");
    }
    void grow() {
        const auto next = grownCapacity(capacity);
        auto buffer = std::make_unique<std::optional<T>[]>(next);
        // Переносим кольцо в новый буфер в логическом порядке.
        for (std::size_t i = 0; i < count; ++i)
            buffer[i].emplace(std::move_if_noexcept(*data[physical(i)]));
        data = std::move(buffer);
        capacity = next;
        first = 0;
    }

public:
    bool isEmpty() const noexcept { return count == 0; }
    std::size_t size() const noexcept { return count; }
    const T& front() const { requireElement(); return *data[first]; }
    const T& back() const { requireElement(); return *data[physical(count - 1)]; }

    void push_back(const T& value) {
        if (count == capacity) {
            T saved(value); // value может лежать в старом буфере.
            grow();
            data[physical(count)].emplace(std::move_if_noexcept(saved));
        } else {
            data[physical(count)].emplace(value);
        }
        ++count;
    }
    void push_front(const T& value) {
        if (count == capacity) {
            T saved(value);
            grow();
            const auto position = first == 0 ? capacity - 1 : first - 1;
            data[position].emplace(std::move_if_noexcept(saved));
            first = position;
        } else {
            const auto position = first == 0 ? capacity - 1 : first - 1;
            data[position].emplace(value);
            first = position;
        }
        ++count;
    }
    T pop_back() {
        requireElement();
        auto& slot = data[physical(count - 1)];
        T value(std::move_if_noexcept(*slot));
        slot.reset();
        --count;
        return value;
    }
    T pop_front() {
        requireElement();
        auto& slot = data[first];
        T value(std::move_if_noexcept(*slot));
        slot.reset();
        first = first + 1 == capacity ? 0 : first + 1;
        --count;
        return value;
    }

    Deque() = default;
    Deque(const Deque&) = delete;
    Deque& operator=(const Deque&) = delete;
};

// Стек использует только правый конец собственного массива.
template <class T>
class Stack {
    Deque<T> items;
public:
    void push(const T& value) { items.push_back(value); }
    T pop() { return items.pop_back(); }
    const T& top() const { return items.back(); }
    bool isEmpty() const noexcept { return items.isEmpty(); }
    std::size_t size() const noexcept { return items.size(); }
};

// Очередь добавляет справа и удаляет слева.
template <class T>
class Queue {
    Deque<T> items;
public:
    void enqueue(const T& value) { items.push_back(value); }
    T dequeue() { return items.pop_front(); }
    const T& front() const { return items.front(); }
    bool isEmpty() const noexcept { return items.isEmpty(); }
    std::size_t size() const noexcept { return items.size(); }
};

class PriorityQueue {
    std::unique_ptr<int[]> heap;
    std::size_t capacity = 0, count = 0;

    void reserve(std::size_t requested) {
        if (requested <= capacity) return;
        auto buffer = std::make_unique<int[]>(requested);
        for (std::size_t i = 0; i < count; ++i) buffer[i] = heap[i];
        heap = std::move(buffer);
        capacity = requested;
    }
    void heapifyUp(std::size_t index) {
        while (index > 0) {
            const auto parent = (index - 1) / 2;
            if (heap[parent] >= heap[index]) break;
            std::swap(heap[parent], heap[index]);
            index = parent;
        }
    }
    void heapifyDown(std::size_t index) {
        while (index < count / 2) {
            const auto left = 2 * index + 1, right = left + 1;
            const auto larger = right < count && heap[right] > heap[left] ? right : left;
            if (heap[index] >= heap[larger]) break;
            std::swap(heap[index], heap[larger]);
            index = larger;
        }
    }

public:
    PriorityQueue() = default;
    PriorityQueue(const PriorityQueue&) = delete;
    PriorityQueue& operator=(const PriorityQueue&) = delete;
    bool isEmpty() const noexcept { return count == 0; }
    std::size_t size() const noexcept { return count; }
    int top() const {
        if (isEmpty()) throw std::underflow_error("Приоритетная очередь пуста");
        return heap[0];
    }
    void push(int value) {
        if (count == capacity) reserve(grownCapacity(capacity));
        heap[count] = value;
        heapifyUp(count++);
    }
    int pop() {
        const int maximum = top();
        heap[0] = heap[--count];
        heapifyDown(0);
        return maximum;
    }
    void merge(const PriorityQueue& other) {
        const auto original = count;
        const auto extra = other.count;
        if (extra == 0) return;
        if (extra > std::numeric_limits<std::size_t>::max() - original)
            throw std::length_error("Слишком большая очередь");
        reserve(original + extra);
        for (std::size_t i = 0; i < extra; ++i) heap[original + i] = other.heap[i];
        count = original + extra;
        // Достраиваем общую кучу снизу вверх за линейное время.
        for (std::size_t i = count / 2; i > 0; --i) heapifyDown(i - 1);
    }
};

int precedence(char op) {
    if (op == '~') return 3;
    if (op == '*' || op == '/') return 2;
    return 1;
}

void apply(Stack<double>& values, char op) {
    if (op == '~') {
        values.push(-values.pop());
        return;
    }
    const double right = values.pop(), left = values.pop();
    if (op == '/' && right == 0) throw std::domain_error("Деление на ноль");
    double result;
    switch (op) {
        case '+': result = left + right; break;
        case '-': result = left - right; break;
        case '*': result = left * right; break;
        default: result = left / right;
    }
    if (!std::isfinite(result)) throw std::overflow_error("Результат вне диапазона double");
    values.push(result);
}

double evaluate(const std::string& expression) {
    Stack<double> values;
    Stack<char> ops;
    bool needOperand = true;
    auto reduce = [&] { apply(values, ops.pop()); };
    auto digit = [](char c) { return c >= '0' && c <= '9'; };

    for (std::size_t i = 0; i < expression.size();) {
        const char c = expression[i];
        if (std::isspace(static_cast<unsigned char>(c))) { ++i; continue; }
        if (needOperand && c == '-') {
            ops.push('~'); // Отдельный оператор с одним аргументом.
        } else if (needOperand && c == '(') {
            ops.push(c);
        } else if (needOperand && (digit(c) || c == '.')) {
            const auto begin = i;
            while (i < expression.size() && digit(expression[i])) ++i;
            if (i < expression.size() && expression[i] == '.') {
                ++i;
                while (i < expression.size() && digit(expression[i])) ++i;
            }
            values.push(std::stod(expression.substr(begin, i - begin)));
            needOperand = false;
            continue;
        } else if (!needOperand && c == ')') {
            while (!ops.isEmpty() && ops.top() != '(') reduce();
            if (ops.isEmpty()) throw std::invalid_argument("Лишняя закрывающая скобка");
            ops.pop();
        } else if (!needOperand && (c == '+' || c == '-' || c == '*' || c == '/')) {
            while (!ops.isEmpty() && ops.top() != '(' && precedence(ops.top()) >= precedence(c))
                reduce();
            ops.push(c);
            needOperand = true;
        } else {
            throw std::invalid_argument("Некорректное выражение");
        }
        ++i;
    }
    if (needOperand) throw std::invalid_argument("Ожидалось число");
    while (!ops.isEmpty()) {
        if (ops.top() == '(') throw std::invalid_argument("Незакрытая скобка");
        reduce();
    }
    return values.pop();
}

void removeOdd(Queue<int>& queue) {
    const auto original = queue.size();
    // Обрабатываем только исходные элементы, сохраняя порядок чётных.
    for (std::size_t i = 0; i < original; ++i) {
        const int value = queue.dequeue();
        if (value % 2 == 0) queue.enqueue(value);
    }
}

void windowMax(const int* values, std::size_t size, std::size_t k,
                      Queue<int>& maxima) {
    if (k == 0 || k > size) throw std::invalid_argument("Некорректная длина окна");
    Deque<std::size_t> indices;
    for (std::size_t i = 0; i < size; ++i) {
        if (i >= k && !indices.isEmpty() && indices.front() <= i - k) indices.pop_front();
        while (!indices.isEmpty() && values[indices.back()] <= values[i]) indices.pop_back();
        indices.push_back(i);
        // Максимум окна стоит в начале дека индексов.
        if (i >= k - 1) maxima.enqueue(values[indices.front()]);
    }
}

template <class Action>
void showError(const char* label, Action action) {
    try {
        action();
    } catch (const std::exception& error) {
        std::cout << label << ": " << error.what() << '\n';
    }
}

int main() {
    std::cout << "Стек — выражения\n";
    for (const std::string expr : {"-5 + 3", "2 * -(3 + 4)", "10 - -2", "--5 + 2.5"})
        std::cout << expr << " = " << evaluate(expr) << '\n';
    showError("Деление на ноль", [] { evaluate("4 / -(2 - 2)"); });
    showError("Пустой стек", [] { Stack<int> s; s.pop(); });

    Queue<int> queue;
    for (int value : {1, 2, -3, 4, 0, 5, -8, 6, 9}) queue.enqueue(value);
    removeOdd(queue);
    std::cout << "\nОчередь после удаления нечётных:";
    while (!queue.isEmpty()) std::cout << ' ' << queue.dequeue();
    std::cout << '\n';
    showError("Пустая очередь", [&] { queue.dequeue(); });

    const int values[] = {1, 3, -1, -3, 5, 3, 6, 7};
    Queue<int> maxima;
    windowMax(values, 8, 3, maxima);
    std::cout << "\nДек — максимумы окон длины 3:";
    while (!maxima.isEmpty()) std::cout << ' ' << maxima.dequeue();
    std::cout << '\n';
    showError("Окно длины 0", [&] { windowMax(values, 8, 0, maxima); });
    showError("Пустой дек", [] { Deque<int> d; d.pop_front(); });

    PriorityQueue first, second;
    for (int value : {7, 2, 9, 1}) first.push(value);
    for (int value : {8, 2, 10, -3, 4}) second.push(value);
    first.merge(second);
    std::cout << "\nПосле merge, максимум: " << first.top()
              << ", размер: " << first.size() << '\n';
    std::cout << "Размер второй очереди: " << second.size() << '\n';
    std::cout << "Извлечение:";
    while (!first.isEmpty()) std::cout << ' ' << first.pop();
    std::cout << '\n';
    showError("Пустая приоритетная очередь", [&] { first.pop(); });
    return 0;
}
