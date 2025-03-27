#include<iostream>
#include<list>
#include<vector>
#include<time.h>
#include "../main/MemoryPool.hpp"

int main(){
    // 创建list
    std::list<int, std::allocator<int>> list;

    clock_t start, end;
    start = clock();
    for(int i = 0; i < 10000000; i++){
        list.push_back(i);
        list.push_back(i);
        list.push_back(i);
        list.push_back(i);
    }

    for(int i = 0; i < 10000000; i++){
        list.pop_back();
        list.pop_back();
        list.pop_back();
        list.pop_back();
    }

    end = clock();

    std::cout << "Time: " << double(end - start) / CLOCKS_PER_SEC << std::endl;

    // 内存池进行对比
    std::list<int, MemoryPool<int>> list2;

    start = clock();
    for(int i = 0; i < 10000000; i++){
        list2.push_back(i);
        list2.push_back(i);
        list2.push_back(i);
        list2.push_back(i);
    }

    for(int i = 0; i < 10000000; i++){
        list2.pop_back();
        list2.pop_back();
        list2.pop_back();
        list2.pop_back();
    }

    end = clock();

    std::cout << "Time: " << double(end - start) / CLOCKS_PER_SEC << std::endl;

    return 0;
}