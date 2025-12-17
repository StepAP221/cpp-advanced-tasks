#pragma once
#include <new>          // Для placement new
#include <stdexcept>    // Для std::exception
#include <string>
#include <cstddef>      // Для std::byte
#include <utility>      // Для std::forward

// --- Классы ошибок (Исключения) ---

// Ошибка: закончилось место
class NoSpaceError : public std::exception 
{
    std::string _err_msg;
public:
    NoSpaceError(size_t count) : _err_msg("Not enough slots. Objects created: " + std::to_string(count)) {}
    const char* what() const noexcept override { return _err_msg.c_str(); }
};

// Ошибка: обращение к пустому слоту
class EmptySlotAccessError : public std::exception 
{
public:
    const char* what() const noexcept override { return "Accessing an empty or invalid slot"; }
};

// Ошибка: объект не найден в хранилище
class ObjectNotManagedError : public std::exception 
{
public:
    const char* what() const noexcept override { return "Object pointer does not belong to this storage"; }
};


/**
 * Задача 3: Выделятор памяти (MemReserver).
 * T - тип хранимого объекта.
 * Capacity - максимальное количество объектов.
 */
template <typename T, size_t Capacity>
class MemReserver
{
private:
    // Буфер сырой памяти. 
    // alignas(T) гарантирует, что память будет выровнена правильно для типа T.
    // Если этого не сделать, программа может упасть на некоторых процессорах.
    alignas(T) std::byte _storage_buffer[Capacity * sizeof(T)];
    
    // Массив флагов: занят слот или нет
    bool _is_occupied[Capacity];

    // Вспомогательный метод: превращает индекс в типизированный указатель T*
    T* _resolve_ptr(size_t index)
    {
        // reinterpret_cast "жестко" говорит компилятору считать эти байты объектом T
        return reinterpret_cast<T*>(&_storage_buffer[index * sizeof(T)]);
    }

    const T* _resolve_ptr(size_t index) const
    {
        return reinterpret_cast<const T*>(&_storage_buffer[index * sizeof(T)]);
    }

public:
    // Конструктор: просто помечаем все слоты свободными
    MemReserver()
    {
        for(size_t i = 0; i < Capacity; ++i) 
            _is_occupied[i] = false;
    }

    // Деструктор: обязательно нужно уничтожить живые объекты
    ~MemReserver()
    {
        for(size_t i = 0; i < Capacity; ++i)
        {
            if (_is_occupied[i])
            {
                // Явный вызов деструктора
                _resolve_ptr(i)->~T();
                _is_occupied[i] = false;
            }
        }
    }

    // Запрещаем копирование хранилища (слишком сложная логика для тестового задания)
    MemReserver(const MemReserver&) = delete;
    MemReserver& operator=(const MemReserver&) = delete;

    /**
     * Метод create.
     * Создает объект T в первом свободном слоте.
     * Args&&... - универсальные ссылки для передачи аргументов в конструктор T.
     */
    template <typename... Args>
    T& create(Args&&... args)
    {
        for(size_t i = 0; i < Capacity; ++i)
        {
            if (!_is_occupied[i])
            {
                T* ptr = _resolve_ptr(i);
                
                // Placement New:
                // Конструирует объект по адресу ptr, не выделяя новую память.
                // std::forward идеально передает аргументы (сохраняя const/r-value).
                new (ptr) T(std::forward<Args>(args)...);
                
                _is_occupied[i] = true;
                return *ptr;
            }
        }
        // Если цикл кончился, а место не найдено
        throw NoSpaceError(count());
    }

    /**
     * Метод для удаления объекта по индексу.
     * В C++ 'delete' - ключевое слово, поэтому метод назван 'delete_item'.
     */
    void delete_item(size_t index)
    {
        if (index >= Capacity || !_is_occupied[index])
        {
            throw EmptySlotAccessError();
        }
        
        // Вручную вызываем деструктор, чтобы объект корректно очистил свои ресурсы
        _resolve_ptr(index)->~T();
        _is_occupied[index] = false;
    }
    
    // Псевдоним для совместимости с формулировкой задания (если требуется именно "delete")
    // Но вызвать его как obj.delete(0) все равно нельзя в C++.
    void delete_obj(size_t index)
    {
        delete_item(index);
    }

    // Возвращает количество активных объектов
    size_t count() const
    {
        size_t cnt = 0;
        for(size_t i = 0; i < Capacity; ++i)
        {
            if (_is_occupied[i]) cnt++;
        }
        return cnt;
    }

    // Получение объекта по индексу
    T& get(size_t index)
    {
        if (index >= Capacity || !_is_occupied[index])
        {
            throw EmptySlotAccessError();
        }
        return *_resolve_ptr(index);
    }

    /**
     * Метод position.
     * Определяет индекс объекта в массиве через адресную арифметику.
     */
    size_t position(const T& object_ref)
    {
        // Получаем адреса в байтах
        const std::byte* obj_addr = reinterpret_cast<const std::byte*>(&object_ref);
        const std::byte* start_addr = &_storage_buffer[0];
        const std::byte* end_addr = &_storage_buffer[Capacity * sizeof(T)];

        // 1. Проверяем, лежит ли адрес в пределах нашего массива
        if (obj_addr < start_addr || obj_addr >= end_addr)
        {
            throw ObjectNotManagedError();
        }

        // Вычисляем смещение от начала
        std::ptrdiff_t byte_offset = obj_addr - start_addr;
        
        // 2. Проверяем, попадает ли адрес ровно на начало слота
        if (byte_offset % sizeof(T) != 0)
        {
            throw ObjectNotManagedError(); // Указатель указывает "в середину" объекта или слота
        }

        size_t index = byte_offset / sizeof(T);
        
        // 3. Проверяем, считается ли этот слот занятым
        if (!_is_occupied[index])
        {
            throw ObjectNotManagedError(); // Слот пуст, значит объект там "мертв"
        }

        return index;
    }
};
