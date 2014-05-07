#ifndef __EXCEPTIONHPP__
#define __EXCEPTIONHPP__

#include <exception>
#include <cstring>
#include "include.hpp"

#undef GEN_EXCEPTION

#define GEN_EXCEPTION(NAME)\
    struct NAME : public std::exception {\
        NAME(const char* m = "") {strncpy(buf, m, 98); buf[99] = '\0';}\
        const char* what() const throw() {return buf;}\
        char buf[100];\
    };

GEN_EXCEPTION(Wrong_parameter)
GEN_EXCEPTION(Edge_allocation_error)
GEN_EXCEPTION(No_path_error)
GEN_EXCEPTION(Ressource_allocation_error)
GEN_EXCEPTION(Not_registered_flow)
GEN_EXCEPTION(Not_registered_value)
GEN_EXCEPTION(Not_implemented_yet)
GEN_EXCEPTION(Loop_error)
GEN_EXCEPTION(Configuration_error)
GEN_EXCEPTION(Duplicated_node_error)

#endif
