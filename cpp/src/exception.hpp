#ifndef __EXCEPTIONHPP__
#define __EXCEPTIONHPP__

#include <exception>
#include "include.hpp"

#undef GEN_EXCEPTION

#define GEN_EXCEPTION(NAME) class NAME : public std::exception {};

GEN_EXCEPTION(Wrong_parameter)
GEN_EXCEPTION(Edge_allocation_error)
GEN_EXCEPTION(No_path_error)
GEN_EXCEPTION(Ressource_allocation_error)
GEN_EXCEPTION(Not_registered_flow)

#endif
