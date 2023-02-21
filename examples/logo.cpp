#include <iostream>
#include <askiplot.hpp>
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono_literals;
using namespace askiplot;

int main() {
  Plot p;

  auto logo = Image("logo.bmp").Invert();

  auto s1 = p.DrawImage(logo).Serialize();
  auto s2 = p.DrawImage(logo, TextGamma("askiplot")).Serialize();

  auto interval1 = 1000ms;
  auto interval2 = 250ms;

  while (true) {
    cout << s1;
    this_thread::sleep_for(interval1);
    cout << s2;
    this_thread::sleep_for(interval2);
  }

  return 0;
}
