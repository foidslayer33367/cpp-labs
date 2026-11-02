#include "BigRational.h"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace {
// Арифметика целых чисел выполняется по цифрам, как на бумаге.
std::string magnitude(const std::string& s) {
    return s[0] == '-' ? s.substr(1) : s;
}

std::string integer(const std::string& s) {
    if (s.empty()) throw std::invalid_argument("Пустое число");
    std::size_t start = (s[0] == '-' || s[0] == '+') ? 1 : 0;
    if (start == s.size()) throw std::invalid_argument("Нет цифр");
    for (std::size_t i = start; i < s.size(); ++i)
        if (s[i] < '0' || s[i] > '9')
            throw std::invalid_argument("Некорректное целое число");
    std::size_t first = s.find_first_not_of('0', start);
    if (first == std::string::npos) return "0";
    return (s[0] == '-' ? "-" : "") + s.substr(first);
}

int compare(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    return a == b ? 0 : (a < b ? -1 : 1);
}

std::string add(const std::string& a, const std::string& b) {
    std::string result;
    int carry = 0;
    std::size_t i = a.size(), j = b.size();
    while (i || j || carry) {
        int sum = carry;
        if (i) sum += a[--i] - '0';
        if (j) sum += b[--j] - '0';
        result.push_back(char('0' + sum % 10));
        carry = sum / 10;
    }
    std::reverse(result.begin(), result.end());
    return result;
}

// Вычитание модулей, a >= b.
std::string subtract(const std::string& a, const std::string& b) {
    std::string result;
    int borrow = 0;
    std::size_t i = a.size(), j = b.size();
    while (i) {
        int digit = a[--i] - '0' - borrow;
        if (j) digit -= b[--j] - '0';
        borrow = digit < 0;
        if (borrow) digit += 10;
        result.push_back(char('0' + digit));
    }
    std::reverse(result.begin(), result.end());
    return integer(result);
}

std::string multiply(const std::string& a, const std::string& b) {
    if (a == "0" || b == "0") return "0";
    std::string result(a.size() + b.size(), '0');
    for (std::size_t i = a.size(); i-- > 0;) {
        int carry = 0;
        for (std::size_t j = b.size(); j-- > 0;) {
            int value = (a[i] - '0') * (b[j] - '0')
                      + (result[i + j + 1] - '0') + carry;
            result[i + j + 1] = char('0' + value % 10);
            carry = value / 10;
        }
        result[i] = char('0' + carry);
    }
    return integer(result);
}

// Деление столбиком возвращает частное и остаток.
std::pair<std::string, std::string> divide(const std::string& a,
                                           const std::string& b) {
    if (b == "0") throw std::domain_error("Деление на ноль");
    std::string quotient, remainder = "0";
    for (char digit : a) {
        remainder = integer(remainder + digit);
        int q = 0;
        while (compare(remainder, b) >= 0) {
            remainder = subtract(remainder, b);
            ++q;
        }
        quotient.push_back(char('0' + q));
    }
    return {integer(quotient), remainder};
}

std::string gcd(std::string a, std::string b) {
    while (b != "0") {
        std::string remainder = divide(a, b).second;
        a = b;
        b = remainder;
    }
    return a;
}

std::string signedAdd(const std::string& a, const std::string& b) {
    bool negativeA = a[0] == '-', negativeB = b[0] == '-';
    std::string x = magnitude(a), y = magnitude(b);
    if (negativeA == negativeB)
        return integer((negativeA ? "-" : "") + add(x, y));
    if (compare(x, y) >= 0)
        return integer((negativeA ? "-" : "") + subtract(x, y));
    return integer((negativeB ? "-" : "") + subtract(y, x));
}

std::string signedMultiply(const std::string& a, const std::string& b) {
    bool negative = (a[0] == '-') != (b[0] == '-');
    return integer((negative ? "-" : "") + multiply(magnitude(a), magnitude(b)));
}
}

BigRational::BigRational() : num("0"), den("1") {}
BigRational::BigRational(long long value) : num(std::to_string(value)), den("1") {}
BigRational::BigRational(const std::string& numerator, const std::string& denominator)
    : num(integer(numerator)), den(integer(denominator)) { normalize(); }
