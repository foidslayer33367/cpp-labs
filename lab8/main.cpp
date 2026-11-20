#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using Array = std::vector<int>;
using Index = std::optional<std::size_t>;

std::size_t lowerPosition(const Array& a, int key) {
    std::size_t left = 0, right = a.size();
    while (left < right) {
        const auto mid = left + (right - left) / 2;
        // Правая граница не входит в рассматриваемый диапазон.
        if (a[mid] < key) left = mid + 1;
        else right = mid;
    }
    return left;
}

Index binarySearch(const Array& a, int key) {
    const auto pos = lowerPosition(a, key);
    if (pos < a.size() && a[pos] == key) return pos;
    return std::nullopt;
}

Index binaryRecursive(const Array& a, int key, std::size_t left, std::size_t right) {
    if (left == right) return std::nullopt;
    const auto mid = left + (right - left) / 2;
    if (a[mid] == key) return mid;
    if (a[mid] < key) return binaryRecursive(a, key, mid + 1, right);
    return binaryRecursive(a, key, left, mid);
}

Index nearest(const Array& a, int key) {
    if (a.empty()) return std::nullopt;
    const auto right = lowerPosition(a, key);
    if (right == 0 || (right < a.size() && a[right] == key)) return right;
    if (right == a.size()) return right - 1;
    // Расширяем тип до вычитания, чтобы обработать INT_MIN и INT_MAX.
    const auto toLeft = static_cast<std::int64_t>(key) - a[right - 1];
    const auto toRight = static_cast<std::int64_t>(a[right]) - key;
    return toLeft <= toRight ? right - 1 : right;
}

class HashTable {
    enum class State { empty, occupied, deleted };
    struct Slot {
        State state = State::empty;
        std::string key, value;
    };
    std::vector<Slot> slots;
    std::size_t count = 0;

    std::size_t firstPosition(std::string_view key) const {
        std::uint64_t hash = 14695981039346656037ULL;
        for (unsigned char byte : key) {
            hash ^= byte;
            hash *= 1099511628211ULL;
        }
        return static_cast<std::size_t>(hash % slots.size());
    }

    std::size_t nextPosition(std::size_t pos) const noexcept {
        return pos + 1 == slots.size() ? 0 : pos + 1;
    }

    Index locate(std::string_view key) const {
        auto pos = firstPosition(key);
        for (std::size_t tried = 0; tried < slots.size(); ++tried) {
            const auto& slot = slots[pos];
            if (slot.state == State::empty) return std::nullopt;
            if (slot.state == State::occupied && slot.key == key) return pos;
            // Удалённая позиция не прерывает цепочку проб.
            pos = nextPosition(pos);
        }
        return std::nullopt;
    }

    void place(std::size_t pos, const std::string& key, const std::string& value) {
        slots[pos] = Slot{State::occupied, key, value};
        ++count;
    }

public:
    explicit HashTable(std::size_t capacity) : slots(capacity) {
        if (capacity == 0) throw std::invalid_argument("Ёмкость должна быть положительной");
    }

    std::size_t size() const noexcept { return count; }
    std::size_t bucket(std::string_view key) const { return firstPosition(key); }

    void insert(const std::string& key, const std::string& value) {
        auto pos = firstPosition(key);
        Index firstDeleted;
        for (std::size_t tried = 0; tried < slots.size(); ++tried) {
            auto& slot = slots[pos];
            if (slot.state == State::occupied && slot.key == key) {
                slot.value = value;
                return;
            }
            if (slot.state == State::deleted && !firstDeleted) firstDeleted = pos;
            if (slot.state == State::empty) {
                place(firstDeleted.value_or(pos), key, value);
                return;
            }
            pos = nextPosition(pos);
        }
        if (!firstDeleted) throw std::overflow_error("Хеш-таблица заполнена");
        place(*firstDeleted, key, value);
    }

    const std::string* find(std::string_view key) const {
        const auto pos = locate(key);
        return pos ? &slots[*pos].value : nullptr;
    }

    bool remove(std::string_view key) {
        const auto pos = locate(key);
        if (!pos) return false;
        slots[*pos] = Slot{State::deleted, {}, {}};
        --count;
        return true;
    }
};

