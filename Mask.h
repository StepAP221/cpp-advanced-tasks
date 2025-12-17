#pragma once
#include <array>
#include <vector>
#include <stdexcept>

// Задание 2: Класс маски
template <size_t N>
class Mask
{
private:
    std::array<int, N> bits; // Хранение элементов маски

public:
    // Конструктор с переменным количеством аргументов.
    // Используем variadic templates для инициализации.
    template <typename... Args>
    Mask(Args... args) : bits{ static_cast<int>(args)... }
    {
        // Проверка количества аргументов на этапе компиляции
        static_assert(sizeof...(Args) == N, "Количество аргументов не соответствует размеру маски");
        
        // Проверка значений (допускаются только 0 и 1)
        for (int val : bits)
        {
            if (val != 0 && val != 1) 
                throw std::logic_error("Маска может содержать только 0 и 1");
        }
    }

    size_t size() const 
    { 
        return N; 
    }

    int at(size_t index) const
    {
        if (index >= N) throw std::out_of_range("Индекс выходит за пределы маски");
        return bits[index];
    }

    // Метод slice: удаляет элементы, где маска равна 0.
    // Работает "на месте" (in-place), без создания копии контейнера.
    template <typename Container>
    void slice(Container& cont)
    {
        auto read_it = cont.begin();  // Итератор чтения
        auto write_it = cont.begin(); // Итератор записи
        
        size_t idx = 0;
        size_t kept_count = 0; // Считаем, сколько элементов оставили

        while (read_it != cont.end())
        {
            // Циклический доступ к элементам маски
            if (bits[idx % N] == 1)
            {
                // Если элемент нужно сохранить
                if (write_it != read_it) 
                {
                    *write_it = std::move(*read_it); // Перемещаем элемент, чтобы не копировать
                }
                ++write_it;
                kept_count++;
            }
            // Если маска 0, элемент просто пропускается (перезаписывается позже или удаляется)
            
            ++read_it;
            ++idx;
        }

        // Уменьшаем размер контейнера до фактического количества элементов
        if (kept_count < cont.size())
        {
            cont.resize(kept_count);
        }
    }

    // Transform: создает новый контейнер, изменяя элементы по маске
    template <typename Container, typename Func>
    Container transform(const Container& src, Func f)
    {
        Container result = src; // Создаем копию
        size_t idx = 0;
        
        for (auto& item : result)
        {
            if (bits[idx % N] == 1)
            {
                item = f(item); // Применяем функцию преобразования
            }
            idx++;
        }
        return result;
    }

    // Slice and Transform: и фильтрация, и изменение в новом контейнере
    template <typename Container, typename Func>
    Container slice_and_transform(const Container& src, Func f)
    {
        Container result;
        // Оптимизация: резервируем память, если контейнер это поддерживает (например, vector)
        if constexpr (std::is_same_v<Container, std::vector<typename Container::value_type>>)
        {
            result.reserve(src.size());
        }

        size_t idx = 0;
        for (const auto& item : src)
        {
            if (bits[idx % N] == 1)
            {
                result.push_back(f(item)); // Добавляем только подходящие элементы
            }
            idx++;
        }
        return result;
    }
};
