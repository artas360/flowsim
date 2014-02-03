#ifndef __EXCEPTIONHPP__
#define __EXCEPTIONHPP__

#include <exception>

#undef GEN_EXCEPTION

#define GEN_EXCEPTION(NAME) class NAME : public std::exception {};

GEN_EXCEPTION(Wrong_parameter)
GEN_EXCEPTION(Edge_allocation_error)

#endif
