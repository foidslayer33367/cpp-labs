#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

template <typename T>
bool isEqual(T a, T b) {
    return a == b;
}

template <typename Tin, typename Tout>
class Mapper {
private:
    std::vector<std::pair<Tin, Tout>> mappings;

public:
    Mapper() = default;

    // Поиск существующего ключа занимает O(m).
    void addMapping(const Tin& input, const Tout& output) {
        for (auto& mapping : mappings) {
            if (mapping.first == input) {
                mapping.second = output;
                return;
            }
        }
        mappings.emplace_back(input, output);
    }

    Tout apply(const Tin& input) const {
        for (const auto& mapping : mappings) {
            if (mapping.first == input) {
                return mapping.second;
            }
        }
        throw std::out_of_range("Для значения нет соответствия");
    }
};

template <typename T>
class SortedList {
private:
    struct Node {
        T value;
        Node* next;

        Node(const T& item, Node* following) : value(item), next(following) {}
    };

    Node* head = nullptr;
    std::size_t count = 0;

public:
    SortedList() = default;
    SortedList(const SortedList&) = delete;
    SortedList& operator=(const SortedList&) = delete;

    ~SortedList() {
        // Итеративное освобождение не увеличивает глубину стека.
        while (head != nullptr) {
            Node* removed = head;
            head = head->next;
            delete removed;
        }
    }

    // O(n): найти место и вставить один узел.
    void insert(const T& value) {
        Node* previous = nullptr;
        Node* current = head;
        while (current != nullptr && current->value < value) {
            previous = current;
            current = current->next;
        }

        Node* added = new Node(value, current);
        if (previous == nullptr) {
            head = added;
        } else {
            previous->next = added;
        }
        ++count;
    }

    // O(n): проход останавливается, когда искомое значение уже пропущено.
    bool contains(const T& value) const {
        Node* current = head;
        while (current != nullptr && current->value < value) {
            current = current->next;
        }
        return current != nullptr && current->value == value;
    }

    // Удаляется одно вхождение. Поиск - O(n), отсоединение узла - O(1).
    bool remove(const T& value) {
        Node* previous = nullptr;
        Node* current = head;
        while (current != nullptr && current->value < value) {
            previous = current;
            current = current->next;
        }
        if (current == nullptr || !(current->value == value)) {
            return false;
        }
        if (previous == nullptr) {
            head = current->next;
        } else {
            previous->next = current->next;
        }
        delete current;
        --count;
        return true;
    }

    const T& getByIndex(std::size_t index) const {
        if (index >= count) {
            throw std::out_of_range("Индекс за пределами списка");
        }
        Node* current = head;
        for (std::size_t i = 0; i < index; ++i) {
            current = current->next;
        }
        return current->value;
    }

    std::size_t size() const {
        return count;
    }

    void print() const {
        for (Node* current = head; current != nullptr; current = current->next) {
            std::cout << current->value << ' ';
        }
        std::cout << '\n';
    }
};

// Один наблюдаемый результат на серию не даёт удалить вычисления при оптимизации.
volatile std::uint64_t benchmarkResult = 0;

template <typename Action>
double measureTime(Action action) {
    const auto start = std::chrono::steady_clock::now();
    benchmarkResult = action();
    const auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(finish - start).count();
}

std::array<double, 8> runBenchmark(const std::vector<int>& values,
                                   const std::vector<int>& queries,
                                   const std::vector<std::size_t>& indexes) {
    SortedList<int> own;
    std::multiset<int> standard;
    std::array<double, 8> times{};

    times[0] = measureTime([&] {
        for (int value : values) {
            own.insert(value);
        }
        return own.size();
    });
    times[1] = measureTime([&] {
        for (int value : values) {
            standard.insert(value);
        }
        return standard.size();
    });
    times[2] = measureTime([&] {
        std::uint64_t found = 0;
        for (int value : queries) {
            found += own.contains(value);
        }
        return found;
    });
    times[3] = measureTime([&] {
        std::uint64_t found = 0;
        for (int value : queries) {
            found += standard.find(value) != standard.end();
        }
        return found;
    });
    times[4] = measureTime([&] {
        std::uint64_t sum = 0;
        for (std::size_t index : indexes) {
            sum += own.getByIndex(index);
        }
        return sum;
    });
    times[5] = measureTime([&] {
        std::uint64_t sum = 0;
        for (std::size_t index : indexes) {
            auto position = standard.begin();
            std::advance(position, static_cast<std::ptrdiff_t>(index));
            sum += *position;
        }
        return sum;
    });
    times[6] = measureTime([&] {
        std::uint64_t removed = 0;
        for (int value : values) {
            removed += own.remove(value);
        }
        return removed;
    });
    times[7] = measureTime([&] {
        std::uint64_t removed = 0;
        for (int value : values) {
            auto position = standard.find(value);
            if (position != standard.end()) {
                standard.erase(position); // Как и remove(), удаляем одно вхождение.
                ++removed;
            }
        }
        return removed;
    });
    return times;
}

