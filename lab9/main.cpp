#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using Values = std::vector<int>;

class BinaryTree {
protected:
    struct Node {
        int key;
        std::unique_ptr<Node> left, right;
        explicit Node(int value) : key(value) {}
    };
    std::unique_ptr<Node> root;

private:
    enum class Order { pre, in, post };
    static void depthOrder(const Node* node, Order order, Values& result) {
        if (!node) return;
        if (order == Order::pre) result.push_back(node->key);
        depthOrder(node->left.get(), order, result);
        if (order == Order::in) result.push_back(node->key);
        depthOrder(node->right.get(), order, result);
        if (order == Order::post) result.push_back(node->key);
    }
    Values traverse(Order order) const {
        Values result;
        depthOrder(root.get(), order, result);
        return result;
    }

    static std::size_t heightOf(const Node* node) {
        if (!node) return 0;
        return 1 + std::max(heightOf(node->left.get()), heightOf(node->right.get()));
    }
    static bool perfectAt(const Node* node, std::size_t levels) {
        if (!node) return levels == 0;
        // У всех внутренних узлов должны быть обе ветви полной глубины.
        return levels > 0 && perfectAt(node->left.get(), levels - 1) &&
               perfectAt(node->right.get(), levels - 1);
    }

    static void draw(const Node* node, std::size_t depth, const char* label) {
        if (!node) return;
        draw(node->right.get(), depth + 1, "R: ");
        std::cout << std::string(4 * depth, ' ') << label << node->key << '\n';
        draw(node->left.get(), depth + 1, "L: ");
    }

public:
    BinaryTree() = default;

    static BinaryTree fromLevel(const std::vector<std::optional<int>>& slots) {
        BinaryTree tree;
        std::vector<Node*> nodes(slots.size(), nullptr);
        for (std::size_t i = 0; i < slots.size(); ++i) {
            if (!slots[i]) continue;
            auto node = std::make_unique<Node>(*slots[i]);
            nodes[i] = node.get();
            if (i == 0) {
                tree.root = std::move(node);
            } else {
                Node* parent = nodes[(i - 1) / 2];
                if (!parent) throw std::invalid_argument("У узла нет родителя");
                if (i % 2) parent->left = std::move(node);
                else parent->right = std::move(node);
            }
        }
        return tree;
    }

    Values preOrder() const { return traverse(Order::pre); }
    Values inOrder() const { return traverse(Order::in); }
    Values postOrder() const { return traverse(Order::post); }

    Values levelOrder() const {
        Values result;
        std::queue<const Node*> pending;
        if (root) pending.push(root.get());
        while (!pending.empty()) {
            const Node* node = pending.front();
            pending.pop();
            result.push_back(node->key);
            if (node->left) pending.push(node->left.get());
            if (node->right) pending.push(node->right.get());
        }
        return result;
    }
    std::size_t height() const { return heightOf(root.get()); }
    bool isPerfect() const { return perfectAt(root.get(), height()); }
    void printSideways() const {
        if (!root) std::cout << "(дерево пусто)\n";
        else draw(root.get(), 0, "Корень: ");
    }
};

class BST : private BinaryTree {
    static std::unique_ptr<Node> build(const Values& pre, std::size_t& i,
                                       std::optional<int> low, std::optional<int> high) {
        if (i == pre.size()) return nullptr;
        const int key = pre[i];
        if ((low && key <= *low) || (high && key >= *high)) return nullptr;
        ++i;
        auto node = std::make_unique<Node>(key);
        // Неподходящий ключ остаётся для другой ветви, индекс не меняется.
        node->left = build(pre, i, low, key);
        node->right = build(pre, i, key, high);
        return node;
    }

    static int extreme(const Node* node, bool minimum) {
        while (true) {
            const Node* next = minimum ? node->left.get() : node->right.get();
            if (!next) return node->key;
            node = next;
        }
    }

public:
    using BinaryTree::height;
    using BinaryTree::inOrder;
    using BinaryTree::preOrder;
    using BinaryTree::printSideways;

    static BST fromPreOrder(const Values& pre) {
        BST tree;
        std::size_t consumed = 0;
        tree.root = build(pre, consumed, std::nullopt, std::nullopt);
        if (consumed != pre.size()) throw std::invalid_argument("Некорректный preorder BST");
        return tree;
    }

