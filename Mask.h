#ifndef MASK_H
#define MASK_H

#include <array>
#include <vector>
#include <stdexcept>
#include <algorithm> // для std::copy, std::transform и т.д.

/*
 * Задача 2: Класс Mask
 * N - размер маски (параметр шаблона).
 */
template <size_t N>
class Mask {
private:
    std::array<int, N> mask_data; // Хранение маски

public:
    // Конструктор с переменным числом аргументов.
    // Проверяет кол-во аргументов на этапе компиляции (static_assert).
    template <typename... Args>
    Mask(Args... args) : mask_data{ static_cast<int>(args)... } {
        static_assert(sizeof...(Args) == N, "Количество аргументов должно совпадать с размером маски N");
        
        // Проверка значений (только 0 и 1)
        for (auto val : mask_data) {
            if (val != 0 && val != 1) {
                // В задании сказано про ошибку компиляции на размер, 
                // но значения проверяем в рантайме (или можно усложнить через constexpr)
                throw std::invalid_argument("Маска может содержать только 0 и 1");
            }
        }
    }

    // Возвращает размер маски
    size_t size() const {
        return N;
    }

    // Возвращает значение по индексу (с проверкой границ)
    int at(size_t index) const {
        if (index >= N) {
            throw std::out_of_range("Индекс маски вне диапазона");
        }
        return mask_data[index];
    }

    /*
     * Метод slice.
     * Видоизменяет переданный контейнер, удаляя элементы, где маска == 0.
     * Маска применяется циклично (index % N).
     * Работает через "два указателя" (read/write), чтобы не создавать копию.
     */
    template <typename Container>
    void slice(Container& c) {
        auto it = c.begin();
        auto write_it = c.begin(); // Куда записывать "выжившие" элементы
        size_t index = 0;
        size_t kept_count = 0;

        while (it != c.end()) {
            // Если в маске 1 — оставляем элемент
            if (mask_data[index % N] == 1) {
                if (write_it != it) {
                    *write_it = std::move(*it); // Перемещаем, если нужно сдвинуть
                }
                ++write_it;
                kept_count++;
            }
            // Если 0 — элемент пропускается (фактически удаляется)
            ++it;
            ++index;
        }

        // Обрезаем контейнер до нового размера
        // У вектора и строки есть метод resize, у других может быть erase
        // Предполагаем интерфейс, похожий на std::vector/std::string
        if (kept_count < c.size()) {
            c.resize(kept_count);
        }
    }

    /*
     * Метод transform.
     * Возвращает НОВЫЙ контейнер. Элементы под маской 1 изменяются функцией func.
     * Остальные копируются без изменений.
     */
    template <typename Container, typename Func>
    Container transform(const Container& c, Func func) {
        Container result = c; // Создаем копию
        size_t index = 0;
        
        for (auto& elem : result) {
            if (mask_data[index % N] == 1) {
                elem = func(elem); // Применяем функцию
            }
            index++;
        }
        return result;
    }

    /*
     * Метод slice_and_transform.
     * Возвращает НОВЫЙ контейнер, содержащий ТОЛЬКО элементы под маской 1,
     * к которым применена функция func.
     */
    template <typename Container, typename Func>
    Container slice_and_transform(const Container& c, Func func) {
        Container result;
        // Резервируем память, если это вектор (оптимизация)
        // if constexpr работает начиная с C++17
        if constexpr (std::is_same_v<Container, std::vector<typename Container::value_type>>) {
            result.reserve(c.size());
        }

        size_t index = 0;
        for (const auto& elem : c) {
            if (mask_data[index % N] == 1) {
                // Вставляем видоизмененный элемент
                // Используем push_back, предполагая последовательный контейнер
                result.push_back(func(elem));
            }
            index++;
        }
        return result;
    }
};

#endif
