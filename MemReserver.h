#pragma once
#include <new>       // тут живет placement new
#include <stdexcept>
#include <string>
#include <cstddef>   // для std::byte
#include <utility>   // для forward

// Мои классы ошибок
class NoSpaceErr : public std::exception 
{
    std::string msg;
public:
    NoSpaceErr(size_t n) : msg("Места нет. Уже создано: " + std::to_string(n)) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

class BadSlotErr : public std::exception 
{
public:
    const char* what() const noexcept override { return "Слот пустой или кривой индекс"; }
};

class NotFoundErr : public std::exception 
{
public:
    const char* what() const noexcept override { return "Объект не отсюда"; }
};


// Класс-выделятор памяти
template <typename T, size_t Cap>
class MemReserver
{
private:
    // Буфер байтов. Важно: alignas(T) выравнивает память, иначе будет краш.
    alignas(T) std::byte storage[Cap * sizeof(T)];
    
    // Массив, где true значит слот занят
    bool used[Cap];

    // Хелпер, чтобы получить указатель на T по индексу
    T* ptr(size_t i)
    {
        // Кастуем сырые байты в указатель на T
        return reinterpret_cast<T*>(&storage[i * sizeof(T)]);
    }

    const T* ptr(size_t i) const
    {
        return reinterpret_cast<const T*>(&storage[i * sizeof(T)]);
    }

public:
    MemReserver()
    {
        // Сначала всё свободно
        for(size_t i=0; i<Cap; ++i) used[i] = false;
    }

    ~MemReserver()
    {
        // В деструкторе надо удалить всё, что осталось живым
        for(size_t i=0; i<Cap; ++i)
        {
            if (used[i])
            {
                // Явно вызываем деструктор, т.к. delete вызывать нельзя (память статическая)
                ptr(i)->~T();
                used[i] = false;
            }
        }
    }

    // Создание объекта. Args&&... чтобы передать любые аргументы конструктора
    template <typename... Args>
    T& create(Args&&... args)
    {
        // Ищем дырку
        for(size_t i=0; i<Cap; ++i)
        {
            if (!used[i])
            {
                T* p = ptr(i);
                // Placement New: создаем объект прямо по адресу p
                new (p) T(std::forward<Args>(args)...);
                
                used[i] = true;
                return *p;
            }
        }
        // Если дошли сюда, значит места нет
        throw NoSpaceErr(count());
    }

    // Удаление (назвал delete_obj, т.к. delete зарезервировано)
    void delete_obj(size_t i)
    {
        if (i >= Cap || !used[i]) throw BadSlotErr();
        
        // Руками зовем деструктор
        ptr(i)->~T();
        used[i] = false;
    }

    // Сколько сейчас объектов
    size_t count() const
    {
        size_t c = 0;
        for(size_t i=0; i<Cap; ++i) if (used[i]) c++;
        return c;
    }

    T& get(size_t i)
    {
        if (i >= Cap || !used[i]) throw BadSlotErr();
        return *ptr(i);
    }

    // Поиск индекса по ссылке на объект. Магия указателей.
    size_t position(const T& obj)
    {
        const std::byte* addr = reinterpret_cast<const std::byte*>(&obj);
        const std::byte* start = &storage[0];
        const std::byte* end = &storage[Cap * sizeof(T)];

        // Проверяем, внутри ли мы массива
        if (addr < start || addr >= end) throw NotFoundErr();

        // Считаем смещение в байтах
        std::ptrdiff_t diff = addr - start;
        
        // Должно делиться на размер T без остатка
        if (diff % sizeof(T) != 0) throw NotFoundErr();

        size_t idx = diff / sizeof(T);
        
        // И слот должен быть помечен как занятый
        if (!used[idx]) throw NotFoundErr();

        return idx;
    }
};
