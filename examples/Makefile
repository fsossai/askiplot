TARGETS = fusion.out gaussian.out grid.out bar_grouper.out textlines.out turing.out turing_animated.out logo.out lines.out

all: $(TARGETS)

%.out: %.cpp
	$(CXX) -std=c++17 -Wall -Wpedantic -Wextra -O3 -I ../include $< -o $@.out

clean:
	rm -f $(TARGETS)
