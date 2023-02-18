#include <iostream>
#include <random>
#include <unordered_map>
#include <askiplot.hpp>

using namespace std;
using namespace askiplot;

int main() {
  const int N = 10000; // Number of samples
  random_device rd;
  mt19937 mt(rd());
  normal_distribution<> nd(0, 1);
  vector<double> gsamples(N);
  for (auto& i : gsamples) {
      i = nd(mt);
  }

  HistPlot hp;
  hp
    .DrawBorders(All)
    .SetTitle("Gaussian distribution")
    .DrawTitle()
    .SetBrush("Area", "@")
    .SetBrush("BorderTop", " ")
    .PlotHistogram(gsamples, "Normal (0,1)")
    .DrawText("Number of samples: " + to_string(N), NorthWest + Offset(2, -2))
    .DrawLegend()
  ;
  
  // Drawing everthing
  cout << hp.Serialize();
  
  return 0;
}
