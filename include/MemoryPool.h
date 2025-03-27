#ifndef MemoryPool_H
#define MemoryPool_H

#include<climits>
#include<cstddef>

/**
 * @brief 内存池
 */
template<typename T, size_t BlockSize = 4096>
class MemoryPool
{
public:

    typedef T               value_type;
    typedef T*              pointer;
    typedef T&              reference;
    typedef const T*        const_pointer;
    typedef const T&        const_reference;
    typedef size_t          size_type;
    typedef ptrdiff_t       difference_type;

    // 结构体模板
    template <typename U> 
    struct rebind {
      typedef MemoryPool<U> other;
    };

    // 声明无参构造函数
    MemoryPool();

    // 声明拷贝构造函数
    MemoryPool(const MemoryPool&);

    // 声明移动构造函数
    MemoryPool(MemoryPool&&);

    //声明拷贝赋值运算符
    MemoryPool& operator=(const MemoryPool&) = delete;
    
    //声明移动赋值运算符
    MemoryPool& operator=(MemoryPool&&);

    //声明析构函数
    ~MemoryPool();

    // 定义分配函数
    T* allocate(size_t n = 1, const T* hint = 0);

    // 定义释放函数
    void deallocate(T* p, size_t n = 1);

    // 定义获取最大分配数量函数
    size_t max_size() const;

    // 定义构造函数
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args);

    // 定义销毁函数
    template<typename U>
    void destroy(U* p);

    // 定义生成新对象函数
    template<typename... Args>
    pointer newElement(Args&&... args);

    // 定义销毁对象函数
    void deleteElement(pointer p);

    /**
     * 定义内存结构体
     * 同一类型的成员函数可以访问其他对象的私有成员
     */
    private:
        union Slot_
        {
            T element;
            Slot_* next;
        };
        
        // 定义指向当前块的指针
        Slot_* currentBlock;

        // 定义指向当前空闲插槽的指针
        Slot_* currentSlot;

        // 指向最后一个插槽的指针
        Slot_* lastSlot;

        // 指向空闲链表的指针
        Slot_* freeSlot;

        /**
         * 计算满足对齐要求的偏移量
         */
        size_t padPointer(char* p, size_t align) const;

        /**
         * 分配一个新块
         */
        void allocateBlock();

        // 编译时断言
        static_assert(BlockSize >= 2 * sizeof(Slot_), "BlockSize too small.");
};


#endif
