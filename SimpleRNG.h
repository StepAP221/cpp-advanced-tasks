#pragma once
#include <iterator>
#include <cmath> // Для std::fmod (остаток от деления дробных чисел)
#include <vector>

/**
 * Задача 1: Генератор псевдослучайных чисел (SimpleRNG).
 * Реализует интерфейс InputIterator для совместимости с алгоритмами STL.
 */
class SimpleRNG
{
private:
    // Параметры генератора согласно формуле: X[N+1] = (A * X[N] + C) % M
    double _factor_a;   // Множитель (a)
    double _term_c;     // Приращение (c)
    double _modulus_m;  // Модуль (m)

    double _initial_x;  // Начальное значение (X[0]), нужно для reset() и end()
    double _current_x;  // Текущее значение генератора

public:
    // Конструктор: инициализирует коэффициенты.
    // m > 1, 0 < a < 1, 0 < c < m
    SimpleRNG(double a, double c, double m) 
        : _factor_a(a), _term_c(c), _modulus_m(m), _initial_x(0.0), _current_x(0.0)
    {
    }

    // Устанавливает новое начальное значение и сбрасывает текущее состояние
    void reset(double start_value)
    {
        _initial_x = start_value;
        _current_x = start_value;
    }

    // Сбрасывает состояние к сохраненному начальному значению
    void reset()
    {
        _current_x = _initial_x;
    }

    /**
     * Внутренний класс итератора.
     * Позволяет использовать generator в циклах for(auto x : gen) и алгоритмах std::copy.
     */
    class Iterator
    {
    public:
        // Обязательные объявления типов для STL (traits)
        using iterator_category = std::input_iterator_tag;
        using value_type = double;
        using difference_type = std::ptrdiff_t;
        using pointer = const double*;
        using reference = const double&;

    private:
        SimpleRNG* _rng_instance; // Указатель на сам генератор, чтобы менять его состояние
        double _value;            // Значение, на которое указывает итератор в данный момент
        double _epsilon;          // Погрешность сравнения (для end итератора)
        bool _is_terminator;      // Флаг: true, если это итератор "конца" (end)

    public:
        // Конструктор обычного итератора (указывает на текущее число)
        Iterator(SimpleRNG* rng, double val) 
            : _rng_instance(rng), _value(val), _epsilon(0.0), _is_terminator(false) 
        {
        }

        // Конструктор итератора конца (sentinel).
        // Он не привязан к генератору, а хранит условие остановки (значение и точность).
        Iterator(double target_val, double eps) 
            : _rng_instance(nullptr), _value(target_val), _epsilon(eps), _is_terminator(true) 
        {
        }

        // Оператор разыменования (*it)
        reference operator*() const
        {
            return _value;
        }

        // Оператор префиксного инкремента (++it).
        // Вычисляет следующее случайное число.
        Iterator& operator++()
        {
            if (_rng_instance)
            {
                // Шаг 1: Вычисляем линейную часть a * X + c
                double linear = (_rng_instance->_factor_a * _value) + _rng_instance->_term_c;
                
                // Шаг 2: Берем остаток от деления на m (для double используется fmod)
                double next_val = std::fmod(linear, _rng_instance->_modulus_m);
                
                // Шаг 3: Обновляем итератор и состояние генератора
                _value = next_val;
                _rng_instance->_current_x = next_val;
            }
            return *this;
        }

        // Постфиксный инкремент (it++)
        Iterator operator++(int)
        {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        // Оператор сравнения.
        // Это самая хитрая часть: мы считаем, что дошли до конца, 
        // если текущее значение совпадает с начальным (цикл замкнулся) с точностью epsilon.
        bool operator==(const Iterator& other) const
        {
            // Сценарий 1: Мы сравниваем текущий итератор с "концевым"
            if (other._is_terminator)
            {
                return std::abs(_value - other._value) < other._epsilon;
            }
            // Сценарий 2: "Концевой" сравнивается с текущим
            if (_is_terminator)
            {
                return std::abs(other._value - _value) < _epsilon;
            }
            // Сценарий 3: Два обычных итератора (сравниваем значения точно)
            return _value == other._value;
        }

        bool operator!=(const Iterator& other) const
        {
            return !(*this == other);
        }
    };

    // Возвращает итератор на текущее состояние
    Iterator begin()
    {
        return Iterator(this, _current_x);
    }

    // Возвращает итератор "конца".
    // Он настроен на ожидание числа _initial_x с точностью eps.
    Iterator end(double eps = 0.05)
    {
        return Iterator(_initial_x, eps);
    }
};
