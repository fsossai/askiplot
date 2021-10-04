/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ASKIPLOT_HPP_
#define ASKIPLOT_HPP_

#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#define ASKIPLOT_VERSION_MAJOR 0
#define ASKIPLOT_VERSION_MINOR 1
#define ASKIPLOT_VERSION_PATCH 0

namespace askiplot {

const std::string kDefaultPenLine = "=";
const std::string kDefaultPenArea = "#";
const std::string kDefaultPenEmpty = " ";
const int kConsoleHeight = 0;
const int kConsoleWidth = 0;

using borders_t = char;

enum class CellStatus : char {
  Empty = 0, Filled
};

enum Borders : char {
  Left = 0x01, Right = 0x02, Bottom = 0x04, Top = 0x08
};

class InvalidPlotSize : public std::exception {
public:
  virtual const char* what() const noexcept override {
    return "Plot height and width must be positive integer numbers.";
  }
};

class PenNotSet : public std::exception {
public:
  virtual const char* what() const noexcept override {
    return "Trying to access a Pen pointer that has not been set.";
  }
};

class InconsistentData : public std::exception {
public:
  virtual const char* what() const noexcept override {
    return "Data to be drawn is inconsistent.";
  }
};

class Pen {
public:
  Pen() = default;
  virtual ~Pen() = default;
  virtual std::string GetValue() const { return value_; }
  virtual void SetDefaultValue() = 0;
  virtual void SetValue(const std::string&) = 0;
protected:
  std::string value_;
};

class ASCIIPen : public Pen {
public:
  ASCIIPen(const std::string& value) { SetValue(value); }
  void SetDefaultValue() override { value_ = " "; }
  void SetValue(const std::string& value) override {
    if (value.size() == 0) {
      SetDefaultValue();
    } else {
      value_.resize(1);
      value_[0] = value[0];
    }
  }
private:
};

class UTF16Pen : public Pen {
public:
  UTF16Pen(const std::string& value) { SetValue(value); }
  void SetDefaultValue() override { value_ = "\u0020"; }
  void SetValue(const std::string& value) override {
    if (value.size() < 2) {
      SetDefaultValue();
    } else {
      value_.resize(2);
      value_[0] = value[0];
      value_[1] = value[1];
    }
  }
};

class Plot {
public:
  Plot(std::string label = "", int height = kConsoleHeight, int width = kConsoleWidth) 
      : label_(label) {
    if (height < 0 || width < 0) {
      throw InvalidPlotSize();
    }
    if (height == kConsoleHeight || width == kConsoleWidth) {
      struct winsize w;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
      if (height == kConsoleHeight) height_ = w.ws_row;
      if (width == kConsoleWidth) width_ = w.ws_col;
    }
    canvas_.resize(height * width);

    xlim_left_ = 0.0;
    xlim_right_ = 1.0;
    ylim_bottom_ = 0.0;
    ylim_top_ = 1.0;
  }

  CellStatus& at(int col, int row) {
    return canvas_[col + height_*row];
  }

  const CellStatus& at(int col, int row) const {
    return canvas_[col + height_*row];
  }

  Plot& DrawPoint(double x, double y) {
    if (xlim_left_ < x && x < xlim_right_ &&
        ylim_bottom_ < y && y < ylim_top_) {
      const double xstep = (xlim_right_ - xlim_left_) / width_;
      const double ystep = (ylim_top_ - ylim_bottom_) / height_;
      at(static_cast<int>(x / xstep),
         static_cast<int>(y / ystep)) = CellStatus::Filled;
    }
    return *this;
  }

  Plot& DrawPoints(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size()) {
      throw InconsistentData();
    }
    const double xstep = (xlim_right_ - xlim_left_) / width_;
    const double ystep = (ylim_top_ - ylim_bottom_) / height_;
    size_t n = x.size();
    for (size_t i = 0; i < n; ++i) {
      if (xlim_left_ < x[i] && x[i] < xlim_right_ &&
          ylim_bottom_ < y[i] && y[i] < ylim_top_) {
        at(static_cast<int>(x[i] / xstep),
           static_cast<int>(y[i] / ystep)) = CellStatus::Filled;
      }
    }
    return *this;
  }

