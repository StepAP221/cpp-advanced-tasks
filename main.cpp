#include <iostream>
#include <vector>
#include <string>

#include "SimpleRNG.h"
#include "Mask.h"
#include "MemReserver.h"

// Класс для проверки вызовов конструкторов/деструкторов
struct TestItem 
{
    int val;
    TestItem(int v) : val(v) { std::cout << "  [TestItem] Конструктор " << val << "\n"; }
    ~TestItem() { std::cout << "  [TestItem] Деструктор " << val << "\n"; }
};

int main() 
{
    std::cout << "=== Задание 1: Генератор ===\n";
    // a=5, c=0.2, m=1
    SimpleRNG gen(5, 0.2, 1);
    gen.reset(0.4);

    std::vector<double> res;
    // Копируем, пока не вернемся к 0.4 с точностью 0.001
    auto it = gen.begin();
    auto stop = gen.end(0.001);

    int limit = 0;
    // Крутим цикл руками, чтобы было нагляднее
    while (it != stop)
    {
        res.push_back(*it);
        ++it;
        if (++limit > 20) break; // На всякий случай
    }

    std::cout << "Числа: ";
    for (auto x : res) std::cout << x << " ";
    std::cout << "\n\n";


    std::cout << "=== Задание 2: Маска ===\n";
    // Маска: да, нет, да
    Mask<3> m(1, 0, 1); 
    
    std::vector<int> nums = {10, 20, 30, 40, 50};
    std::cout << "Было: ";
    for(auto x : nums) std::cout << x << " ";
    std::cout << "\n";

    // Slice
    m.slice(nums); // Ожидаем 10, 30, 40 (50 отпадет т.к. 0 в маске)
    std::cout << "Стало (slice): ";
    for(auto x : nums) std::cout << x << " ";
    std::cout << "\n";

    // Transform
    auto changed = m.transform(nums, [](int x){ return x * 10; });
    std::cout << "Трансформ (x10): ";
    for(auto x : changed) std::cout << x << " ";
    std::cout << "\n\n";


    std::cout << "=== Задание 3: Память ===\n";
    MemReserver<TestItem, 2> mem;

    try 
    {
        auto& t1 = mem.create(111);
        auto& t2 = mem.create(222);
        
        std::cout << "Занято мест: " << mem.count() << "\n";
        
        // Проверка позиции
        size_t pos = mem.position(t1);
        std::cout << "t1 лежит в слоте: " << pos << "\n";

        // Удаление
        mem.delete_obj(pos);
        std::cout << "Удалили t1. Занято: " << mem.count() << "\n";

        // Создаем новые, проверяем переполнение
        mem.create(333); // займет 0-й слот
        mem.create(444); // ОШИБКА, места нет (всего 2)
    }
    catch (const std::exception& e) 
    {
        std::cout << "Поймали ошибку: " << e.what() << "\n";
    }

    return 0;
}
