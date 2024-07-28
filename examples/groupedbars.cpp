#include <iostream>
#include <vector>

#include <askiplot.hpp>

using namespace std;
using namespace askiplot;

int main() {
  BarPlot bp;
  auto gb = GroupedBars<BarPlot>(bp, 2);
  gb
    (vector<int>{80,40}, "Data Source 1", Brush('@'))
    (vector<int>{20,50}, "Data Source 2", Brush('$'))
    (vector<int>{10,20}, "Data Source 3", Brush('.'))
    .Plot();
  bp
    .DrawBarLabels(Offset(0,1))
    .DrawLegend()
    .SetBrush("BorderTop", "/")
    .DrawBorders(Top + Right)
  ;
  // Drawing everthing
  cout << bp.Serialize();
  
  return 0;
}
