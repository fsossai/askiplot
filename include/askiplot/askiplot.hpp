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
#include <sstream>

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

enum class CellStatus { Line, Empty, Area };

enum Borders : char {
  Left = 0x01, Right = 0x02, Bottom = 0x04, Top = 0x08
};

class InvalidPlotSize : public std::exception {
public:
  virtual const char* what() const noexcept override {
    return "Plot height and width must be positive integer numbers.";
  }
};

class InconsistentData : public std::exception {
public:
  virtual const char* what() const noexcept override {
    return "Data to be drawn is inconsistent.";
  }
};

class InvalidCellOrPenValue : public std::exception {
public:
  virtual const char* what() const noexcept override {
    return
      "The value of a Cell or a Pen must be a single ASCII character or a single UTF16 char "
      "and cannot be 0x00 (string termination character).";
  }
};

class Pen {
public:
  Pen() {
    SetLine(kDefaultPenLine);
    SetEmpty(kDefaultPenEmpty);
    SetArea(kDefaultPenArea);
  }

  Pen(const std::string& line,
      const std::string& empty = kDefaultPenEmpty,
      const std::string& area = kDefaultPenArea) {
    SetLine(line);
    SetEmpty(empty);
    SetArea(area);
  }
  
  Pen(const char *line,
      const char *empty = kDefaultPenEmpty.data(),
      const char *area = kDefaultPenArea.data()) {
    SetLine(line);
    SetEmpty(empty);
    SetArea(area);
  }

  Pen(const Pen&) = default;
  Pen(Pen&&) = default;
  Pen& operator=(const Pen&) = default;
  Pen& operator=(Pen&&) = default;
  ~Pen() = default;
  
  // Getters

  std::string GetLine() const { return line_; }
  std::string GetEmpty() const { return empty_; }
  std::string GetArea() const { return area_; }

  // Setters

  Pen& SetLine(const std::string& line) {
    line_ = CheckAndReformat(line.size() == 0 ? kDefaultPenLine : line);
    return *this;
  }

  Pen& SetEmpty(const std::string& empty) {
    empty_ = CheckAndReformat(empty.size() == 0 ? kDefaultPenEmpty : empty);
    return *this;
  }

  Pen& SetArea(const std::string& area) {
    area_ = CheckAndReformat(area.size() == 0 ? kDefaultPenArea : area);
    return *this;
  }

private:
  std::string CheckAndReformat(const std::string& candidate) const {
    std::string result("xx");
    if (candidate.size() == 0) {
      throw InvalidCellOrPenValue();
    } else if (std::isprint(static_cast<unsigned char>(candidate[0]))) {
      result[0] = candidate[0];
      result[1] = '\0';
    } else if (candidate.size() == 1) {
      throw InvalidCellOrPenValue();
    } else {
      result[0] = candidate[0];
      result[1] = candidate[1];
    }
    return result;
  }

  std::string line_;
  std::string empty_;
  std::string area_;
};

class Cell {
public:
  Cell() {
    SetValue(kDefaultPenEmpty, CellStatus::Empty);
  }

  Cell(const Cell&) = default;
  Cell(Cell&&) = default;
  Cell& operator=(const Cell&) = default;
  Cell& operator=(Cell&&) = default;
  ~Cell() = default;

  std::string GetValue() const {
    if (value_[1] == '\0') {
      return std::string(value_, 1);
    }
    return std::string(value_, 2);
  }

  Cell& SetValue(const Pen& pen, const CellStatus& status) {
    status_ = status;
    switch (status) { 
    case CellStatus::Line:
      SetValueRaw(pen.GetLine());
      break;
    case CellStatus::Empty:
      SetValueRaw(pen.GetEmpty());
      break;
    case CellStatus::Area:
      SetValueRaw(pen.GetArea());
      break;
    }
    return *this;
  }

  Cell& SetValue(const Pen& pen) {
    SetValue(pen, status_);
    return *this;
  }

  const CellStatus& GetStatus() const { return status_; }

  bool IsLine() const { return status_ == CellStatus::Line; }
  bool IsEmpty() const { return status_ == CellStatus::Empty; }
  bool IsArea() const { return status_ == CellStatus::Area; }

private:
  void SetValueRaw(const std::string& value) {
    value_[0] = value[0];
    value_[1] = value[1];
  }

  char value_[2];
  CellStatus status_;
};

class __PlotDummySubtype;

