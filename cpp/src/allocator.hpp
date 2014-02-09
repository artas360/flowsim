#ifndef __ALLOCATOR_HPP__
#define __ALLOCATOR_HPP__

#include "include.hpp"

struct FooAllocator {
        template <class T, typename... Args>
        T* construct(Args&&... params) {return new T(params...);}
        template <class T, typename... Args>
        void construct(T* p, Args&&... params) {p = new T(params...);}
        template <class T>
        void destroy(T* t) {delete t;}
};

#endif
