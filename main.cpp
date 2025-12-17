#include <iostream>
#include <vector>
#include <string>
#include "SimpleRNG.h"
#include "Mask.h"
#include "MemReserver.h"

struct Test 
{
    int id;
    Test(int i) : id(i) { std::cout << "ctor " << id << "\n"; }
    ~Test() { std::cout << "dtor " << id << "\n"; }
};

int main() 
{
    std::cout << "--- 1. RNG ---\n";
    SimpleRNG rng(5, 0.2, 1); // a, c, m
    rng.reset(0.4);

    std::vector<double> vec;
    // Копируем пока не вернемся в начало (0.4) с точностью 0.001
    auto it = rng.begin();
    auto end = rng.end(0.001);

    for(int k=0; k<20 && it != end; ++k, ++it)
    {
        vec.push_back(*it);
    }

    for (auto v : vec) std::cout << v << " ";
    std::cout << "\n\n";

    std::cout << "--- 2. Mask ---\n";
    Mask<3> m(1, 0, 1);
    std::vector<int> nums = {10, 20, 30, 40, 50};
    
    // slice
    m.slice(nums); // оставит 10, 30, 40
    for(auto x : nums) std::cout << x << " ";
    std::cout << "\n";

    // transform
    auto res = m.transform(nums, [](int x){ return x*2; });
    for(auto x : res) std::cout << x << " ";
    std::cout << "\n\n";

    std::cout << "--- 3. Memory ---\n";
    MemReserver<Test, 2> mem;
    try 
    {
        auto& t1 = mem.create(1);
        auto& t2 = mem.create(2);
        
        std::cout << "Pos t1: " << mem.position(t1) << "\n";
        
        mem.delete_obj(0);
        mem.create(3); // займет 0 слот
        
        mem.create(4); // тут упадет
    }
    catch (const std::exception& e) 
    {
        std::cout << "Err: " << e.what() << "\n";
    }

    return 0;
}
