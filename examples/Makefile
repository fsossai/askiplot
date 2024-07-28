CXX=g++

all: fusion	gaussian grid bar_grouper textlines turing turing_animated logo lines

%: %.cpp
	$(CXX) -std=c++17 -Wall -Wpedantic -Wextra -O3 -I../include $< -o $@.out

clean:
	rm -rf *.out

.PHONY: all clean
