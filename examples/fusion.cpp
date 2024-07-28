#include <iostream>

#include <askiplot.hpp>

using namespace std;
using namespace askiplot;

int main() {
  auto box1 = Plot(10,5).Fill(".").DrawTextCentered("BOX1", Center);
  auto box2 = Plot(box1).DrawTextCentered("BOX2", Center);

  Plot p(60,15);
  p
    .Fuse(box1, NorthWest)
    .Fuse(box1, SouthEast)
    .Fuse(box2, NorthEast)
    .Fuse(box2, SouthWest)
    .DrawLineHorizontalAtRow(0.5)
    .DrawLineVerticalAtCol(0.5)
  ;
  cout << p.Serialize();
  return 0;
}
