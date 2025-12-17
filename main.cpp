#include <iostream>
#include <vector>
#include <string>

#include "SimpleRNG.h"
#include "Mask.h"
#include "MemReserver.h"

// Структура для тестов памяти
struct TestEntity 
{
    int id;
    TestEntity(int v) : id(v) { std::cout << "  [TestEntity] Конструктор ID: " << id << "\n"; }
    ~TestEntity() { std::cout << "  [TestEntity] Деструктор ID: " << id << "\n"; }
};

int main() 
{
    std::cout << "=== Тест 1: Генератор (SimpleRNG) ===\n";
    // Параметры: a=5, c=0.2, m=1
    SimpleRNG gen(5, 0.2, 1);
    gen.reset(0.4);

    std::vector<double> results;
    // Используем итераторы. Копируем значения, пока не вернемся к 0.4 (точность 0.001)
    auto it = gen.begin();
    auto end_it = gen.end(0.001);

    int safety_limit = 0;
    while (it != end_it)
    {
        results.push_back(*it);
        ++it;
        // Ограничитель на случай бесконечного цикла
        if (++safety_limit > 20) break; 
    }

    std::cout << "Сгенерировано: ";
    for (auto val : results) std::cout << val << " ";
    std::cout << "\n\n";


    std::cout << "=== Тест 2: Маска (Mask) ===\n";
    // Маска: 1(оставить), 0(удалить), 1(оставить)
    Mask<3> m(1, 0, 1); 
    
    std::vector<int> data = {10, 20, 30, 40, 50};
    std::cout << "Исходные данные: ";
    for(auto x : data) std::cout << x << " ";
    std::cout << "\n";

    // Проверка slice
    m.slice(data); // Должны остаться 10, 30, 40
    std::cout << "После slice: ";
    for(auto x : data) std::cout << x << " ";
    std::cout << "\n";

    // Проверка transform
    auto changed = m.transform(data, [](int x){ return x * 2; });
    std::cout << "После transform (x2): ";
    for(auto x : changed) std::cout << x << " ";
    std::cout << "\n\n";


    std::cout << "=== Тест 3: Память (MemReserver) ===\n";
    MemReserver<TestEntity, 2> pool;

    try 
    {
        auto& obj1 = pool.create(100);
        auto& obj2 = pool.create(200);
        
        std::cout << "Занято слотов: " << pool.count() << "\n";
        
        // Проверка определения позиции
        size_t pos = pool.position(obj1);
        std::cout << "obj1 находится в слоте: " << pos << "\n";

        // Удаление объекта
        pool.delete_obj(pos);
        std::cout << "Удалили obj1. Занято: " << pool.count() << "\n";

        // Проверка повторного использования памяти
        pool.create(300); // Займет освободившийся слот
        
        // Проверка переполнения
        pool.create(400); // Должно выбросить исключение (мест больше нет)
    }
    catch (const std::exception& e) 
    {
        std::cout << "Исключение: " << e.what() << "\n";
    }

    return 0;
}