BigRational::BigRational(const BigRational& other) = default;
BigRational::~BigRational() = default;
BigRational& BigRational::operator=(const BigRational& other) = default;

void BigRational::normalize() {
    if (den == "0") throw std::domain_error("Знаменатель равен нулю");
    bool negative = (num[0] == '-') != (den[0] == '-');
    num = magnitude(num);
    den = magnitude(den);
    std::string divisor = gcd(num, den);
    num = divide(num, divisor).first;
    den = divide(den, divisor).first;
    if (negative && num != "0") num = "-" + num;
}

const std::string& BigRational::getNumerator() const { return num; }
const std::string& BigRational::getDenominator() const { return den; }
void BigRational::setNumerator(const std::string& value) {
    *this = BigRational(value, den);
}
void BigRational::setDenominator(const std::string& value) {
    *this = BigRational(num, value);
}

BigRational operator+(const BigRational& a, const BigRational& b) {
    return BigRational(signedAdd(signedMultiply(a.num, b.den),
                                 signedMultiply(b.num, a.den)), multiply(a.den, b.den));
}
BigRational operator-(const BigRational& a, const BigRational& b) { return a + (-b); }
BigRational operator*(const BigRational& a, const BigRational& b) {
    return BigRational(signedMultiply(a.num, b.num), multiply(a.den, b.den));
}
BigRational operator/(const BigRational& a, const BigRational& b) {
    if (b.num == "0") throw std::domain_error("Деление на нулевую дробь");
    return BigRational(signedMultiply(a.num, b.den), signedMultiply(a.den, b.num));
}
BigRational BigRational::operator-() const { return BigRational(signedMultiply(num, "-1"), den); }
BigRational& BigRational::operator+=(const BigRational& b) { return *this = *this + b; }
BigRational& BigRational::operator-=(const BigRational& b) { return *this = *this - b; }
BigRational& BigRational::operator*=(const BigRational& b) { return *this = *this * b; }
BigRational& BigRational::operator/=(const BigRational& b) { return *this = *this / b; }

bool BigRational::operator==(const BigRational& b) const { return num == b.num && den == b.den; }
bool BigRational::operator!=(const BigRational& b) const { return !(*this == b); }
bool BigRational::operator<(const BigRational& b) const {
    return (*this - b).num[0] == '-';
}
bool BigRational::operator>(const BigRational& b) const { return b < *this; }
bool BigRational::operator<=(const BigRational& b) const { return !(*this > b); }
bool BigRational::operator>=(const BigRational& b) const { return !(*this < b); }

BigRational& BigRational::operator++() { return *this += 1; }
BigRational BigRational::operator++(int) { BigRational old(*this); ++*this; return old; }
BigRational& BigRational::operator--() { return *this -= 1; }
BigRational BigRational::operator--(int) { BigRational old(*this); --*this; return old; }

const std::string& BigRational::operator[](int index) const {
    if (index == 0) return num;
    if (index == 1) return den;
    throw std::out_of_range("Индекс должен быть 0 или 1");
}

// Десятичная запись с заданным числом знаков после точки, без округления.
std::string BigRational::operator()(int precision) const {
    if (precision < 0) throw std::invalid_argument("Точность не может быть отрицательной");
    auto parts = divide(magnitude(num), den);
    std::string result = (num[0] == '-' ? "-" : "") + parts.first;
    if (precision > 0) result += '.';
    for (int i = 0; i < precision; ++i) {
        parts = divide(parts.second + "0", den);
        result += parts.first;
    }
    return result;
}

BigRational::operator double() const {
    // double приблизителен и ограничен по диапазону; stod может бросить out_of_range.
    return std::stod(num) / std::stod(den);
}
BigRational::operator std::string() const { return num + "/" + den; }
std::ostream& operator<<(std::ostream& out, const BigRational& value) {
    return out << value.num << '/' << value.den;
}
std::istream& operator>>(std::istream& in, BigRational& value) {
    std::string text;
    if (!(in >> text)) return in;
    std::size_t slash = text.find('/');
    // Объект меняется только после успешной проверки всей дроби.
    value = slash == std::string::npos ? BigRational(text, "1")
          : BigRational(text.substr(0, slash), text.substr(slash + 1));
    return in;
}
