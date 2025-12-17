#pragma once
#include <iterator>
#include <cmath>
#include <vector>

// Задание 1: Генератор (линейный конгруэнтный метод)
class SimpleRNG
{
private:
    double _a, _c, _m;
    double _start; // Начальное число
    double _curr;  // Текущее число

public:
    SimpleRNG(double a, double c, double m) 
        : _a(a), _c(c), _m(m), _start(0), _curr(0)
    {
    }

    void reset(double start_val)
    {
        _start = start_val;
        _curr = start_val;
    }

    void reset()
    {
        _curr = _start;
    }

    // Итератор для совместимости с STL
    class Iterator
    {
    public:
        // Трейты
        using iterator_category = std::input_iterator_tag;
        using value_type = double;
        using difference_type = std::ptrdiff_t;
        using pointer = const double*;
        using reference = const double&;

    private:
        SimpleRNG* _gen;
        double _val;
        double _eps;
        bool _is_end; 

    public:
        // Обычный конструктор
        Iterator(SimpleRNG* gen, double v) 
            : _gen(gen), _val(v), _eps(0), _is_end(false) 
        {
        }

        // Конструктор для end()
        Iterator(double target, double eps) 
            : _gen(nullptr), _val(target), _eps(eps), _is_end(true) 
        {
        }

        reference operator*() const { return _val; }

        Iterator& operator++()
        {
            if (_gen)
            {
                // Формула из задания
                double next = std::fmod((_gen->_a * _val + _gen->_c), _gen->_m);
                _val = next;
                _gen->_curr = next;
            }
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const Iterator& other) const
        {
            // Если сравниваем с end(), проверяем точность eps
            if (other._is_end)
                return std::abs(_val - other._val) < other._eps;
            
            if (_is_end)
                return std::abs(other._val - _val) < _eps;
            
            return _val == other._val;
        }

        bool operator!=(const Iterator& other) const { return !(*this == other); }
    };

    Iterator begin() { return Iterator(this, _curr); }
    
    // end хранит начальное значение, чтобы поймать цикл
    Iterator end(double eps = 0.05) { return Iterator(_start, eps); }
};