  Plot& FillAreaUnderCurve() {
    std::vector<int> stop_idx(width_, 0);
    for (int i = 0; i < width_; ++i) {
      for (int j = height_ - 1; j > 0; --j) {
        if (at(i, j) == CellStatus::Filled) {
          stop_idx[i] = j;
          break;
        }
      }
    }
    for (int i = 0; i < width_; ++i) {
      auto first = canvas_.begin() + i * height_;
      auto last = first + stop_idx[i];
      std::fill(first, last, CellStatus::Filled);
    }
    return *this;
  }

  Plot& DrawBorders(borders_t borders) {
    if (borders & Borders::Left) {
      auto first = canvas_.begin();
      auto last = first + height_;
      std::fill(first, last, CellStatus::Filled);
    }
    if (borders & Borders::Right) {
      auto first = canvas_.begin();
      first += height_ * (width_ - 1);
      auto last = first + height_;
      std::fill(first, last, CellStatus::Filled);
    }
    if (borders & Borders::Bottom) {
      for (int i = 0; i < width_; ++i) {
        at(i, 0) = CellStatus::Filled;
      }
    }
    if (borders & Borders::Top) {
      for (int i = 0; i < width_; ++i) {
        at(i, height_ - 1) = CellStatus::Filled;
      }
    }
    return *this;
  }

  // Getters

  std::string GetName() const { return name_; }
  std::string GetLabel() const { return label_; }
  double GetXlimLeft() const { return xlim_left_; }
  double GetXlimRight() const { return xlim_right_; }
  double GetYlimBottom() const { return ylim_bottom_; }
  double GetYlimTop() const { return ylim_top_; }

  Pen& GetPen() {
    if (pen_ == nullptr) {
      throw PenNotSet();
    }
    return *pen_;
  }

  const Pen& GetPen() const {
    if (pen_ == nullptr) {
      throw PenNotSet();
    }
    return *pen_;
  }

  // Setters

  Plot& SetName(std::string name) { name_ = name; return *this; }
  Plot& SetLabel(std::string label) { label_ = label; return *this; }
  Plot& SetPen(Pen *pen) { pen_ = pen; return *this; }

  Plot& SetXlimLeft(double new_xlim_left) {
    if (new_xlim_left < xlim_right_) {
      xlim_left_ = new_xlim_left;
    }
    return *this;
  }

  Plot& SetXlimRight(double new_xlim_right) {
    if (xlim_left_ < new_xlim_right) {
      xlim_right_ = new_xlim_right;
    }
    return *this;
  }

  Plot& SetYlimBottom(double new_ylim_bottom) {
    if (new_ylim_bottom < ylim_top_) {
      ylim_bottom_ = new_ylim_bottom;
    }
    return *this;
  }

  Plot& SetYlimTop(double new_ylim_top) {
    if (ylim_bottom_ < new_ylim_top) {
      ylim_top_ = new_ylim_top;
    }
    return *this;
  }

  Plot& SetXlimits(double new_xlim_left, double new_xlim_right) {
    if (new_xlim_left < new_xlim_right) {
      xlim_left_ = new_xlim_left;
      xlim_right_ = new_xlim_right;
    }
    return *this;
  }

  Plot& SetYlimits(double new_ylim_bottom, double new_ylim_top) {
    if (new_ylim_bottom < new_ylim_top) {
      ylim_bottom_ = new_ylim_bottom;
      ylim_top_ = new_ylim_top;
    }
    return *this;
  }
  
protected:
  std::string name_;
  std::string label_;
  int height_;
  int width_;
  std::vector<CellStatus> canvas_;
  Pen *pen_ = nullptr;
  double xlim_left_;
  double xlim_right_;
  double ylim_bottom_;
  double ylim_top_;
private:
};


class HistPlot : public Plot {
public:
  HistPlot() = default;
};


} // namespace askiplot

#endif // ASKIPLOT_HPP_