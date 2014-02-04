#ifndef __ALLOCATOR_HPP__
#define __ALLOCATOR_HPP__

struct FooAllocator {
        template <class T, typename... Args>
        T* allocate(Args... params) {return new T(params...);}
        template <class T>
        void free(T* t) {delete t;}
};

#endif
