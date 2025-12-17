#ifndef MEMRESERVER_H
#define MEMRESERVER_H

#include <cstddef> // std::byte
#include <new>     // placement new
#include <utility> // std::forward
#include <stdexcept>
#include <string>

// Исключения, требуемые заданием
class NotEnoughSlotsError : public std::exception {
    std::string message;
public:
    NotEnoughSlotsError(size_t count) {
        message = "Not enough slots. Objects created: " + std::to_string(count);
    }
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class EmptySlotError : public std::exception {
public:
    const char* what() const noexcept override { return "Slot is empty or invalid index"; }
};

class ObjectNotFoundError : public std::exception {
public:
    const char* what() const noexcept override { return "Object not found in storage"; }
};

/*
 * Задача 3: Выделятор памяти.
 * T - тип хранимого объекта, N - максимальное количество.
 */
template <typename T, size_t N>
class MemReserver {
private:
    // Статический массив байт для хранения объектов.
    // alignas(T) гарантирует правильное выравнивание памяти для типа T.
    alignas(T) std::byte storage[N * sizeof(T)];
    
    // Массив флагов: true, если слот занят объектом
    bool occupied[N];

    // Вспомогательный метод для получения "сырого" указателя на i-й слот
    T* get_ptr(size_t index) {
        return reinterpret_cast<T*>(&storage[index * sizeof(T)]);
    }

    const T* get_ptr(size_t index) const {
        return reinterpret_cast<const T*>(&storage[index * sizeof(T)]);
    }

public:
    // Конструктор: помечаем все слоты как свободные
    MemReserver() {
        for (size_t i = 0; i < N; ++i) {
            occupied[i] = false;
        }
    }

    // Деструктор: должен вызвать деструкторы всех живых объектов
    ~MemReserver() {
        for (size_t i = 0; i < N; ++i) {
            if (occupied[i]) {
                get_ptr(i)->~T(); // Явный вызов деструктора
                occupied[i] = false;
            }
        }
    }

    /*
     * Метод create.
     * Принимает аргументы конструктора T.
     * Ищет свободный слот и создает объект с помощью Placement New.
     */
    template <typename... Args>
    T& create(Args&&... args) {
        for (size_t i = 0; i < N; ++i) {
            if (!occupied[i]) {
                T* ptr = get_ptr(i);
                // Placement New: конструируем объект прямо по адресу ptr
                new (ptr) T(std::forward<Args>(args)...);
                occupied[i] = true;
                return *ptr;
            }
        }
        // Если места нет, бросаем исключение с количеством созданных объектов
        throw NotEnoughSlotsError(count());
    }

    /*
     * Метод delete (в C++ нельзя назвать метод ключевым словом delete).
     * Поэтому назовем его destroy, но в задании просят "delete". 
     * Если компилятор строгий, имя придется сменить. 
     * Здесь используем destroy для безопасности, но логика соответствует заданию.
     */
    void destroy(size_t index) {
        if (index >= N || !occupied[index]) {
            throw EmptySlotError();
        }
        // Явно вызываем деструктор
        get_ptr(index)->~T();
        occupied[index] = false;
    }

    // Возвращает количество живых объектов
    size_t count() const {
        size_t c = 0;
        for (size_t i = 0; i < N; ++i) {
            if (occupied[i]) c++;
        }
        return c;
    }

    // Получение объекта по индексу
    T& get(size_t index) {
        if (index >= N || !occupied[index]) {
            throw EmptySlotError();
        }
        return *get_ptr(index);
    }

    /*
     * Метод position.
     * Принимает ссылку на объект и возвращает его индекс в хранилище.
     * Работает через адресную арифметику.
     */
    size_t position(const T& obj) {
        const std::byte* obj_addr = reinterpret_cast<const std::byte*>(&obj);
        const std::byte* start_addr = &storage[0];
        const std::byte* end_addr = &storage[N * sizeof(T)];

        // Проверяем, лежит ли адрес внутри нашего массива storage
        if (obj_addr < start_addr || obj_addr >= end_addr) {
            throw ObjectNotFoundError();
        }

        std::ptrdiff_t diff = obj_addr - start_addr;

        // Проверяем, попадает ли адрес ровно на начало слота
        if (diff % sizeof(T) != 0) {
             throw ObjectNotFoundError();
        }

        size_t index = diff / sizeof(T);

        // Проверяем, числится ли этот слот занятым
        if (!occupied[index]) {
            throw ObjectNotFoundError(); // Слот пуст, значит объект "мертв"
        }

        return index;
    }
};

#endif
