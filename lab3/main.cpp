#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

template <typename T>
class Vector {
    using Allocator = std::allocator<T>;
    using Traits = std::allocator_traits<Allocator>;

    Allocator allocator;
    T* data = nullptr;
    std::size_t used = 0;
    std::size_t reserved = 0;
    std::size_t relocated = 0;

    void clearElements() noexcept {
        while (used != 0) {
            Traits::destroy(allocator, data + --used);
        }
    }

    void swap(Vector& other) noexcept {
        std::swap(data, other.data);
        std::swap(used, other.used);
        std::swap(reserved, other.reserved);
        std::swap(relocated, other.relocated);
    }

public:
    Vector() = default;

    Vector(std::size_t count, const T& value) : Vector() {
        reserve(count);
        for (; used < count; ++used) {
            Traits::construct(allocator, data + used, value);
        }
    }

    Vector(const Vector& other) : Vector() {
        reserve(other.used);
        for (const T& value : other) {
            push_back(value);
        }
    }

    Vector(Vector&& other) noexcept {
        swap(other);
    }

    ~Vector() {
        clearElements();
        if (data != nullptr) {
            Traits::deallocate(allocator, data, reserved);
        }
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            // Сначала создаём копию, затем отдаём ей старый буфер.
            Vector copy(other);
            swap(copy);
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            Vector moved(std::move(other));
            swap(moved);
        }
        return *this;
    }

    std::size_t size() const noexcept { return used; }
    std::size_t capacity() const noexcept { return reserved; }
    bool empty() const noexcept { return used == 0; }
    std::size_t relocationCount() const noexcept { return relocated; }

    // Указатели подходят как итераторы: элементы лежат подряд.
    T* begin() noexcept { return data; }
    const T* begin() const noexcept { return data; }
    T* end() noexcept { return used == 0 ? data : data + used; }
    const T* end() const noexcept { return used == 0 ? data : data + used; }

    T& at(std::size_t index) {
        if (index >= used) {
            throw std::out_of_range("Индекс за пределами массива");
        }
        return data[index];
    }

    const T& at(std::size_t index) const {
        if (index >= used) {
            throw std::out_of_range("Индекс за пределами массива");
        }
        return data[index];
    }

    void reserve(std::size_t count) {
        if (count <= reserved) {
            return;
        }
        Vector buffer;
        buffer.data = Traits::allocate(buffer.allocator, count);
        buffer.reserved = count;
        // Временный вектор освободит память, если создание элемента бросит исключение.
        for (; buffer.used < used; ++buffer.used) {
            Traits::construct(buffer.allocator, buffer.data + buffer.used,
                              std::move_if_noexcept(data[buffer.used]));
        }
        buffer.relocated = relocated + used;
        swap(buffer);
    }

    void push_back(const T& value) {
        if (used == reserved) {
            const std::size_t limit = Traits::max_size(allocator);
            if (reserved == limit) {
                throw std::length_error("Достигнут предельный размер массива");
            }
            const std::size_t next = reserved == 0 ? 1 :
                                     reserved > limit / 2 ? limit : reserved * 2;
            // value может ссылаться на элемент этого же массива.
            T saved(value);
            reserve(next);
            Traits::construct(allocator, data + used, std::move_if_noexcept(saved));
        } else {
            Traits::construct(allocator, data + used, value);
        }
        ++used;
    }

    void unique() {
        // Оставляем первое вхождение каждого значения в исходном порядке.
        std::size_t kept = 0;
        for (std::size_t i = 0; i < used; ++i) {
            if (std::find(data, data + kept, data[i]) == data + kept) {
                if (kept != i) {
                    data[kept] = std::move(data[i]);
                }
                ++kept;
            }
        }
        while (used > kept) {
            Traits::destroy(allocator, data + --used);
        }
    }

    std::size_t partition(const T& pivot) {
        if (empty()) {
            return 0;
        }
        const T boundary(pivot); // pivot может лежать внутри вектора.
        // Слева будут элементы < pivot, справа — элементы >= pivot.
        T* middle = std::partition(begin(), end(), [&boundary](const T& value) {
            return value < boundary;
        });
        return static_cast<std::size_t>(middle - begin());
    }
};

template <typename T>
void print(const char* label, const Vector<T>& values) {
    std::cout << label;
    for (const T& value : values) {
        std::cout << ' ' << value;
    }
    std::cout << '\n';
}

int main() {
    std::cout << std::boolalpha;
    Vector<int> values;
    std::cout << "Пустой: " << values.empty() << ", размер: " << values.size() << '\n';
    values.reserve(8);
    std::cout << "После reserve(8): размер " << values.size()
              << ", ёмкость " << values.capacity() << '\n';

    for (int value : {7, 2, 7, 5, 2, 1, 9, 5}) {
        values.push_back(value);
    }
    values.push_back(values.at(0));
    print("После push_back:", values);
    values.unique();
    print("После unique:", values);

    const int pivot = 5;
    const std::size_t middle = values.partition(pivot);
    std::cout << "partition(5), граница: " << middle << '\n';
    std::cout << "  < 5:";
    for (std::size_t i = 0; i < middle; ++i) {
        std::cout << ' ' << values.at(i);
    }
    std::cout << "\n  >= 5:";
    for (std::size_t i = middle; i < values.size(); ++i) {
        std::cout << ' ' << values.at(i);
    }
    std::cout << '\n';

    std::sort(values.begin(), values.end());
    print("После std::sort:", values);

    Vector<int> copy(values);
    copy.at(0) = -1;
    std::cout << "Копия[0]: " << copy.at(0)
              << ", оригинал[0]: " << values.at(0) << '\n';
    Vector<int> assigned;
    assigned = values;
    Vector<int>& same = assigned;
    assigned = same;
    Vector<int> moved(std::move(copy));
    assigned = std::move(moved);
    print("После перемещения:", assigned);
    std::cout << "Источники перемещения пусты: "
              << (copy.empty() && moved.empty()) << '\n';

    Vector<std::string> words(3, "кот");
    words.push_back("пёс");
    words.unique();
    print("Vector<string>:", words);
    try {
        words.at(words.size());
    } catch (const std::out_of_range& error) {
        std::cout << "Ошибка: " << error.what() << '\n';
    }

    std::cout << "\nРост при push_back без reserve\n"
              << "Размер | Переносы | Операций на вставку\n";
    for (int n : {1000, 10000, 100000}) {
        Vector<int> sample;
        for (int i = 0; i < n; ++i) {
            sample.push_back(i);
        }
        const double average = 1.0 + static_cast<double>(sample.relocationCount()) / n;
        std::cout << std::setw(6) << n << " | " << std::setw(8) << sample.relocationCount()
                  << " | " << std::fixed << std::setprecision(3) << average << '\n';
    }
    return 0;
}
