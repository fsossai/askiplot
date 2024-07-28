#include <chrono>
#include <iostream>
#include <thread>

#include <askiplot.hpp>

using namespace std;
using namespace std::chrono_literals;
using namespace askiplot;

int main() {
  Plot p;

  auto box = Plot(16, 5).DrawTextCentered("AskiPlot", Center);

  while (true) {
    if (rand() % 2) {
      p.DrawLineVerticalAtCol((float)rand() / (float)RAND_MAX);
    } else {
      p.DrawLineHorizontalAtRow((float)rand() / (float)RAND_MAX);
    }

    p.Fuse(box, Center - Offset(box.GetWidth()/2, box.GetHeight()/2));

    cout << p.Serialize();
    this_thread::sleep_for(100ms); 
  }
  
  return 0;
}
