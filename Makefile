CC=g++

all: fusion	gaussian grid groupedbars	textlines

%: examples/%.cpp
	$(CC) -std=c++14 -Wall -Wpedantic -Wextra -O3 -Iinclude $< -o examples/$@.out

clean:
	rm -rf examples/*.out

.PHONY: all clean
