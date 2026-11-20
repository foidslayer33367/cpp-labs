#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

std::uint64_t factorial(int n) {
    if (n < 0 || n > 20) throw std::out_of_range("Факториал: допустимо 0–20");
    if (n <= 1) return 1; // Базовый случай: 0! и 1! равны 1.
    return static_cast<std::uint64_t>(n) * factorial(n - 1);
}

std::uint64_t factorialIterative(int n) {
    if (n < 0 || n > 20) throw std::out_of_range("Факториал: допустимо 0–20");
    std::uint64_t value = 1;
    for (int i = 2; i <= n; ++i) value *= i;
    return value;
}

std::uint64_t fibonacci(int n) {
    if (n < 0 || n > 40) throw std::out_of_range("Рекурсивный Фибоначчи: допустимо 0–40");
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

std::uint64_t fibonacciIterative(int n) {
    if (n < 0 || n > 93) throw std::out_of_range("Фибоначчи: допустимо 0–93");
    if (n == 0) return 0;
    std::uint64_t previous = 0, current = 1;
    for (int i = 2; i <= n; ++i) {
        const auto next = previous + current;
        previous = current;
        current = next;
    }
    return current;
}

constexpr int mazeSize = 5;
using Maze = std::array<std::string, mazeSize>;
using Visited = std::array<std::array<bool, mazeSize>, mazeSize>;

bool walkMaze(Maze& maze, Visited& seen, int row, int col) {
    if (row < 0 || row >= mazeSize || col < 0 || col >= mazeSize) return false;
    char& cell = maze[row][col];
    if (cell == '#' || seen[row][col]) return false;
    if (cell == 'F') return true;
    seen[row][col] = true;
    // Звёздочки появляются только на успешной ветви при возврате.
    if (walkMaze(maze, seen, row + 1, col) || walkMaze(maze, seen, row, col + 1) ||
        walkMaze(maze, seen, row - 1, col) || walkMaze(maze, seen, row, col - 1)) {
        if (cell != 'S') cell = '*';
        return true;
    }
    return false;
}

bool findPath(Maze& maze) {
    int startRow = -1, startCol = -1, starts = 0, finishes = 0;
    for (int row = 0; row < mazeSize; ++row) {
        if (maze[row].size() != mazeSize) throw std::invalid_argument("Нужен лабиринт 5×5");
        for (int col = 0; col < mazeSize; ++col) {
            if (maze[row][col] == 'S') {
                ++starts;
                startRow = row;
                startCol = col;
            } else if (maze[row][col] == 'F') {
                ++finishes;
            }
        }
    }
    if (starts != 1 || finishes != 1) throw std::invalid_argument("Нужны один старт и один финиш");
    Visited seen{};
    return walkMaze(maze, seen, startRow, startCol);
}

constexpr int boardSize = 4;
using Placement = std::array<int, boardSize>;

bool safe(const Placement& columns, int row, int col) {
    for (int previous = 0; previous < row; ++previous) {
        const int distance = row - previous;
        const int offset = col - columns[previous];
        if (offset == 0 || offset == distance || offset == -distance) return false;
    }
    return true;
}

void solveQueens(int row, Placement& columns, std::vector<Placement>& solutions) {
    if (row == boardSize) {
        solutions.push_back(columns);
        return;
    }
    for (int col = 0; col < boardSize; ++col) {
        if (!safe(columns, row, col)) continue;
        // Ставим ферзя в этой строке и перебираем следующую.
        columns[row] = col;
        solveQueens(row + 1, columns, solutions);
    }
}

void bubbleSort(std::vector<int>& a) {
    for (std::size_t end = a.size(); end > 1; --end) {
        bool swapped = false;
        for (std::size_t i = 1; i < end; ++i) {
            if (a[i - 1] > a[i]) {
                std::swap(a[i - 1], a[i]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

void insertionSort(std::vector<int>& a) {
    for (std::size_t i = 1; i < a.size(); ++i) {
        const int value = a[i];
        std::size_t j = i;
        while (j > 0 && a[j - 1] > value) {
            a[j] = a[j - 1];
            --j;
        }
        a[j] = value;
    }
}

void selectionSort(std::vector<int>& a) {
    for (std::size_t i = 0; i < a.size(); ++i) {
        std::size_t minimum = i;
        for (std::size_t j = i + 1; j < a.size(); ++j) {
            if (a[j] < a[minimum]) minimum = j;
        }
        if (minimum != i) std::swap(a[i], a[minimum]);
    }
}

void selectionSortList(std::forward_list<int>& list) {
    std::forward_list<int> sorted;
    auto tail = sorted.before_begin();
    while (!list.empty()) {
        auto beforeMin = list.before_begin();
        int minimum = list.front();
        auto previous = list.before_begin();
        for (auto it = list.begin(); it != list.end(); ++it, ++previous) {
            if (*it < minimum) {
                minimum = *it;
                beforeMin = previous;
            }
        }
        // Переставляем узел целиком: значения не копируются и не меняются местами.
        sorted.splice_after(tail, list, beforeMin);
        ++tail;
    }
    list.swap(sorted);
}

template <class Range>
void printRange(const Range& range) {
    bool first = true;
    for (const auto& value : range) {
        if (!first) std::cout << ' ';
        first = false;
        std::cout << value;
    }
    std::cout << '\n';
}

template <class Container, class Sort>
double medianTime(const Container& original, const std::vector<int>& expected, Sort sort) {
    std::array<double, 5> samples{};
    for (double& sample : samples) {
        Container copy = original;
        const auto start = std::chrono::steady_clock::now();
        sort(copy);
        const auto end = std::chrono::steady_clock::now();
        if (!std::equal(copy.begin(), copy.end(), expected.begin(), expected.end()))
            throw std::logic_error("Сортировка дала неверный результат");
        sample = std::chrono::duration<double, std::milli>(end - start).count();
    }
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

void benchmark(const char* title, const std::vector<int>& values) {
    auto expected = values;
    std::sort(expected.begin(), expected.end());
    const std::forward_list<int> list(values.begin(), values.end());
    std::cout << std::setw(17) << title
              << " | " << std::setw(9) << medianTime(values, expected, bubbleSort)
              << " | " << std::setw(9) << medianTime(values, expected, insertionSort)
              << " | " << std::setw(9) << medianTime(values, expected, selectionSort)
              << " | " << std::setw(9) << medianTime(list, expected, selectionSortList) << '\n';
}

void demonstrateRecursion() {
    std::cout << "Рекурсия и цикл\n";
    for (int n : {0, 1, 5, 10}) {
        std::cout << n << "! = " << factorial(n) << ", циклом " << factorialIterative(n)
                  << "; F(" << n << ") = " << fibonacci(n)
                  << ", циклом " << fibonacciIterative(n) << '\n';
    }

    Maze maze{"S.#..", "#.#.#", "...#.", "##...", "..#.F"};
    const bool found = findPath(maze);
    std::cout << "\nЛабиринт 5×5: " << (found ? "путь найден" : "пути нет") << '\n';
    for (const auto& row : maze) std::cout << row << '\n';
    Maze blocked{"S####", "#####", "#####", "#####", "####F"};
    std::cout << "Заблокированный лабиринт: "
              << (findPath(blocked) ? "путь найден" : "пути нет") << '\n';

    Placement columns{};
    std::vector<Placement> solutions;
    solveQueens(0, columns, solutions);
    std::cout << "Ферзи 4×4: решений " << solutions.size() << '\n';
    for (std::size_t i = 0; i < solutions.size(); ++i) {
        std::cout << "Расстановка " << i + 1 << '\n';
        for (int column : solutions[i]) {
            for (int c = 0; c < boardSize; ++c) std::cout << (c == column ? 'Q' : '.');
            std::cout << '\n';
        }
    }
}

int main() {
    demonstrateRecursion();
    const std::vector<int> sample{7, -2, 4, 4, 0, 1};
    auto bubble = sample, insertion = sample, selection = sample;
    std::forward_list<int> linked(sample.begin(), sample.end());
    bubbleSort(bubble);
    insertionSort(insertion);
    selectionSort(selection);
    selectionSortList(linked);
    std::cout << "\nСортировка примера\n";
    std::cout << "Вход: "; printRange(sample);
    std::cout << "Пузырьковая: "; printRange(bubble);
    std::cout << "Вставками: "; printRange(insertion);
    std::cout << "Выбором: "; printRange(selection);
    std::cout << "Выбором в списке: "; printRange(linked);

    constexpr int n = 3000;
    std::vector<int> random(n);
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> distribution(-n, n);
    for (int& value : random) value = distribution(generator);
    auto sorted = random;
    std::sort(sorted.begin(), sorted.end());
    auto reversed = sorted;
    std::reverse(reversed.begin(), reversed.end());

    std::cout << "\nМедиана 5 запусков, n = " << n << ", мс\n"
              << "             Вход |    Пузырь |   Вставки |     Выбор |    Список\n"
              << std::fixed << std::setprecision(3);
    benchmark("random", random);
    benchmark("sorted", sorted);
    benchmark("reverse", reversed);
    return 0;
}
