#ifndef BIG_RATIONAL_H
#define BIG_RATIONAL_H

#include <iostream>
#include <string>

class BigRational {
private:
    // Строки хранят массивы десятичных цифр без ограничения размером int.
    std::string num;
    std::string den;
    void normalize();

public:
    BigRational();
    BigRational(long long value);
    BigRational(const std::string& numerator, const std::string& denominator);
    BigRational(const BigRational& other);
    ~BigRational();
    BigRational& operator=(const BigRational& other);

    const std::string& getNumerator() const;
    const std::string& getDenominator() const;
    void setNumerator(const std::string& value);
    void setDenominator(const std::string& value);

    friend BigRational operator+(const BigRational& a, const BigRational& b);
    friend BigRational operator-(const BigRational& a, const BigRational& b);
    friend BigRational operator*(const BigRational& a, const BigRational& b);
    friend BigRational operator/(const BigRational& a, const BigRational& b);
    BigRational operator-() const;
    BigRational& operator+=(const BigRational& other);
    BigRational& operator-=(const BigRational& other);
    BigRational& operator*=(const BigRational& other);
    BigRational& operator/=(const BigRational& other);

    bool operator==(const BigRational& other) const;
    bool operator!=(const BigRational& other) const;
    bool operator<(const BigRational& other) const;
    bool operator>(const BigRational& other) const;
    bool operator<=(const BigRational& other) const;
    bool operator>=(const BigRational& other) const;

    BigRational& operator++();
    BigRational operator++(int);
    BigRational& operator--();
    BigRational operator--(int);

    // 0 - числитель, 1 - знаменатель. Изменение выполняется сеттерами.
    const std::string& operator[](int index) const;
    std::string operator()(int precision) const;
    explicit operator double() const;
    explicit operator std::string() const;

    friend std::ostream& operator<<(std::ostream& out, const BigRational& value);
    friend std::istream& operator>>(std::istream& in, BigRational& value);
};

#endif
