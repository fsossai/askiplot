#include "askiplot/askiplot.hpp"

#include <iostream>

using namespace std;
using namespace askiplot;

int main() {
  auto box1 = Plot(10,5).Fill(".").DrawTextCentered("BOX1", Center);
  auto box2 = Plot(box1).DrawTextCentered("BOX2", Center);

  Plot p(60,15);
  p.Fusion()(box1, NorthWest)
            (box1, SouthEast)
            (box2, NorthEast)
            (box2, SouthWest)
            .Fuse()
   .DrawLineHorizontalAtRow(0.5)
   .DrawLineVerticalAtCol(0.5)
  ;
  cout << p.Serialize();
  return 0;
}