template<class Subtype = __PlotDummySubtype>
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
      if (height == kConsoleHeight) height_ = w.ws_row - 1;
      if (width == kConsoleWidth) width_ = w.ws_col;
    }
    canvas_.resize(height_ * width_);

    xlim_left_ = 0.0;
    xlim_right_ = 1.0;
    ylim_bottom_ = 0.0;
    ylim_top_ = 1.0;
  }

  Cell& at(int col, int row) {
    return canvas_[row + height_*col];
  }

  const Cell& at(int col, int row) const {
    return canvas_[row + height_*col];
  }

  Subtype& DrawPoint(double x, double y) {
    if (xlim_left_ < x && x < xlim_right_ &&
        ylim_bottom_ < y && y < ylim_top_) {
      const double xstep = (xlim_right_ - xlim_left_) / width_;
      const double ystep = (ylim_top_ - ylim_bottom_) / height_;
      at(static_cast<int>(x / xstep),
         static_cast<int>(y / ystep))
        .SetValue(pen_, CellStatus::Line);
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawPoints(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size()) {
      throw InconsistentData();
    }
    const double xstep = (xlim_right_ - xlim_left_) / width_;
    const double ystep = (ylim_top_ - ylim_bottom_) / height_;
    const auto cell_line = Cell{}.SetValue(pen_, CellStatus::Line);
    size_t n = x.size();
    for (size_t i = 0; i < n; ++i) {
      if (xlim_left_ < x[i] && x[i] < xlim_right_ &&
          ylim_bottom_ < y[i] && y[i] < ylim_top_) {
        at(static_cast<int>(x[i] / xstep),
           static_cast<int>(y[i] / ystep)) = cell_line;
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawBorders(borders_t borders) {
    const auto cell_line = Cell{}.SetValue(pen_, CellStatus::Line);
    if (borders & Borders::Left) {
      auto first = canvas_.begin();
      auto last = first + height_;
      std::fill(first, last, cell_line);
    }
    if (borders & Borders::Right) {
      auto first = canvas_.begin();
      first += height_ * (width_ - 1);
      auto last = first + height_;
      std::fill(first, last, cell_line);
    }
    if (borders & Borders::Bottom) {
      for (int i = 0; i < width_; ++i) {
        at(i, 0) = cell_line;
      }
    }
    if (borders & Borders::Top) {
      for (int i = 0; i < width_; ++i) {
        at(i, height_ - 1) = cell_line;
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& FillAreaUnderCurve() {
    std::vector<int> stop_idx(width_, 0);
    for (int i = 0; i < width_; ++i) {
      for (int j = height_ - 1; j > 0; --j) {
        if (!at(i, j).IsEmpty()) {
          stop_idx[i] = j;
          break;
        }
      }
    }

    const auto cell_line = Cell{}.SetValue(pen_, CellStatus::Line);
    for (int i = 0; i < width_; ++i) {
      auto first = canvas_.begin() + i * height_;
      auto last = first + stop_idx[i];
      std::fill(first, last, cell_line);
    }
    return *this;
  }

  std::string Serialize() const {
    std::stringstream ss("");
    for (int j = height_ - 1; j >= 0; --j) {
      for (int i = 0; i < width_; ++i) {
        ss << at(i, j).GetValue();
      }
      ss << "\n";
    }
    return ss.str();
  }

  // Getters

  std::string GetName() const { return name_; }
  std::string GetLabel() const { return label_; }
  int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }
  double GetXlimLeft() const { return xlim_left_; }
  double GetXlimRight() const { return xlim_right_; }
  double GetYlimBottom() const { return ylim_bottom_; }
  double GetYlimTop() const { return ylim_top_; }
  Pen& GetPen() { return pen_; }
  const Pen& GetPen() const { return pen_; }

  // Setters

  Subtype& SetName(std::string name) {
    name_ = name;
    return static_cast<Subtype&>(*this);
  }

  Subtype &SetLabel(std::string label) {
    label_ = label;
    return static_cast<Subtype&>(*this);
  }

  Subtype &SetPen(const Pen &pen) {
    pen_ = pen;
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetXlimLeft(double new_xlim_left) {
    if (new_xlim_left < xlim_right_) {
      xlim_left_ = new_xlim_left;
    }
    return *this;
  }

  Subtype& SetXlimRight(double new_xlim_right) {
    if (xlim_left_ < new_xlim_right) {
      xlim_right_ = new_xlim_right;
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetYlimBottom(double new_ylim_bottom) {
    if (new_ylim_bottom < ylim_top_) {
      ylim_bottom_ = new_ylim_bottom;
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetYlimTop(double new_ylim_top) {
    if (ylim_bottom_ < new_ylim_top) {
      ylim_top_ = new_ylim_top;
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetXlimits(double new_xlim_left, double new_xlim_right) {
    if (new_xlim_left < new_xlim_right) {
      xlim_left_ = new_xlim_left;
      xlim_right_ = new_xlim_right;
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetYlimits(double new_ylim_bottom, double new_ylim_top) {
    if (new_ylim_bottom < new_ylim_top) {
      ylim_bottom_ = new_ylim_bottom;
      ylim_top_ = new_ylim_top;
    }
    return static_cast<Subtype&>(*this);
  }
  
protected:
  std::string name_;
  std::string label_;
  int height_;
  int width_;
  std::vector<Cell> canvas_;
  Pen pen_;
  double xlim_left_;
  double xlim_right_;
  double ylim_bottom_;
  double ylim_top_;
private:
};

class __PlotDummySubtype : public Plot<__PlotDummySubtype> { };

class HistPlot : public Plot<HistPlot> {
public:
  HistPlot() = default;
};


} // namespace askiplot

#endif // ASKIPLOT_HPP_