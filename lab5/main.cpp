#include "BigRational.h"
#include <stdexcept>

int main() {
    std::cout << std::boolalpha;
    BigRational a("3", "4"), b("2", "3"), zero;
    BigRational copy(a), assigned;
    assigned = b;
    std::cout << "=== Создание и арифметика ===\n";
    std::cout << "a = " << a << ", b = " << b << ", zero = " << zero << '\n';
    std::cout << "Копия a: " << copy << ", присваивание b: " << assigned << '\n';
    std::cout << "a + b = " << a + b << '\n';
    std::cout << "a - b = " << a - b << '\n';
    std::cout << "a * b = " << a * b << '\n';
    std::cout << "a / b = " << a / b << '\n';
    std::cout << "-a = " << -a << '\n';
    std::cout << "6/-8 = " << BigRational("6", "-8") << '\n';

    std::cout << "\n=== Составное присваивание и сравнение ===\n";
    BigRational c(a);
    c += b; std::cout << "c += b: " << c << '\n';
    c -= b; std::cout << "c -= b: " << c << '\n';
    c *= b; std::cout << "c *= b: " << c << '\n';
    c /= b; std::cout << "c /= b: " << c << '\n';
    std::cout << "a == b: " << (a == b) << ", a != b: " << (a != b) << '\n';
    std::cout << "a < b: " << (a < b) << ", a > b: " << (a > b) << '\n';
    std::cout << "a <= b: " << (a <= b) << ", a >= b: " << (a >= b) << '\n';
    std::cout << "3/4 == 6/8: " << (a == BigRational("6", "8")) << '\n';

    std::cout << "\n=== Инкремент и декремент ===\n";
    std::cout << "++c: " << ++c << '\n';
    std::cout << "c++ вернул " << c++ << ", теперь c = " << c << '\n';
    std::cout << "--c: " << --c << '\n';
    std::cout << "c-- вернул " << c-- << ", теперь c = " << c << '\n';

    std::cout << "\n=== Доступ и преобразования ===\n";
    std::cout << "a[0] = " << a[0] << ", a[1] = " << a[1] << '\n';
    c.setNumerator("9");
    c.setDenominator("12");
    std::cout << "После сеттеров: " << c.getNumerator() << '/' << c.getDenominator() << '\n';
    std::cout << "a(6) = " << a(6) << '\n';
    std::cout << "1/3 с 20 знаками: " << BigRational("1", "3")(20) << '\n';
    std::cout << "double(a) = " << static_cast<double>(a) << '\n';
    std::cout << "string(a) = " << static_cast<std::string>(a) << '\n';

    std::cout << "\n=== Дружественные операции с целыми ===\n";
    std::cout << "2 + a = " << 2 + a << ", a + 2 = " << a + 2 << '\n';
    std::cout << "2 - a = " << 2 - a << ", a - 2 = " << a - 2 << '\n';
    std::cout << "2 * a = " << 2 * a << ", a * 2 = " << a * 2 << '\n';
    std::cout << "2 / a = " << 2 / a << ", a / 2 = " << a / 2 << '\n';

    std::cout << "\n=== Числа больше long long ===\n";
    BigRational large("123456789012345678901234567890", "97");
    std::cout << "Большая дробь: " << large << '\n';
    std::cout << "Прибавили 1: " << large + 1 << '\n';
    std::cout << "Умножили на 97: " << large * 97 << '\n';

    std::cout << "\n=== Обработка ошибок ===\n";
    try { BigRational bad("1", "0"); }
    catch (const std::exception& e) { std::cout << e.what() << '\n'; }
    try { std::cout << a / zero; }
    catch (const std::exception& e) { std::cout << e.what() << '\n'; }
    try { std::cout << a[2]; }
    catch (const std::exception& e) { std::cout << e.what() << '\n'; }
    try { std::cout << a(-1); }
    catch (const std::exception& e) { std::cout << e.what() << '\n'; }
    try { BigRational bad("12a", "3"); }
    catch (const std::exception& e) { std::cout << e.what() << '\n'; }

    std::cout << "\nВведите дробь (например -10/15) или целое число: ";
    try {
        BigRational input;
        if (std::cin >> input) std::cout << "После сокращения: " << input << '\n';
        else std::cout << "Ввод не получен\n";
    } catch (const std::exception& e) {
        std::cout << "Ошибка ввода: " << e.what() << '\n';
    }
}