void demonstrateTemplates() {
    std::cout << std::boolalpha;
    std::cout << "isEqual<int>(7, 7): " << isEqual(7, 7) << '\n';
    std::cout << "isEqual<double>(1.5, 2.5): " << isEqual(1.5, 2.5) << '\n';
    std::cout << "isEqual<string>: "
              << isEqual(std::string("cat"), std::string("cat")) << '\n';

    Mapper<int, std::string> mapper;
    mapper.addMapping(1, "один");
    mapper.addMapping(2, "два");
    mapper.addMapping(2, "второй");
    std::cout << "Mapper(1): " << mapper.apply(1) << '\n';
    std::cout << "Mapper(2) после замены: " << mapper.apply(2) << '\n';
    try {
        mapper.apply(3);
    } catch (const std::out_of_range& error) {
        std::cout << "Ошибка Mapper: " << error.what() << '\n';
    }

    SortedList<int> numbers;
    for (int value : {5, 1, 3, 3, -2}) {
        numbers.insert(value);
    }
    std::cout << "\nSortedList<int>: ";
    numbers.print();
    std::cout << "contains(3): " << numbers.contains(3)
              << ", contains(8): " << numbers.contains(8) << '\n';
    std::cout << "getByIndex(2): " << numbers.getByIndex(2) << '\n';
    std::cout << "remove(3): " << numbers.remove(3) << ", список: ";
    numbers.print();
    std::cout << "remove(99): " << numbers.remove(99) << '\n';
    try {
        numbers.getByIndex(numbers.size());
    } catch (const std::out_of_range& error) {
        std::cout << "Ошибка индекса: " << error.what() << '\n';
    }

    SortedList<double> fractions;
    fractions.insert(2.5);
    fractions.insert(-1.25);
    std::cout << "SortedList<double>: ";
    fractions.print();

    SortedList<std::string> words;
    words.insert("pear");
    words.insert("apple");
    words.insert("banana");
    std::cout << "SortedList<string>: ";
    words.print();

    SortedList<int> empty;
    std::cout << "Пустой список: size=" << empty.size()
              << ", contains(1)=" << empty.contains(1)
              << ", remove(1)=" << empty.remove(1) << '\n';
    try {
        empty.getByIndex(0);
    } catch (const std::out_of_range& error) {
        std::cout << "Ошибка пустого списка: " << error.what() << '\n';
    }
}

int main() {
    demonstrateTemplates();
    std::cout << "\nЗамеры, мс. В каждой паре сначала SortedList, затем multiset.\n";
    std::cout << "n      insert / STL      contains / STL    index / STL       remove / STL\n";
    std::cout << std::fixed << std::setprecision(3);

    for (int n : {1000, 5000, 10000}) {
        std::mt19937 generator(42);
        std::uniform_int_distribution<int> distribution(0, n);
        std::uniform_int_distribution<std::size_t> indexDistribution(0, n - 1);
        std::vector<int> values(n);
        std::vector<int> queries(1000);
        std::vector<std::size_t> indexes(1000);
        for (int& value : values) {
            value = distribution(generator);
        }
        for (int& query : queries) {
            query = distribution(generator);
        }
        for (auto& index : indexes) {
            index = indexDistribution(generator);
        }

        std::array<std::array<double, 8>, 3> runs;
        for (auto& run : runs) {
            run = runBenchmark(values, queries, indexes);
        }
        std::cout << std::left << std::setw(7) << n;
        for (std::size_t operation = 0; operation < 8; ++operation) {
            std::array<double, 3> samples{
                runs[0][operation], runs[1][operation], runs[2][operation]
            };
            std::sort(samples.begin(), samples.end());
            std::cout << std::right << std::setw(9) << samples[1];
        }
        std::cout << '\n';
    }
    return 0;
}
