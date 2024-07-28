#include <iostream>
#include <vector>

#include <askiplot.hpp>

using namespace std;
using namespace askiplot;

int main() {
  BarPlot bp(80, 25);
  BarGrouper(bp, askiplot::kSymbolBrushes)
    .Add(vector<int>{80,40}, "Data Source 1")
    .Add(vector<int>{20,50}, "Data Source 2", Brush('x'))
    .Add(vector<int>{10,20}, "Data Source 3")
    .Commit();
  bp
    .DrawBarLabels(Offset(0,1))
    .DrawLegend()
    .SetBrush("BorderTop", "/")
    .DrawBorders(Top + Right)
  ;
  cout << bp.Serialize();
  
  return 0;
}
