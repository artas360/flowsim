CXX=clang++
CXXFLAGS=-std=c++11 -Wall -Wextra -Werror -O3 -DNDEBUG -march=native -I./src $(OPTFLAGS)
LDFLAGS= $(OPTLIBS)
LDCONFIG=-lxerces-c
SOURCES=$(wildcard src/**/*.cpp src/*.cpp)
HEADERS=$(wildcard src/**/*.hpp src/*.hpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCES))

TEST_SRC=$(wildcard tests/test_*.cpp)
TESTS=$(patsubst %.cpp,%,$(TEST_SRC))

TARGET=flowsim

EXECUTABLES=flowsim flowsim-debug

all: $(TARGET) tests

debug: CXXFLAGS=-std=c++11 -Wall -Wextra -Werror -g3 -march=native -I./src $(OPTFLAGS)
debug: all

$(TARGET): CXXFLAGS += -DSIMULATION
$(TARGET): build
		$(CXX) $(CXXFLAGS) src/simulation.cpp $(LDCONFIG) -o build/$(TARGET)

build:
		@mkdir -p build
		@mkdir -p bin

flowsim: $(SOURCES)

.PHONY: tests clean valgrind
tests:
tests: $(TESTS)
		sh ./tests/runtests.sh

valgrind:
		VALGRIND="valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE)

doc: $(SOURCES) $(HEADERS)
		doxygen Doxyfile

clean:
		rm -rf build $(OBJECTS) $(TESTS)
		rm -f tests/tests.log
		rm -f build/$(TARGET)
		rm -rf doc/*
