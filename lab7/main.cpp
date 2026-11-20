#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using Array = std::vector<int>;
struct Frame {
    std::string label;
    Array values;
};
using Trace = std::vector<Frame>;
using Sort = void (*)(Array&, Trace*);

void saveFrame(Trace* trace, const std::string& label, const Array& a) {
    if (trace) trace->push_back({label, a});
}

int medianOfThree(int x, int y, int z) {
    if (x > y) std::swap(x, y);
    if (y > z) std::swap(y, z);
    return std::max(x, y);
}

void quickPart(Array& a, std::size_t lo, std::size_t hi, Trace* trace) {
    while (hi - lo > 1) {
        const int pivot = medianOfThree(a[lo], a[lo + (hi - lo) / 2], a[hi - 1]);
        std::size_t left = lo, i = lo, right = hi;
        // Области: меньше опорного, равны ему, не просмотрены и больше него.
        while (i < right) {
            if (a[i] < pivot) std::swap(a[left++], a[i++]);
            else if (a[i] > pivot) std::swap(a[i], a[--right]);
            else ++i;
        }
        if (trace) saveFrame(trace, "Опорный " + std::to_string(pivot), a);
        // Рекурсивно обрабатываем меньшую часть, чтобы ограничить глубину стека.
        if (left - lo < hi - right) {
            quickPart(a, lo, left, trace);
            lo = right;
        } else {
            quickPart(a, right, hi, trace);
            hi = left;
        }
    }
}

void quickSort(Array& a, Trace* trace = nullptr) {
    quickPart(a, 0, a.size(), trace);
}

void mergePart(Array& a, Array& buffer, std::size_t lo, std::size_t hi, Trace* trace) {
    if (hi - lo < 2) return;
    const std::size_t mid = lo + (hi - lo) / 2;
    mergePart(a, buffer, lo, mid, trace);
    mergePart(a, buffer, mid, hi, trace);
    std::size_t left = lo, right = mid;
    for (std::size_t out = lo; out < hi; ++out) {
        // При равных ключах первым берём элемент левой половины.
        if (left < mid && (right == hi || a[left] <= a[right]))
            buffer[out] = a[left++];
        else
            buffer[out] = a[right++];
    }
    for (std::size_t i = lo; i < hi; ++i) a[i] = buffer[i];
    if (trace) saveFrame(trace, "Слияние " + std::to_string(lo + 1) + "–" + std::to_string(hi), a);
}

void mergeSort(Array& a, Trace* trace = nullptr) {
    Array buffer(a.size()); // Один буфер на всю сортировку.
    mergePart(a, buffer, 0, a.size(), trace);
}

void siftDown(Array& a, std::size_t root, std::size_t end) {
    while (root < end / 2) {
        const auto left = 2 * root + 1, right = left + 1;
        const auto child = right < end && a[right] > a[left] ? right : left;
        if (a[root] >= a[child]) break;
        std::swap(a[root], a[child]);
        root = child;
    }
}

void heapSort(Array& a, Trace* trace = nullptr) {
    for (std::size_t i = a.size() / 2; i > 0; --i) siftDown(a, i - 1, a.size());
    saveFrame(trace, "Построена куча", a);
    for (std::size_t end = a.size(); end > 1; --end) {
        std::swap(a[0], a[end - 1]);
        siftDown(a, 0, end - 1);
        if (trace) saveFrame(trace, "Зафиксирован " + std::to_string(a[end - 1]), a);
    }
}

constexpr std::array<Sort, 3> algorithms{quickSort, mergeSort, heapSort};
constexpr std::array<const char*, 3> names{"Быстрая", "Слиянием", "Пирамидальная"};
constexpr std::array<int, 4> sizes{1000, 2000, 5000, 10000};
using Times = std::array<std::array<double, 3>, sizes.size()>;

double medianTime(Sort sort, const Array& input, const Array& expected) {
    std::array<double, 9> times{};
    for (double& ms : times) {
        auto copy = input;
        const auto start = std::chrono::steady_clock::now();
        sort(copy, nullptr);
        const auto end = std::chrono::steady_clock::now();
        if (copy != expected) throw std::logic_error("Неверный результат сортировки");
        ms = std::chrono::duration<double, std::milli>(end - start).count();
    }
    std::sort(times.begin(), times.end());
    return times[times.size() / 2];
}

Times benchmark() {
    Times result{};
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> distribution(-100000, 100000);
    std::cout << "Медиана 9 запусков на случайном массиве, мс\n"
              << "     N |  Quick Sort |  Merge Sort |   Heap Sort\n"
              << std::fixed << std::setprecision(4);
    for (std::size_t i = 0; i < sizes.size(); ++i) {
        Array data(sizes[i]);
        for (int& x : data) x = distribution(rng);
        auto expected = data;
        std::sort(expected.begin(), expected.end());
        std::cout << std::setw(6) << sizes[i];
        for (std::size_t j = 0; j < algorithms.size(); ++j) {
            result[i][j] = medianTime(algorithms[j], data, expected);
            std::cout << " | " << std::setw(11) << result[i][j];
        }
        std::cout << '\n';
    }
    return result;
}

