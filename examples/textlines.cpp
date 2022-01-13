#include "askiplot/askiplot.hpp"

#include <iostream>
#include <random>
#include <unordered_map>

using namespace std;
using namespace askiplot;

int main() {

  Plot p;
  p
   .SetBrush("LineVertical", DefaultBrushLineVertical) // Optional
   .DrawLineVerticalAtCol(13)
   .DrawLineVerticalAtCol(15)
   .SetBrush("LineVertical", "!")
   .DrawLineVerticalAtCol(0.5)
   .SetBrush("LineHorizontal", ".")
   .DrawLineHorizontalAtRow(p.GetHeight() - 2)
   .DrawLineHorizontalAtRow(1)
   .DrawText("North", North)
   .DrawText("South", South)
   .DrawText("East", East)
   .DrawText("West", West)
   .DrawText("NorthEast", NorthEast)
   .DrawText("NorthWest", NorthWest)
   .DrawText("SouthEast", SouthEast)
   .DrawText("SouthWest", SouthWest)
   .DrawText("Center", Center)
   .DrawTextCentered("Centered text at South + Offset(0,2)", South + Offset(0,2))
   .DrawTextCentered("Centered text at South", South)
   .SetBrush("LineHorizontal", ">")
   .DrawLineHorizontalAtRow(0.66)
   .SetBrush("LineHorizontal", "<")
   .DrawLineHorizontalAtRow(0.33)
   .DrawTextVerticalCentered("Vertical text", East - Offset(10,0))
   .DrawText("{3,3}", {3,3})
  ;

  cout << p.Serialize();
  return 0;
}