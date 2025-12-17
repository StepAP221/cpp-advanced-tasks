#ifndef SIMPLERNG_H
#define SIMPLERNG_H

#include <iterator>
#include <cmath>
#include <iostream>

/*
 * Задача 1: Генератор псевдослучайных чисел.
 * Реализован как InputIterator.
 * Формула: X[N+1] = ( a * X[N] + c ) % m
 *
 * Пояснение реализации:
 * Так как числа вещественные (double), вместо целочисленного деления по модулю (%)
 * используется функция std::fmod.
 */
class SimpleRNG {
private:
    double a, c, m; // Коэффициенты генерации
    double x0;      // Начальное значение (для сброса)
    double current; // Текущее значение

public:
    // Конструктор: инициализирует параметры генератора.
    // m > 1, 0 < a < 1, 0 < c < m
    SimpleRNG(double a_val, double c_val, double m_val)
        : a(a_val), c(c_val), m(m_val), x0(0), current(0) {}

    // Метод установки начального состояния
    void reset(double start_val) {
        x0 = start_val;
        current = start_val;
    }

    // Перегрузка reset без аргументов - сбрасывает к изначально заданному x0
    void reset() {
        current = x0;
    }

    /*
     * Вложенный класс Iterator.
     * Реализует требования InputIterator для совместимости с STL (std::copy, range-based for).
     */
    class Iterator {
    public:
        // Объявление типов итератора для STL
        using iterator_category = std::input_iterator_tag;
        using value_type = double;
        using difference_type = std::ptrdiff_t;
        using pointer = const double*;
        using reference = const double&;

    private:
        SimpleRNG* rng_ptr; // Указатель на генератор, чтобы менять его состояние
        double value;       // Текущее значение, которое хранит итератор
        double epsilon;     // Точность сравнения (для end итератора)
        bool is_end_marker; // Флаг: является ли этот итератор "концом" (sentinel)

    public:
        // Конструктор для обычного итератора (текущее состояние)
        Iterator(SimpleRNG* ptr, double val)
            : rng_ptr(ptr), value(val), epsilon(0.0), is_end_marker(false) {}

        // Конструктор для end итератора (хранит целевое значение для остановки)
        Iterator(double target_val, double eps)
            : rng_ptr(nullptr), value(target_val), epsilon(eps), is_end_marker(true) {}

        // Оператор разыменования: возвращает текущее число
        reference operator*() const {
            return value;
        }

        // Оператор инкремента (префиксный): генерирует следующее число
        Iterator& operator++() {
            if (rng_ptr) {
                // Формула: X[N+1] = ( a * X[N] + c ) % m
                double next_val = std::fmod((rng_ptr->a * value + rng_ptr->c), rng_ptr->m);
                value = next_val;
                rng_ptr->current = next_val; // Обновляем состояние самого генератора
            }
            return *this;
        }

        // Оператор инкремента (постфиксный)
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        // Оператор сравнения !=
        // Логика: мы не равны end-итератору, пока разница между текущим значением
        // и значением end-итератора больше epsilon.
        bool operator!=(const Iterator& other) const {
            // Если сравниваем текущий итератор с "концевым"
            if (other.is_end_marker) {
                return std::abs(value - other.value) >= other.epsilon;
            }
            // Зеркальная проверка
            if (is_end_marker) {
                return std::abs(other.value - value) >= epsilon;
            }
            return value != other.value;
        }

        bool operator==(const Iterator& other) const {
            return !(*this != other);
        }
    };

    // begin() возвращает итератор на текущее состояние
    Iterator begin() {
        return Iterator(this, current);
    }

    // end(eps) возвращает "стражника".
    // Он хранит значение x0 (или ожидаемое значение цикла) и точность eps.
    // Когда генератор вернется к числу, близкому к x0, цикл остановится.
    Iterator end(double eps = 0.05) {
        return Iterator(x0, eps);
    }
};

#endif
