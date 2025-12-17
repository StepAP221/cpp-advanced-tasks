#pragma once
#include <iterator>
#include <cmath> // Для std::fmod
#include <vector>

// Задание 1: Генератор псевдослучайных чисел
// Формула: X[N+1] = (a * X[N] + c) % m
class SimpleRNG
{
private:
    double a, c, m;
    
    double start_val; // Начальное значение (X0)
    double current;   // Текущее значение

public:
    // Конструктор инициализации
    SimpleRNG(double _a, double _c, double _m) 
        : a(_a), c(_c), m(_m), start_val(0), current(0)
    {
    }

    // Сброс с установкой нового начального значения
    void reset(double new_start)
    {
        start_val = new_start;
        current = new_start;
    }

    // Сброс к сохраненному начальному значению
    void reset()
    {
        current = start_val;
    }

    // Реализация InputIterator для совместимости со стандартной библиотекой
    class Iterator
    {
    public:
        // Определение типов для итератора (требование STL)
        using iterator_category = std::input_iterator_tag;
        using value_type = double;
        using difference_type = std::ptrdiff_t;
        using pointer = const double*;
        using reference = const double&;

    private:
        SimpleRNG* rng_ptr;   // Указатель на генератор
        double value;         // Значение, на которое указывает итератор
        double eps;           // Точность сравнения (для end)
        bool is_sentinel;     // Флаг, является ли итератор "концевым"

    public:
        // Конструктор для обычного итератора
        Iterator(SimpleRNG* ptr, double v) 
            : rng_ptr(ptr), value(v), eps(0), is_sentinel(false) 
        {
        }

        // Конструктор для итератора end()
        Iterator(double target, double _eps) 
            : rng_ptr(nullptr), value(target), eps(_eps), is_sentinel(true) 
        {
        }

        // Оператор разыменования
        reference operator*() const 
        { 
            return value; 
        }

        // Префиксный инкремент: генерация следующего числа
        Iterator& operator++()
        {
            if (rng_ptr)
            {
                // Вычисляем следующее значение по формуле
                // Используем fmod, так как работаем с double
                double next = std::fmod((rng_ptr->a * value + rng_ptr->c), rng_ptr->m);
                
                value = next;
                rng_ptr->current = next; // Обновляем состояние генератора
            }
            return *this;
        }

        // Постфиксный инкремент
        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        // Оператор сравнения
        bool operator==(const Iterator& other) const
        {
            // Сравнение с "концевым" итератором
            // Проверяем, вернулись ли мы к началу цикла с заданной точностью
            if (other.is_sentinel)
            {
                return std::abs(value - other.value) < other.eps;
            }
            
            if (is_sentinel)
            {
                return std::abs(other.value - value) < eps;
            }
            
            // Сравнение двух обычных итераторов
            return value == other.value;
        }

        bool operator!=(const Iterator& other) const 
        { 
            return !(*this == other); 
        }
    };

    // Возвращает итератор на текущее состояние
    Iterator begin() 
    { 
        return Iterator(this, current); 
    }

    // Возвращает итератор конца.
    // Он "ждет" появления числа start_val с погрешностью eps.
    Iterator end(double eps = 0.05) 
    { 
        return Iterator(start_val, eps); 
    }
};