void demonstrateSearch() {
    const Array data{-20, -5, 0, 10, 10, 18, 30};
    std::cout << "Отсортированный массив: -20 -5 0 10 10 18 30\n";
    for (int key : {-30, 6, 10, 14, 99}) {
        const auto pos = nearest(data, key);
        std::cout << "Ближайший к " << key << ": " << data[*pos] << ", индекс " << *pos << '\n';
    }
    std::cout << "Точный поиск 9: " << (binarySearch(data, 9) ? "найден" : "не найден") << '\n'
              << "Рекурсивный поиск 18, индекс: " << *binaryRecursive(data, 18, 0, data.size())
              << "\nПустой массив: " << (nearest({}, 5) ? "найден" : "нет результата") << '\n';
}

void demonstrateHash() {
    HashTable table(5);
    std::vector<std::string> keys;
    for (int i = 0; keys.size() < 3; ++i) {
        const std::string key = "key-" + std::to_string(i);
        if (table.bucket(key) == 4) keys.push_back(key);
    }
    for (std::size_t i = 0; i < keys.size(); ++i)
        table.insert(keys[i], "значение " + std::to_string(i + 1));

    std::cout << "Коллизия в корзине 4: " << keys[0] << ", " << keys[1] << ", " << keys[2] << '\n';
    table.remove(keys[0]);
    std::cout << "После удаления первого ключа найден " << keys[1] << ": "
              << *table.find(keys[1]) << '\n';
    table.insert(keys[1], "обновлено");
    std::cout << "Обновление за удалённой ячейкой: " << *table.find(keys[1])
              << ", размер: " << table.size() << '\n';
    table.insert(keys[0], "добавлен снова");
    std::cout << "Повторная вставка: " << *table.find(keys[0])
              << "\nНесуществующий ключ: " << (table.find("missing") ? "найден" : "не найден") << '\n';

    HashTable full(1);
    full.insert("a", "1");
    try {
        full.insert("b", "2");
    } catch (const std::overflow_error& error) {
        std::cout << "Проверка заполнения: " << error.what() << '\n';
    }
}

template <class Query, class Lookup>
double medianTime(const std::vector<Query>& queries, Lookup lookup, std::size_t expected) {
    std::array<double, 9> samples{};
    for (double& ms : samples) {
        std::size_t found = 0;
        const auto start = std::chrono::steady_clock::now();
        for (const auto& query : queries) found += lookup(query);
        const auto end = std::chrono::steady_clock::now();
        if (found != expected) throw std::logic_error("Неверное число найденных ключей");
        ms = std::chrono::duration<double, std::milli>(end - start).count();
    }
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

void benchmark() {
    constexpr std::size_t queryCount = 10000;
    std::mt19937 rng(42);
    std::cout << "10 000 точных запросов, 50% попаданий, медиана 9 серий, мс\n"
              << "      N | Двоичный |      Хеш\n" << std::fixed << std::setprecision(4);
    for (int n : {1000, 10000, 100000}) {
        Array data(n);
        HashTable table(2 * n + 1);
        for (int i = 0; i < n; ++i) {
            data[i] = 2 * i;
            const auto key = std::to_string(data[i]);
            table.insert(key, key);
        }

        std::vector<int> queries;
        std::vector<std::string> strings;
        std::uniform_int_distribution<int> distribution(0, n - 1);
        for (std::size_t i = 0; i < queryCount; ++i) {
            const int key = 2 * distribution(rng) + static_cast<int>(i % 2);
            queries.push_back(key);
            strings.push_back(std::to_string(key));
        }
        const double binary = medianTime(queries, [&](int key) {
            return binarySearch(data, key).has_value();
        }, queryCount / 2);
        const double hash = medianTime(strings, [&](const std::string& key) {
            return table.find(key) != nullptr;
        }, queryCount / 2);
        std::cout << std::setw(7) << n << " | " << std::setw(8) << binary
                  << " | " << std::setw(8) << hash << '\n';
    }
}

int main() {
    demonstrateSearch();
    std::cout << '\n';
    demonstrateHash();
    std::cout << '\n';
    benchmark();
    return 0;
}
