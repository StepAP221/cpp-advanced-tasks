#include <iostream>
#include <vector>
#include <string>

// Подключение наших модулей
#include "SimpleRNG.h"
#include "Mask.h"
#include "MemReserver.h"

// Тестовый класс для проверки MemReserver (чтобы видеть вызов конструктора/деструктора)
struct TestData 
{
    int id;
    std::string label;

    TestData(int i, std::string s) : id(i), label(s) 
    { 
        std::cout << "  [TestData Constructor] ID: " << id << ", Label: " << label << "\n"; 
    }
    
    ~TestData() 
    { 
        std::cout << "  [TestData Destructor]  ID: " << id << "\n"; 
    }
};

int main() 
{
    // ==========================================
    // ТЕСТ 1: Генератор случайных чисел
    // ==========================================
    std::cout << "--- Test 1: SimpleRNG ---\n";
    
    // Параметры: a=5, c=0.2, m=1
    SimpleRNG generator(5.0, 0.2, 1.0);
    generator.reset(0.4); // Начальное состояние X[0] = 0.4

    std::vector<double> results;
    
    // Используем итераторы. end(0.001) означает: 
    // "остановись, когда число станет равно 0.4 с точностью 0.001"
    auto it_begin = generator.begin();
    auto it_end = generator.end(0.001);

    // Собираем числа вручную (аналог std::copy)
    int safety_counter = 0;
    for (auto it = it_begin; it != it_end; ++it)
    {
        results.push_back(*it);
        
        // Защита от бесконечного цикла (если параметры подобраны так, что цикл не замыкается)
        if (++safety_counter > 20) break; 
    }

    std::cout << "Generated sequence: ";
    for (double val : results) std::cout << val << " ";
    std::cout << "\n\n";


    // ==========================================
    // ТЕСТ 2: Маска (Mask)
    // ==========================================
    std::cout << "--- Test 2: Mask ---\n";
    
    // Маска: 1 (оставить), 0 (убрать), 1 (оставить)
    Mask<3> filter_mask(1, 0, 1); 
    
    std::vector<int> numbers = {10, 20, 30, 40, 50, 60, 70};
    
    std::cout << "Original vector: ";
    for (auto n : numbers) std::cout << n << " ";
    std::cout << "\n";

    // 1. Тест метода slice (модификация)
    // Паттерн {1,0,1} применяется к {10,20,30, 40,50,60, 70}
    // Ожидаем: 10(да), 20(нет), 30(да), 40(да), 50(нет), 60(да), 70(да) -> {10, 30, 40, 60, 70}
    filter_mask.slice(numbers);
    
    std::cout << "After slice:     ";
    for (auto n : numbers) std::cout << n << " ";
    std::cout << "\n";

    // 2. Тест transform
    // Умножаем элементы на 2 согласно маске
    std::vector<int> source = {1, 2, 3, 4, 5};
    auto transformed = filter_mask.transform(source, [](int x) { return x * 2; });
    
    std::cout << "Transformed:     ";
    for (auto n : transformed) std::cout << n << " ";
    std::cout << "\n\n";


    // ==========================================
    // ТЕСТ 3: Выделятор памяти (MemReserver)
    // ==========================================
    std::cout << "--- Test 3: MemReserver ---\n";
    
    // Создаем пул на 2 объекта
    MemReserver<TestData, 2> memory_pool;

    try 
    {
        // Создаем объекты
        TestData& obj1 = memory_pool.create(101, "Alpha");
        TestData& obj2 = memory_pool.create(102, "Beta");
        
        std::cout << "Objects count: " << memory_pool.count() << "\n";
        
        // Проверяем поиск позиции
        size_t pos = memory_pool.position(obj1);
        std::cout << "Obj1 is at index: " << pos << "\n";

        // Удаляем первый объект
        memory_pool.delete_item(pos);
        std::cout << "Obj1 deleted. Count: " << memory_pool.count() << "\n";

        // Проверяем переполнение
        memory_pool.create(103, "Gamma"); // Займет место удаленного obj1
        memory_pool.create(104, "Delta"); // ОШИБКА: места нет (всего 2 слота)
    }
    catch (const std::exception& ex) 
    {
        std::cout << "EXCEPTION caught: " << ex.what() << "\n";
    }

    return 0;
}
