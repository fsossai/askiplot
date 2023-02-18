#include <iostream>
#include <askiplot.hpp>

using namespace std;
using namespace askiplot;

int main() {
  Plot p;
  Image turing("turing.bmp");
  p.DrawImage(turing);
  cout << p.Serialize();

  return 0;
}
