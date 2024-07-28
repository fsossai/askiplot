#include <iostream>
#include <vector>

#include <askiplot.hpp>

using namespace std;
using namespace askiplot;

int main() {
  BarPlot bp;
  auto gb = GroupedBars(bp);
  gb
    .Add(vector<int>{80,40}, "Data Source 1", Brush('@'))
    .Add(vector<int>{20,50}, "Data Source 2", Brush('$'))
    .Add(vector<int>{10,20}, "Data Source 3", Brush('.'))
    .Commit();
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
