#include <iostream>
#include <vector>

#include <askiplot.hpp>

using namespace std;
using namespace askiplot;

int main() {
  BarPlot bp;
  BarGrouper(bp, askiplot::kSymbolBrushes)
    .Add(vector<int>{80, 40}, Scaled, "Data Source 1")
    .Add(vector<int>{20, 50}, Scaled, "Data Source 2", Brush('x'))
    .Add(vector<int>{30, 15}, NotScaled, "Data Source 3 (not scaled)")
    .SetGroupNames({"Group 1", "Group 2"})
    .ShowGroupNames(true)
    .Commit();
  bp
    .DrawBarLabels(Offset(0, 1))
    .DrawLegend()
    .SetBrush("BorderTop", "/")
    .DrawBorders(Top + Right)
  ;
  cout << bp.Serialize();
  
  return 0;
}