void text(std::ostream& out, int x, int y, const std::string& label, int size = 17) {
    out << "<text x='" << x << "' y='" << y << "' font-size='" << size << "'>"
        << label << "</text>\n";
}

void drawTrace(const Array& input) {
    std::array<Trace, 3> traces;
    std::size_t rows = 0;
    for (std::size_t col = 0; col < traces.size(); ++col) {
        auto values = input;
        saveFrame(&traces[col], "Начало", values);
        algorithms[col](values, &traces[col]);
        saveFrame(&traces[col], "Готово", values);
        rows = std::max(rows, traces[col].size());
    }
    const auto height = 58 + rows * 98;
    std::ofstream out("steps.svg");
    out.exceptions(std::ios::badbit | std::ios::failbit);
    out << "<svg xmlns='http://www.w3.org/2000/svg' width='780' height='" << height
        << "' font-family='Arial,sans-serif' fill='#182334'>"
        << "<rect width='100%' height='100%' fill='white'/>\n";
    for (std::size_t col = 0; col < traces.size(); ++col) {
        const int left = 15 + static_cast<int>(col) * 260;
        text(out, left, 30, names[col], 23);
        for (std::size_t i = 0; i < traces[col].size(); ++i) {
            const int y = 52 + static_cast<int>(i) * 98;
            const auto& frame = traces[col][i];
            text(out, left, y + 15, std::to_string(i) + ". " + frame.label);
            for (std::size_t j = 0; j < frame.values.size(); ++j) {
                const int x = left + static_cast<int>(j) * 36;
                const int bar = frame.values[j] * 6;
                out << "<rect x='" << x << "' y='" << y + 72 - bar
                    << "' width='26' height='" << bar << "' fill='#4079ad'/>\n";
                text(out, x + 7, y + 89, std::to_string(frame.values[j]), 14);
            }
        }
    }
    out << "</svg>\n";
}

void drawTiming(const Times& times) {
    const std::array<const char*, 3> colors{"#126a98", "#ae4b21", "#23754f"};
    double longest = 0;
    for (const auto& row : times) for (double ms : row) longest = std::max(longest, ms);
    const double limit = longest > 0 ? longest * 1.15 : 1.0;
    auto x = [](int n) { return 105 + (n - sizes.front()) * 570 / (sizes.back() - sizes.front()); };
    auto y = [limit](double ms) { return 340 - static_cast<int>(ms * 255 / limit); };

    std::ofstream out("timing.svg");
    out.exceptions(std::ios::badbit | std::ios::failbit);
    out << "<svg xmlns='http://www.w3.org/2000/svg' width='740' height='410' "
           "font-family='Arial,sans-serif' fill='#182334'>"
           "<rect width='100%' height='100%' fill='white'/>\n";
    text(out, 95, 29, "Время сортировки случайных массивов", 23);
    text(out, 24, 68, "мс");
    text(out, 698, 365, "N");
    for (int tick = 0; tick <= 5; ++tick) {
        const double ms = limit * tick / 5;
        out << "<line x1='100' x2='690' y1='" << y(ms) << "' y2='" << y(ms)
            << "' stroke='#d1dbe4'/>\n";
        auto label = std::to_string(ms);
        text(out, 31, y(ms) + 5, label.substr(0, label.find('.') + 4), 14);
    }
    for (int n : sizes) text(out, x(n) - 17, 365, std::to_string(n), 14);
    for (std::size_t j = 0; j < algorithms.size(); ++j) {
        out << "<polyline fill='none' stroke='" << colors[j] << "' stroke-width='3' points='";
        for (std::size_t i = 0; i < sizes.size(); ++i)
            out << x(sizes[i]) << ',' << y(times[i][j]) << ' ';
        out << "'/>\n";
        for (std::size_t i = 0; i < sizes.size(); ++i)
            out << "<circle cx='" << x(sizes[i]) << "' cy='" << y(times[i][j])
                << "' r='4' fill='" << colors[j] << "'/>\n";
        const int lx = 100 + static_cast<int>(j) * 190;
        out << "<rect x='" << lx << "' y='383' width='14' height='12' fill='" << colors[j] << "'/>\n";
        text(out, lx + 21, 395, names[j], 15);
    }
    out << "</svg>\n";
}

int main() {
    const Array input{8, 3, 6, 1, 7, 4};
    std::cout << "Исходный массив: ";
    for (int value : input) std::cout << value << ' ';
    std::cout << '\n';
    for (std::size_t i = 0; i < algorithms.size(); ++i) {
        auto values = input;
        algorithms[i](values, nullptr);
        std::cout << names[i] << ": ";
        for (int value : values) std::cout << value << ' ';
        std::cout << '\n';
    }
    std::cout << '\n';
    drawTrace(input);
    drawTiming(benchmark());
    std::cout << "Графика сохранена в steps.svg и timing.svg\n";
    return 0;
}
