#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <askiplot.hpp>

using namespace std;
using namespace std::chrono_literals;
using namespace askiplot;

int main() {
  Plot p;
  Image turing("turing.bmp");
  RandomGamma gamma("01");

  int zt = 0;
  int step = 5;

  while (true) {
    gamma.SetZeroThreshold(zt);
    p.DrawImage(turing, gamma);
    cout << p.Serialize();
    this_thread::sleep_for(125ms);

    zt += step;
    if (zt <= 0 || zt >= 255) {
      // Reverse fade direction
      if (zt <= 0) {
        zt = 0;
      } else if (zt >= 255) {
        zt = 255;
      }
      step *= -1;
    }
  }

  return 0;
}
