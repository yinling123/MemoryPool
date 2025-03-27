#ifndef MEMORYPOOL_CPP
#define MEMORYPOOL_CPP

#include "../../include/MemoryPool.h"

/**
 * 初始化所有变量为nullptr
 */
template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(){
    currentBlock = nullptr;
    currentSlot = nullptr;
    lastSlot = nullptr;
    freeSlot = nullptr;
}


/**
 * 对于拷贝构造函数，我们只需要调用默认构造函数即可
 */
template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool& memoryPool){
    MemoryPool();
}

/**
 * 进行移动构造，将内容全部移动到新的内存池中
 */
template<typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(MemoryPool&& memoryPool){
    currentBlock = memoryPool.currentBlock;    
    memoryPool.currentBlock = nullptr;
    currentSlot = memoryPool.currentSlot;
    memoryPool.currentSlot = nullptr;
    lastSlot = memoryPool.lastSlot;
    memoryPool.lastSlot = nullptr;
    freeSlot = memoryPool.freeSlots;
    memoryPool.freeSlots = nullptr;
}


/**
 * 对于移动赋值函数，进行资源转换
 */
template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>& MemoryPool<T, BlockSize>::operator=(MemoryPool && memoryPool)
{   

    // 确保两个类型不一致
    if(memoryPool != *this){
        currentBlock = memoryPool.currentBlock;
        memoryPool.currentBlock = nullptr;
        currentSlot = memoryPool.currentSlot;
        memoryPool.currentSlot = nullptr;
        lastSlot = memoryPool.lastSlot;
        memoryPool.lastSlot = nullptr;
        freeSlot = memoryPool.freeSlots;
        memoryPool.freeSlots = nullptr;
    }    
}


/**
 * 析构函数，释放链表中的所有块
 */
template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool()
{
    Slot_* curr = currentBlock;
    // 遍历块进行释放内存
    while(curr != nullptr){
        Slot_* prev = curr;
        curr = curr->next;
        // 直接释放内存，性能更高
        operator delete(reinterpret_cast<void*>(prev));
    }
}

template <typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::allocate(size_t n, const T* hint)
{
    //如果有空闲插槽，从空闲插槽中分配空间
    if(freeSlot != nullptr){
        // 指针指向当前空闲块
        pointer result = reinterpret_cast<pointer>(freeSlot);
        // 空闲块指针指向下一个空闲块
        freeSlot = freeSlot -> next;
        return result;
    }else{
        // 如果没有空闲块，则要分配新的内存块
        if(currentSlot >= lastSlot){
            allocateBlock();
        }
        return (pointer)currentSlot++;
    }
    return nullptr;
}

/**
 * 释放对应的空间，并且指向下一个空闲块
 */
template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocate(T* p, size_t n)
{
    // 将释放的内存区域插入空闲链表头部
    if(p != nullptr){
        reinterpret_cast<Slot_*>(p)->next = freeSlot;
        freeSlot = reinterpret_cast<Slot_*>(p);
    }
}


/**
 * 获取最大内存块
 */
template <typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::max_size() const
{   
    // 计算最大内存块数量
    size_t maxBlocks = -1 / BlockSize;
    // 计算最大插槽的数量，减去指针的大小是因为开头需要存储下一个插槽的指针
    return (BlockSize - sizeof(Slot_*) / sizeof(Slot_)) * maxBlocks;
}

/**
 * 删除对应位置的对象，并且将该空间存入空闲链表
 */
template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deleteElement(pointer p)
{
    if(p != nullptr){
        p -> ~T();
        deallocate(p);
    }
}

/**
 * 计算对齐偏移量
 */
template <typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::padPointer(char* p, size_t align) const
{
    // 转化当前的地址
    size_t result = reinterpret_cast<size_t>(p);
    // 计算偏移量
    return ((align - result) % align);
}

/**
 * 分配内存块，并且进行内存对齐和修改各个指针指向
 */
template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::allocateBlock()
{
    // 将申请的内存解释为char*类型
    char* newBlock = reinterpret_cast<char*>(operator new(BlockSize));
    // 将当前内存块的指针指向当前的内存块
    reinterpret_cast<Slot_*>(newBlock)->next = currentBlock;
    // 当前块指针指向新的内存块
    currentBlock = reinterpret_cast<Slot_*>(newBlock);
    // 计算后续的存储空间位置
    char* body = newBlock + sizeof(Slot_*);
    // 计算对齐偏移量
    size_t bodyPadding = padPointer(body, alignof(Slot_));
    // 将当前插槽指针指向第一个插槽
    currentSlot = reinterpret_cast<Slot_*>(body + bodyPadding);
    // 表示最后一个不可用指针位置
    lastSlot = reinterpret_cast<Slot_*>(newBlock + BlockSize - sizeof(Slot_) + 1);
}

/**
 * 在p指针位置进行对象构造
 */
template <typename T, size_t BlockSize>
template <typename U, typename... Args>
inline void MemoryPool<T, BlockSize>::construct(U *p, Args &&...args)
{
    new (p) U(std::forward<Args>(args)...);
}

/**
 * 析构指定位置的对象，但是不释放内存
 */
template <typename T, size_t BlockSize>
template <typename U>
inline void MemoryPool<T, BlockSize>::destroy(U *p)
{
    p->~U();
}

/**
 * 创建一个对象，并且返回该对象的指针
 */
template <typename T, size_t BlockSize>
template <typename... Args>
inline typename MemoryPool<T, BlockSize>::pointer MemoryPool<T, BlockSize>::newElement(Args &&...args)
{
    pointer result = allocate();
    construct<T>(result, std::forward<Args>(args)...);
    return result;
}


#endif