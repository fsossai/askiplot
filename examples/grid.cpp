#include "askiplot/askiplot.hpp"

#include <iostream>
#include <random>
#include <unordered_map>

using namespace std;
using namespace askiplot;

int main() {

  int width = 10, height = 5;
  GridPlot gp(2, 3, 3 * width, 2 * height); // Rows, Columns, Width, Height
  Plot base = Plot(width, height).Fill().DrawBorders(All - Bottom);
  
  Plot sp1 = Plot(base).SetMainBrush("1").Redraw();
  Plot sp2 = Plot(base).SetMainBrush("2").Redraw();
  Plot sp3 = Plot(base).SetMainBrush("3").Redraw();

  gp.SetInRowMajor()(sp1)(sp2)(sp3)(sp3)(sp2)(sp1).Set();
  gp.Get<Plot>(1,2).DrawTextCentered("--", Center);
  
  cout << gp.Serialize();
  return 0;
}