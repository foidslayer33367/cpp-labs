#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class Customer {
private:
    std::string name;
    static int count;

public:
    explicit Customer(const std::string& customerName) : name(customerName) {
        if (name.find_first_not_of(" \t\r\n") == std::string::npos) {
            throw std::invalid_argument("Имя клиента не должно быть пустым");
        }
        ++count;
        std::cout << "Создан клиент: " << name << '\n';
    }

    Customer(const Customer&) = delete;
    Customer& operator=(const Customer&) = delete;

    ~Customer() {
        --count;
        std::cout << "Удалён клиент: " << name << '\n';
    }

    const std::string& getName() const {
        return name;
    }

    static int getCount() {
        return count;
    }
};

int Customer::count = 0;

class Purchase {
private:
    const Customer* customer; // Клиент принадлежит внешнему коду.
    std::string product;
    int quantity;
    double price;

public:
    Purchase(const Customer* buyer, const std::string& productName,
             int productQuantity, double productPrice)
        : customer(buyer), product(productName),
          quantity(productQuantity), price(productPrice) {
        if (customer == nullptr) {
            throw std::invalid_argument("Клиент не задан");
        }
        if (product.find_first_not_of(" \t\r\n") == std::string::npos) {
            throw std::invalid_argument("Название товара не должно быть пустым");
        }
        if (quantity <= 0 || !std::isfinite(price) || price <= 0 ||
            !std::isfinite(price * quantity)) {
            throw std::invalid_argument("Количество и цена должны быть положительными");
        }
        std::cout << "Создана покупка: " << product << '\n';
    }

    const Customer* getCustomer() const {
        return customer;
    }

    ~Purchase() {
        std::cout << "Удалена покупка: " << product << '\n';
    }

    void print() const {
        std::cout << "  " << customer->getName() << ": " << product
                  << ", " << quantity << " шт. по " << price
                  << " руб., сумма " << quantity * price << " руб.\n";
    }
};

class Store {
private:
    static Store* instance;
    std::vector<Customer*> customers; // Агрегация: магазин не удаляет клиентов.
    std::vector<std::unique_ptr<Purchase>> purchases; // Композиция.

    Store() {
        std::cout << "Магазин создан\n";
    }

    ~Store() {
        std::cout << "Магазин уничтожается\n";
        // Покупки удаляются автоматически вместе с их unique_ptr.
    }

public:
    Store(const Store&) = delete;
    Store& operator=(const Store&) = delete;

    static Store& getInstance() {
        if (instance == nullptr) {
            instance = new Store;
        }
        return *instance;
    }

    static void destroyInstance() {
        delete instance;
        instance = nullptr;
    }

    void addCustomer(Customer* customer) {
        if (customer == nullptr) {
            throw std::invalid_argument("Нельзя добавить nullptr вместо клиента");
        }
        if (std::find(customers.begin(), customers.end(), customer) != customers.end()) {
            throw std::invalid_argument("Клиент уже зарегистрирован");
        }
        customers.push_back(customer);
    }

    void addPurchase(Customer* customer, const std::string& product,
                     int quantity, double price) {
        if (customer == nullptr ||
            std::find(customers.begin(), customers.end(), customer) == customers.end()) {
            throw std::invalid_argument("Клиент не зарегистрирован в магазине");
        }
        purchases.push_back(std::make_unique<Purchase>(customer, product, quantity, price));
    }

    void removeCustomer(Customer* customer) {
        auto found = std::find(customers.begin(), customers.end(), customer);
        if (found == customers.end()) {
            throw std::invalid_argument("Клиент не найден");
        }
        // Перед отсоединением клиента удаляются связанные с ним покупки.
        purchases.erase(std::remove_if(purchases.begin(), purchases.end(),
                        [customer](const auto& purchase) {
                            return purchase->getCustomer() == customer;
                        }), purchases.end());
        customers.erase(found);
    }

    void print() const {
        std::cout << "Клиентов в магазине: " << customers.size()
                  << ", покупок: " << purchases.size() << '\n';
        for (const auto& purchase : purchases) {
            purchase->print();
        }
    }
};

Store* Store::instance = nullptr;

int main() {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Клиентов до создания: " << Customer::getCount() << "\n\n";

    try {
        Customer anna("Анна");
        Customer ivan("Иван");

        Store& store = Store::getInstance();
        Store& sameStore = Store::getInstance();
        std::cout << "Один экземпляр Store: " << std::boolalpha
                  << (&store == &sameStore) << '\n';
        std::cout << "Живых клиентов: " << Customer::getCount() << '\n';
        store.print();

        store.addCustomer(&anna);
        store.addCustomer(&ivan);
        store.addPurchase(&anna, "Клавиатура", 2, 1500.0);
        store.addPurchase(&ivan, "Мышь", 1, 900.0);
        store.print();

        std::cout << "\nОбработка ошибок\n";
        try {
            store.addCustomer(nullptr);
        } catch (const std::invalid_argument& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
        try {
            store.addCustomer(&anna);
        } catch (const std::invalid_argument& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
        try {
            store.addPurchase(&anna, "Монитор", 0, 10000.0);
        } catch (const std::invalid_argument& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
        try {
            Customer emptyName(" ");
        } catch (const std::invalid_argument& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }

        std::cout << "\nОтсоединение клиента\n";
        store.removeCustomer(&anna);
        std::cout << "Клиент продолжает существовать: " << anna.getName() << '\n';
        std::cout << "Живых клиентов: " << Customer::getCount() << '\n';
        store.print();

        try {
            store.addPurchase(&anna, "Кабель", 1, 300.0);
        } catch (const std::invalid_argument& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }

        std::cout << "\nУничтожение магазина\n";
        Store::destroyInstance();
        std::cout << "Клиенты после уничтожения магазина: "
                  << anna.getName() << ", " << ivan.getName() << '\n';
        std::cout << "Живых клиентов: " << Customer::getCount() << '\n';
    } catch (const std::exception& error) {
        Store::destroyInstance();
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }

    Store::destroyInstance(); // Повторное освобождение безопасно: instance == nullptr.
    std::cout << "\nКлиентов после выхода из блока: " << Customer::getCount() << '\n';
    return 0;
}
