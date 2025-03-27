为什么要设计MemoryPool:
   C++默认的Allocator的allocate和dellocate操作只是简单的new和delete操作，但是这样会导致重复调用操作系统的原语，并且进行内核态和用户态的转换，
   导致大量的开销，因此可以通过将申请的空间进行重复利用来进行优化，即通过将销毁的空间进行重复利用，而不直接进行删除来实现，这可以通过维护一个空闲链表来实现，
   但是缺点就是无法针对同时需要申请多个变量的情况进行使用，只可以对一个个插入和删除的情况使用。

allocate函数介绍：优先分配空闲链表的空间，如果空闲链表不为空，则申请新的内存块，并且分配新的内存块的空间
```c++
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
```

deallocate函数介绍：当要释放空间的时候，将要释放的空间进插入到空闲链表头部
```c++
template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocate(T* p, size_t n)
{
    // 将释放的内存区域插入空闲链表头部
    if(p != nullptr){
        reinterpret_cast<Slot_*>(p)->next = freeSlot;
        freeSlot = reinterpret_cast<Slot_*>(p);
    }
}
```

padPointer函数介绍：为了减少访存次数，进行内存对齐
```c++
template <typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::padPointer(char* p, size_t align) const
{
    // 转化当前的地址
    size_t result = reinterpret_cast<size_t>(p);
    // 计算偏移量
    return ((align - result) % align);
}
```
allocateBlock函数介绍：申请新的内存块，并且块头设置指针指向已申请的内存块，然后进行后续指针修改
```c++
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
```
剩下实现都较为简单，没啥可介绍的；

MemoryPool仅适用于每次进行单个申请的容器，如list，map等，因为底层每次只会返回一个空间，对于存在扩容策略的不支持




   
   
