all: $(TARGETS)

%.out: %.cpp
	$(CXX) -std=c++17 -Wall -Wpedantic -Wextra -O3 -I ../include $< -o $@

clean:
	rm -f $(TARGETS)

