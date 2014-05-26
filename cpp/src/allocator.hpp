#ifndef __ALLOCATOR_HPP__
#define __ALLOCATOR_HPP__

#include "include.hpp"

struct FooAllocator {
        template <class Base, class T, typename... Args>
        T* construct(Args&&... params) {
            //T* result = reinterpret_cast<T*>(boost::pool_allocator<Base>::pool_allocator::allocate(sizeof (T)));
            //new (result) T(params...);
            //return result;
            return new T(params...);
        }
        //template <class T, typename... Args>
        //void construct(T* p, Args&&... params) {p = new T(params...);}

        //template <class Base, class T>
        template <class Base>
        void destroy(Base* t) {
            delete t;
            //boost::pool_allocator<Base>::pool_allocator::deallocate(t, sizeof(Base));
        }
};

#endif