    bool insert(int key) {
        auto* link = &root;
        while (*link) {
            if (key == (*link)->key) return false;
            link = key < (*link)->key ? &(*link)->left : &(*link)->right;
        }
        *link = std::make_unique<Node>(key);
        return true;
    }
    bool search(int key) const {
        const Node* node = root.get();
        while (node) {
            if (key == node->key) return true;
            node = key < node->key ? node->left.get() : node->right.get();
        }
        return false;
    }
    std::optional<int> minimum() const {
        if (!root) return std::nullopt;
        return extreme(root.get(), true);
    }
    std::optional<int> maximum() const {
        if (!root) return std::nullopt;
        return extreme(root.get(), false);
    }

    bool remove(int key) {
        auto* link = &root;
        while (*link && (*link)->key != key)
            link = key < (*link)->key ? &(*link)->left : &(*link)->right;
        if (!*link) return false;
        if ((*link)->left && (*link)->right) {
            // Подставляем минимум правой ветви и удаляем исходный узел преемника.
            auto* successor = &(*link)->right;
            while ((*successor)->left) successor = &(*successor)->left;
            (*link)->key = (*successor)->key;
            link = successor;
        }
        auto old = std::move(*link);
        *link = old->left ? std::move(old->left) : std::move(old->right);
        return true;
    }
};

void printValues(const char* label, const Values& values) {
    std::cout << label;
    for (int key : values) std::cout << ' ' << key;
    std::cout << '\n';
}

void demonstrateTraversals() {
    const auto tree = BinaryTree::fromLevel({1, 2, 3, 4, 5, 6, 7});
    const auto incomplete = BinaryTree::fromLevel({1, 2, 3, 4});
    printValues("Прямой:", tree.preOrder());
    printValues("Симметричный:", tree.inOrder());
    printValues("Обратный:", tree.postOrder());
    printValues("По уровням:", tree.levelOrder());
    std::cout << "Совершенное: " << tree.isPerfect()
              << ", с разной глубиной листьев: " << incomplete.isPerfect() << '\n';
}

void demonstrateReconstruction() {
    const Values prefix{8, 3, 1, 6, 4, 7, 10, 14, 13};
    auto tree = BST::fromPreOrder(prefix);
    printValues("Исходный preorder:", prefix);
    printValues("Восстановленный preorder:", tree.preOrder());
    printValues("Симметричный обход:", tree.inOrder());
    std::cout << "Поиск 6: " << tree.search(6) << ", поиск 5: " << tree.search(5) << '\n';
    for (int key : {1, 14, 8}) {
        if (tree.remove(key)) {
            std::cout << "Удалён " << key << ". ";
            printValues("Симметричный обход:", tree.inOrder());
        }
    }
}

int main() {
    std::cout << std::boolalpha;
    demonstrateTraversals();
    std::cout << '\n';
    demonstrateReconstruction();

    Values candidates(99);
    std::iota(candidates.begin(), candidates.end(), 1);
    std::mt19937 generator(42);
    std::shuffle(candidates.begin(), candidates.end(), generator);
    candidates.resize(20);
    BST tree;
    for (int key : candidates) tree.insert(key);

    std::cout << '\n';
    printValues("20 случайных ключей:", candidates);
    printValues("Отсортированный обход:", tree.inOrder());
    std::cout << "Минимум: " << *tree.minimum() << ", максимум: " << *tree.maximum()
              << ", высота в уровнях: " << tree.height() << '\n';
    std::cout << "До удаления (правая ветвь сверху):\n";
    tree.printSideways();

    std::cout << "\nВведите целый ключ для удаления: " << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "\nКлюч не введён.\n";
        return 0;
    }
    std::istringstream input(line);
    int key;
    char extra;
    if (!(input >> key) || (input >> extra)) {
        std::cout << "Некорректный ввод.\n";
        return 0;
    }
    std::cout << "\nКлюч: " << key << ". " << (tree.remove(key) ? "Удалён." : "Не найден.") << '\n';
    printValues("Обход после запроса:", tree.inOrder());
    std::cout << "После запроса на удаление:\n";
    tree.printSideways();
    return 0;
}
