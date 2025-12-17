#include <iostream>
#include <vector>
#include <string>

// Подключаем наши заголовочные файлы
#include "SimpleRNG.h"
#include "Mask.h"
#include "MemReserver.h"

// Класс для теста MemReserver
struct TestObj {
    int id;
    std::string name;
    TestObj(int i, std::string n) : id(i), name(n) {
        std::cout << "  [TestObj Constructed] " << name << "\n";
    }
    ~TestObj() {
        std::cout << "  [TestObj Destructed] " << name << "\n";
    }
};

int main() {
    // --- Тест 1: SimpleRNG ---
    std::cout << "=== 1. SimpleRNG Test ===\n";
    // Создаем генератор: m=1, a=5, c=0.2 (параметры из примера задания)
    SimpleRNG generator(5, 0.2, 1);
    generator.reset(0.4); // Начальное состояние

    std::vector<double> vec;
    // Копируем в вектор, пока не сработает условие остановки (повтор цикла)
    // end(0.01) означает точность сравнения 0.01
    std::copy(generator.begin(), generator.end(0.01), std::back_inserter(vec));

    std::cout << "Generated sequence: ";
    for (auto v : vec) std::cout << v << " ";
    std::cout << "\n\n";

    // --- Тест 2: Mask ---
    std::cout << "=== 2. Mask Test ===\n";
    Mask<3> mask(1, 0, 1); // Маска размера 3: оставить, убрать, оставить
    std::cout << "Mask size: " << mask.size() << "\n";

    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7};
    std::cout << "Original: ";
    for(int x : numbers) std::cout << x << " "; 
    std::cout << "\n";

    // Slice: модифицирует numbers
    // Маска {1,0,1} применяется циклично:
    // 1(keep), 2(drop), 3(keep), 4(keep), 5(drop), 6(keep), 7(keep)
    mask.slice(numbers);
    
    std::cout << "Sliced:   ";
    for(int x : numbers) std::cout << x << " "; 
    std::cout << "\n";

    // Transform: создает новый массив, умножая элементы под маской на 10
    std::vector<int> nums2 = {1, 2, 3, 4, 5};
    auto transformed = mask.transform(nums2, [](int x){ return x * 10; });
    
    std::cout << "Transformed (nums2): ";
    for(int x : transformed) std::cout << x << " ";
    std::cout << "\n\n";

    // --- Тест 3: MemReserver ---
    std::cout << "=== 3. MemReserver Test ===\n";
    // Резервируем место под 3 объекта TestObj
    MemReserver<TestObj, 3> reserver;

    try {
        // Создаем объекты
        TestObj& o1 = reserver.create(1, "Object 1");
        TestObj& o2 = reserver.create(2, "Object 2");
        
        std::cout << "Count created: " << reserver.count() << "\n";
        std::cout << "Position of o2: " << reserver.position(o2) << "\n"; // Должно быть 1

        // Удаляем первый объект
        size_t pos1 = reserver.position(o1);
        reserver.destroy(pos1); // Метод называется destroy (вместо delete)
        std::cout << "After delete o1, count: " << reserver.count() << "\n";

        // Пытаемся получить удаленный объект (ошибка)
        // reserver.get(pos1); // Выбросит EmptySlotError

        // Заполняем до отказа
        reserver.create(3, "Object 3");
        reserver.create(4, "Object 4");
        std::cout << "Full. Trying one more...\n";
        reserver.create(5, "Object 5"); // Должно выбросить исключение
    }
    catch (const std::exception& e) {
        std::cout << "EXCEPTION CAUGHT: " << e.what() << "\n";
    }

    return 0;
}
