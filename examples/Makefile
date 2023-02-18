CC=g++

all: fusion	gaussian grid groupedbars	textlines

%: %.cpp
	$(CC) -std=c++14 -Wall -Wpedantic -Wextra -O3 -I../include $< -o $@.out

clean:
	rm -rf *.out

.PHONY: all clean
