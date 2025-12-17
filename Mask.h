#pragma once
#include <array>
#include <vector>
#include <stdexcept>    // Для исключений
#include <type_traits>  // Для проверки типов

/**
 * Задача 2: Класс Mask.
 * Позволяет фильтровать и преобразовывать контейнеры по битовому шаблону.
 * Size - размер маски, задается при компиляции.
 */
template <size_t Size>
class Mask
{
private:
    // Хранилище маски (статический массив, так как размер известен)
    std::array<int, Size> _bits;

public:
    /**
     * Конструктор с переменным количеством аргументов (Variadic Templates).
     * Позволяет создать маску так: Mask<3> m(1, 0, 1);
     */
    template <typename... Args>
    Mask(Args... args) : _bits{ static_cast<int>(args)... }
    {
        // Проверка на этапе компиляции: количество аргументов должно быть равно Size
        static_assert(sizeof...(Args) == Size, "Error: Argument count does not match mask size!");

        // Проверка в рантайме: значения могут быть только 0 или 1
        for (int bit : _bits)
        {
            if (bit != 0 && bit != 1)
            {
                throw std::logic_error("Mask values must be strictly 0 or 1");
            }
        }
    }

    // Возвращает размер маски
    size_t size() const
    {
        return Size;
    }

    // Безопасный доступ к элементу маски
    int at(size_t index) const
    {
        if (index >= Size) 
        {
            throw std::out_of_range("Mask index is out of range");
        }
        return _bits[index];
    }

    /**
     * Метод slice.
     * Модифицирует переданный контейнер "на месте" (in-place).
     * Удаляет элементы, соответствующие 0 в маске.
     */
    template <typename Container>
    void slice(Container& container)
    {
        auto read_it = container.begin();  // "Читающий" итератор
        auto write_it = container.begin(); // "Пишущий" итератор (всегда отстает или равен читающему)
        
        size_t step_counter = 0;
        size_t kept_elements = 0;

        while (read_it != container.end())
        {
            // Берем бит маски циклически: step % Size
            int current_bit = _bits[step_counter % Size];
            
            if (current_bit == 1)
            {
                // Если элемент нужно оставить:
                if (read_it != write_it)
                {
                    // Перемещаем элемент из "читающего" в "пишущий" (оптимизация вместо копирования)
                    *write_it = std::move(*read_it);
                }
                ++write_it;      // Сдвигаем позицию записи
                kept_elements++; // Увеличиваем счетчик оставленных
            }
            // Если current_bit == 0, мы просто двигаем read_it дальше, пропуская элемент
            
            ++read_it;
            ++step_counter;
        }

        // Обрезаем контейнер до реального количества оставшихся элементов.
        // Требует наличия метода resize() (есть у vector, deque, string).
        if (kept_elements < container.size())
        {
            container.resize(kept_elements);
        }
    }

    /**
     * Метод transform.
     * Возвращает НОВЫЙ контейнер.
     * К элементам под маской 1 применяется функция func. Остальные копируются.
     */
    template <typename Container, typename Func>
    Container transform(const Container& source, Func func)
    {
        Container result = source; // Создаем копию
        size_t i = 0;
        
        for (auto& item : result)
        {
            if (_bits[i % Size] == 1)
            {
                item = func(item); // Применяем функцию трансформации
            }
            i++;
        }
        return result;
    }

    /**
     * Метод slice_and_transform.
     * Комбинирует оба подхода: возвращает новый контейнер, содержащий 
     * ТОЛЬКО элементы под маской 1, к которым применена функция.
     */
    template <typename Container, typename Func>
    Container slice_and_transform(const Container& source, Func func)
    {
        Container result;
        // Оптимизация: резервируем память заранее, если это вектор
        if constexpr (std::is_same_v<Container, std::vector<typename Container::value_type>>)
        {
            result.reserve(source.size());
        }

        size_t i = 0;
        for (const auto& item : source)
        {
            if (_bits[i % Size] == 1)
            {
                // Трансформируем и добавляем в новый контейнер
                result.push_back(func(item));
            }
            i++;
        }
        return result;
    }
};
