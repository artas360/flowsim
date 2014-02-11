#ifndef __INCLUDE_HPP__
#define __INCLUDE_HPP__

#include <cassert>
#include <cstdlib> // For EXIT_SUCCESS, EXIT_FAILURE

#include "exception.hpp"

#if TEST

#define FTEST(b) if(not (b)) {return EXIT_FAILURE;}

#define ASSERT_RAISES(f, ...) try {f(__VA_ARGS__); return EXIT_FAILURE;} catch(std::exception e) {}

#define ASSERT_NOT_RAISES(f, ...) try {f(__VA_ARGS__);} catch(std::exception e) {return EXIT_FAILURE;}

#endif

#endif
