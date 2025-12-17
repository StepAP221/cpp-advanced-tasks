#pragma once
#include <array>
#include <vector>
#include <stdexcept>

// Задание 2: Маска
template <size_t N>
class Mask
{
private:
    std::array<int, N> _mask;

public:
    // Variadic template конструктор
    template <typename... Args>
    Mask(Args... args) : _mask{ static_cast<int>(args)... }
    {
        static_assert(sizeof...(Args) == N, "Неверное количество аргументов маски");
        
        // Проверяем, что только 0 и 1
        for (int x : _mask)
        {
            if (x != 0 && x != 1) throw std::logic_error("В маске только 0 и 1");
        }
    }

    size_t size() const { return N; }

    int at(size_t i) const
    {
        if (i >= N) throw std::out_of_range("Выход за пределы маски");
        return _mask[i];
    }

    // Slice: меняет контейнер на месте
    template <typename T>
    void slice(T& cont)
    {
        auto reader = cont.begin();
        auto writer = cont.begin();
        size_t idx = 0;
        size_t count = 0;

        while (reader != cont.end())
        {
            // Если в маске 1 - оставляем
            if (_mask[idx % N] == 1)
            {
                if (writer != reader) 
                    *writer = std::move(*reader);
                
                ++writer;
                count++;
            }
            ++reader;
            ++idx;
        }

        // Обрезаем лишнее
        if (count < cont.size()) cont.resize(count);
    }

    // Transform: новый контейнер с измененными элементами
    template <typename T, typename Func>
    T transform(const T& src, Func f)
    {
        T res = src;
        size_t idx = 0;
        for (auto& el : res)
        {
            if (_mask[idx % N] == 1) el = f(el);
            idx++;
        }
        return res;
    }

    // Slice + Transform
    template <typename T, typename Func>
    T slice_and_transform(const T& src, Func f)
    {
        T res;
        // Резервируем память, если это вектор
        if constexpr (std::is_same_v<T, std::vector<typename T::value_type>>)
            res.reserve(src.size());

        size_t idx = 0;
        for (const auto& el : src)
        {
            if (_mask[idx % N] == 1) res.push_back(f(el));
            idx++;
        }
        return res;
    }
};
