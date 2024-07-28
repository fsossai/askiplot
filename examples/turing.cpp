#include <iostream>

#include <askiplot.hpp>

using namespace std;
using namespace askiplot;

int main() {
  cout << Plot{}.DrawImage(Image("turing.bmp")).Serialize();

  return 0;
}
