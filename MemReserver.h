#pragma once
#include <new>       // Для placement new
#include <stdexcept>
#include <string>
#include <cstddef>   // Для std::byte
#include <utility>   // Для std::forward

// Классы исключений
class NotEnoughSlotsError : public std::exception 
{
    std::string msg;
public:
    NotEnoughSlotsError(size_t n) : msg("Недостаточно места. Создано объектов: " + std::to_string(n)) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

class InvalidSlotError : public std::exception 
{
public:
    const char* what() const noexcept override { return "Слот пуст или некорректный индекс"; }
};

class ObjectNotFoundError : public std::exception 
{
public:
    const char* what() const noexcept override { return "Объект не принадлежит этому хранилищу"; }
};


// Задание 3: Класс для управления статической памятью
template <typename T, size_t Capacity>
class MemReserver
{
private:
    // Буфер памяти. 
    // alignas(T) необходим для корректного выравнивания данных.
    alignas(T) std::byte storage[Capacity * sizeof(T)];
    
    // Массив флагов занятости слотов
    bool is_occupied[Capacity];

    // Вспомогательный метод для получения указателя на объект по индексу
    T* get_ptr(size_t index)
    {
        return reinterpret_cast<T*>(&storage[index * sizeof(T)]);
    }

    const T* get_ptr(size_t index) const
    {
        return reinterpret_cast<const T*>(&storage[index * sizeof(T)]);
    }

public:
    MemReserver()
    {
        for(size_t i = 0; i < Capacity; ++i) 
            is_occupied[i] = false;
    }

    ~MemReserver()
    {
        // Очистка оставшихся объектов при удалении хранилища
        for(size_t i = 0; i < Capacity; ++i)
        {
            if (is_occupied[i])
            {
                get_ptr(i)->~T(); // Явный вызов деструктора
                is_occupied[i] = false;
            }
        }
    }

    // Создание объекта в свободном слоте
    template <typename... Args>
    T& create(Args&&... args)
    {
        // Поиск первого свободного слота
        for(size_t i = 0; i < Capacity; ++i)
        {
            if (!is_occupied[i])
            {
                T* ptr = get_ptr(i);
                // Placement New: конструирование объекта в выделенной памяти
                new (ptr) T(std::forward<Args>(args)...);
                
                is_occupied[i] = true;
                return *ptr;
            }
        }
        throw NotEnoughSlotsError(count());
    }

    // Удаление объекта по индексу
    void delete_obj(size_t index)
    {
        if (index >= Capacity || !is_occupied[index]) 
            throw InvalidSlotError();
        
        // Ручной вызов деструктора обязателен для placement new
        get_ptr(index)->~T();
        is_occupied[index] = false;
    }

    // Количество занятых слотов
    size_t count() const
    {
        size_t cnt = 0;
        for(size_t i = 0; i < Capacity; ++i) 
        {
            if (is_occupied[i]) cnt++;
        }
        return cnt;
    }

    T& get(size_t index)
    {
        if (index >= Capacity || !is_occupied[index]) 
            throw InvalidSlotError();
        return *get_ptr(index);
    }

    // Определение индекса объекта по указателю (адресная арифметика)
    size_t position(const T& obj)
    {
        const std::byte* obj_addr = reinterpret_cast<const std::byte*>(&obj);
        const std::byte* start_addr = &storage[0];
        const std::byte* end_addr = &storage[Capacity * sizeof(T)];

        // Проверка: находится ли адрес внутри массива storage
        if (obj_addr < start_addr || obj_addr >= end_addr) 
            throw ObjectNotFoundError();

        // Вычисление смещения
        std::ptrdiff_t offset = obj_addr - start_addr;
        
        // Проверка выравнивания (адрес должен быть кратен размеру T)
        if (offset % sizeof(T) != 0) 
            throw ObjectNotFoundError();

        size_t index = offset / sizeof(T);
        
        // Проверка, что слот действительно занят
        if (!is_occupied[index]) 
            throw ObjectNotFoundError();

        return index;
    }
};
