#pragma once
#include <new>
#include <stdexcept>
#include <string>
#include <cstddef>
#include <utility>

// Исключения
class NoSlots : public std::exception 
{
    std::string msg;
public:
    NoSlots(size_t n) : msg("Места нет. Создано: " + std::to_string(n)) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

class EmptySlot : public std::exception 
{
public:
    const char* what() const noexcept override { return "Слот пуст или индекс неверен"; }
};

class WrongObj : public std::exception 
{
public:
    const char* what() const noexcept override { return "Объект не из этого хранилища"; }
};


// Задание 3: Менеджер памяти
template <typename T, size_t Cap>
class MemReserver
{
private:
    // alignas нужен для правильного выравнивания T
    alignas(T) std::byte _mem[Cap * sizeof(T)];
    bool _used[Cap];

    T* ptr_at(size_t i) { return reinterpret_cast<T*>(&_mem[i * sizeof(T)]); }
    const T* ptr_at(size_t i) const { return reinterpret_cast<const T*>(&_mem[i * sizeof(T)]); }

public:
    MemReserver()
    {
        for(size_t i=0; i<Cap; ++i) _used[i] = false;
    }

    ~MemReserver()
    {
        // Удаляем оставшиеся объекты
        for(size_t i=0; i<Cap; ++i)
        {
            if (_used[i])
            {
                ptr_at(i)->~T();
                _used[i] = false;
            }
        }
    }

    // Создание объекта (placement new)
    template <typename... Args>
    T& create(Args&&... args)
    {
        for(size_t i=0; i<Cap; ++i)
        {
            if (!_used[i])
            {
                T* p = ptr_at(i);
                new (p) T(std::forward<Args>(args)...);
                _used[i] = true;
                return *p;
            }
        }
        throw NoSlots(count());
    }

    // Удаление по индексу
    void delete_obj(size_t i)
    {
        if (i >= Cap || !_used[i]) throw EmptySlot();
        
        ptr_at(i)->~T();
        _used[i] = false;
    }

    size_t count() const
    {
        size_t cnt = 0;
        for(size_t i=0; i<Cap; ++i) if (_used[i]) cnt++;
        return cnt;
    }

    T& get(size_t i)
    {
        if (i >= Cap || !_used[i]) throw EmptySlot();
        return *ptr_at(i);
    }

    // Поиск индекса по указателю
    size_t position(const T& obj)
    {
        const std::byte* addr = reinterpret_cast<const std::byte*>(&obj);
        const std::byte* start = &_mem[0];
        const std::byte* end = &_mem[Cap * sizeof(T)];

        if (addr < start || addr >= end) throw WrongObj();

        std::ptrdiff_t diff = addr - start;
        if (diff % sizeof(T) != 0) throw WrongObj(); // Не выровнен

        size_t idx = diff / sizeof(T);
        if (!_used[idx]) throw WrongObj();

        return idx;
    }
};
