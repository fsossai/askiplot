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

#include <algorithm>
#include <iostream>
#include <limits>
//#include <numeric>
#include <sstream>
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

enum class CellStatus : uint16_t { Line, Empty, Area };

enum Borders : char {
  None = 0x00, Left = 0x01, Right = 0x02, Bottom = 0x04, Top = 0x08, All = 0x0F
};

Borders operator+(const Borders& a, const Borders& b) {
  return static_cast<Borders>(static_cast<char>(a) | static_cast<char>(b));
}

Borders operator-(const Borders& a, const Borders& b) {
  return static_cast<Borders>(static_cast<char>(a) & (~static_cast<char>(b)));
}

Borders operator&(const Borders& a, const Borders& b) {
  return static_cast<Borders>(static_cast<char>(a) & static_cast<char>(b));
}

Borders operator|(const Borders& a, const Borders& b) {
  return a + b;
}

enum Position : char {
  North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest, Center
};

double operator ""_percent(long double p) {
  return p / 100.0;
}

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
    std::string result;
    if (candidate.size() == 0) {
      throw InvalidCellOrPenValue();
    } else if (std::isprint(static_cast<unsigned char>(candidate[0]))) {
      result.resize(1);
      result[0] = candidate[0];
    } else if (candidate.size() == 1) {
      throw InvalidCellOrPenValue();
    } else {
      result.resize(2);
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

struct PlotMetadata {
  PlotMetadata& SetLabel(const std::string& l) { label = l; return *this; }
  PlotMetadata& SetLength(std::size_t l) { length = l; return *this; }
  PlotMetadata& SetPen(const Pen& p) { pen = p; return *this; }
  
  Pen pen;
  std::size_t length = 0;
  std::string label = "";
};

template<class Subtype>
class __IPlot {
public:
  __IPlot(std::string title = "", int height = kConsoleHeight, int width = kConsoleWidth) 
      : title_(title) {
    if (height < 0 || width < 0) {
      throw InvalidPlotSize();
    }
    if (height == kConsoleHeight || width == kConsoleWidth) {
      struct winsize w;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
      if (height == kConsoleHeight) height_ = w.ws_row - 1;
      if (width == kConsoleWidth) width_ = w.ws_col;
    }
    autolimit_ = Borders::All;
    xlim_margin_ = 0.05;
    xlim_left_ = 0.0;
    xlim_right_ = 1.0;
    ylim_margin_ = 0.020;
    ylim_bottom_ = 0.0;
    ylim_top_ = 1.0;
    canvas_.resize(height_ * width_);
  }

  virtual ~__IPlot() = default;

  virtual Cell& at(int col, int row) {
    return canvas_[row + height_*col];
  }

  virtual const Cell& at(int col, int row) const {
    return canvas_[row + height_*col];
  }

  Subtype& AutoLimit(Borders borders) {
    autolimit_ = borders;
    return static_cast<Subtype&>(*this);
  }

  Subtype& Clear() {
    auto cell_empty = Cell{}.SetValue(pen_, CellStatus::Empty);
    std::fill(canvas_.begin(), canvas_.end(), cell_empty);
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawBorders(Borders borders) {
    const auto cell_line = Cell{}.SetValue(pen_, CellStatus::Line);
    if (borders & Borders::Left) {
      for (int j = 0; j < height_; ++j) {
        at(0, j) = cell_line;
      }
    }
    if (borders & Borders::Right) {
      for (int j = 0; j < height_; ++j) {
        at(width_ - 1, j) = cell_line;
      }
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

  Subtype& DrawLegend(Position position = Position::NorthEast) {
    if (metadata_.size() == 0) return static_cast<Subtype&>(*this);

    int text_width =
      std::max_element(metadata_.begin(), metadata_.end(),
                       [](const auto& a, const auto& b)
                       { return a.label.size() < b.label.size(); }
      )->label.size();
    const int box_width = text_width + 6;
    const int box_height = metadata_.size() + 2;

    auto locs = CalcBoxLocation(position, box_width, box_height);
    const int loc_x = locs.first;
    const int loc_y = locs.second;

    // Setting up Cells
    auto cell_bottom = Cell{}.SetValue(Pen("_"), CellStatus::Line);
    auto cell_top = Cell{}.SetValue(Pen("_"), CellStatus::Line);
    auto cell_left = Cell{}.SetValue(Pen("|"), CellStatus::Line);
    auto cell_right = Cell{}.SetValue(Pen("|"), CellStatus::Line);

    // Drawing box borders
    for (int i = loc_x; i < loc_x + box_width; ++i) {
      at(i, loc_y) = cell_bottom;                 // Bottom
      at(i, loc_y + box_height - 1) = cell_top;   // Top
    }
    for (int j = loc_y; j < loc_y + box_height - 1; ++j) {
      at(loc_x, j) = cell_left;                    // Left
      at(loc_x + box_width - 1, j) = cell_right;   // Right
    }

    // Writing labels
    for (std::size_t i = 0; i < metadata_.size(); ++i) {
      DrawText(metadata_[i].pen.GetLine() + " " + metadata_[i].label,
               loc_x + 2, loc_y + box_height - 2 - i
      );
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawLineHorizontalAtRow(int row) {
    if (row < height_) {
      auto cell_line = Cell{}.SetValue(pen_, CellStatus::Line);
      for (int i = 0; i < width_; ++i) {
        at(i, row) = cell_line;
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawLineHorizontalAtRow(double height_ratio) {
    int row = height_ * std::max(0.0, std::min(1.0, height_ratio));
    return DrawLineHorizontalAtRow(row);
  }

  Subtype& DrawLineVerticalAtCol(int col) {
    if (col < width_) {
      auto cell_line = Cell{}.SetValue(pen_, CellStatus::Line);
      for (int j = 0; j < height_; ++j) {
        at(col, j) = cell_line;
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawLineVerticalAtCol(double width_ratio) {
    int col = width_ * std::max(0.0, std::min(1.0, width_ratio));
    return DrawLineVerticalAtCol(col);
  }

  Subtype& DrawLineHorizontalAtY(double y) {
    if (ylim_bottom_ < y && y < ylim_top_) {
      int row = (y - ylim_bottom_) / ((ylim_top_ - ylim_bottom_) / height_);
      return DrawLineHorizontalAtRow(row);
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawLineVerticallAtX(double x) {
    if (xlim_left_ < x && x < xlim_right_) {
      int col = (x - xlim_left_) / ((xlim_right_ - xlim_left_) / width_);
      return DrawLineVerticalAtCol(col);
    }
    return static_cast<Subtype&>(*this);
  }

  template<class Tx, class Ty>
  Subtype& DrawPoint(Tx x, Ty y) {
    if (xlim_left_   < x && x < xlim_right_ &&
        ylim_bottom_ < y && y < ylim_top_) {
      const double xstep = (xlim_right_ - xlim_left_) / width_;
      const double ystep = (ylim_top_ - ylim_bottom_) / height_;
      at(static_cast<int>((x - xlim_left_  ) / xstep),
         static_cast<int>((y - ylim_bottom_) / ystep))
        .SetValue(pen_, CellStatus::Line);
    }
    return static_cast<Subtype&>(*this);
  }

  template<class Tx, class Ty>
  Subtype& DrawPoints(const std::vector<Tx>& x,
                      const std::vector<Ty>& y,
                      std::size_t how_many) {
    SetAutoLimits(x, y);
    
    const double xstep = (xlim_right_ - xlim_left_) / width_;
    const double ystep = (ylim_top_ - ylim_bottom_) / height_;
    
    const std::size_t n = std::min({x.size(), y.size(), how_many});
    const auto cell_line = Cell{}.SetValue(pen_, CellStatus::Line);

    for (std::size_t i = 0; i < n; ++i) {
      if (xlim_left_   < x[i] && x[i] < xlim_right_ &&
          ylim_bottom_ < y[i] && y[i] < ylim_top_) {
        at(static_cast<int>((x[i] - xlim_left_  ) / xstep),
           static_cast<int>((y[i] - ylim_bottom_) / ystep)) = cell_line;
      }
    }
    return static_cast<Subtype&>(*this);
  }

  template<class Tx, class Ty>
  Subtype& DrawPoints(const std::vector<Tx>& x,
                      const std::vector<Ty>& y) {
    return DrawPoints(x, y, std::numeric_limits<std::size_t>::max());
  }

  Subtype& DrawText(const std::string& text, int col, int row) {
    int n = std::min(width_ - col, static_cast<int>(text.size()));
    for (int i = 0; i < n; ++i) {
      at(i + col, row).SetValue(Pen(&text[i]), CellStatus::Line);
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawText(const std::string& text, Position position) {
    auto locs = CalcBoxLocation(position, text.size(), 1);
    return DrawText(text, locs.first, locs.second);
  }

  Subtype& DrawTitle() {
    return DrawText(title_, width_ / 2 - title_.size() / 2, height_ - 1);
  }

  Subtype& Fill(const Pen& pen) {
    auto cell_new = Cell{}.SetValue(pen, CellStatus::Line);
    std::fill(canvas_.begin(), canvas_.end(), cell_new);
    return static_cast<Subtype&>(*this);
  }

  Subtype& Fill() {
    Fill(pen_);
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
    return static_cast<Subtype&>(*this);
  }

  template<class Tx, class Ty>
  Subtype& PlotData(const std::vector<Tx>& x,
                    const std::vector<Ty>& y,
                    const std::string& label,
                    std::size_t how_many) {
    DrawPoints(x, y, how_many);
    metadata_.push_back(
      PlotMetadata{}.SetLabel(label)
                    .SetLength(how_many)
                    .SetPen(pen_)
    );
    return static_cast<Subtype&>(*this);
  }

  template<class Tx, class Ty>
  Subtype& PlotData(const std::vector<Tx>& x,
                    const std::vector<Ty>& y,
                    const std::string& label) {
    return PlotData(x, y, label, std::numeric_limits<std::size_t>::max());
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
  std::string GetTitle() const { return title_; }
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

  Subtype& SetTitle(std::string title) {
    title_ = title;
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetPen(const Pen &pen) {
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
  std::string title_;
  int height_;
  int width_;
  Pen pen_;
  Borders autolimit_;
  double xlim_margin_;
  double xlim_left_;
  double xlim_right_;
  double ylim_margin_;
  double ylim_bottom_;
  double ylim_top_;
  std::vector<Cell> canvas_;
  std::vector<PlotMetadata> metadata_;
  
private:
  std::pair<int, int> CalcBoxLocation(Position pos, int box_width, int box_height) {
    switch (pos) {
    case Position::North:
      return std::pair<int, int>(width_ / 2 - box_width / 2, height_ - box_height - 1);
    default:
    case Position::NorthEast:
      return std::pair<int, int>(width_ - box_width - 2, height_ - box_height - 1);
    case Position::East:
      return std::pair<int, int>(width_ - box_width - 2, height_ / 2 - box_height / 2);
    case Position::SouthEast:
      return std::pair<int, int>(width_ - box_width - 2, 1);
    case Position::South:
      return std::pair<int, int>(width_ / 2 - box_width / 2, 1);
    case Position::SouthWest:
      return std::pair<int, int>(2, 1);
    case Position::West:
      return std::pair<int, int>(2, height_ / 2 - box_height / 2);
    case Position::NorthWest:
      return std::pair<int, int>(2, height_ - box_height - 1);
    case Position::Center:
      return std::pair<int, int>(width_  / 2 - box_width  / 2,
                                 height_ / 2 - box_height / 2
      );
    }
  }

  template<class Tx, class Ty>
  void SetAutoLimits(const std::vector<Tx>& x,
                     const std::vector<Ty>& y) {
    // Left and Right
    auto x_margin_surplus = [this](){ return std::abs((xlim_right_ - xlim_left_) * xlim_margin_); };
    auto y_margin_surplus = [this](){ return std::abs((ylim_top_ - ylim_bottom_) * ylim_margin_); };
    
    if (autolimit_ & Borders::Left) {
      if (autolimit_ & Borders::Right) {
        auto mm = std::minmax_element(x.begin(), x.end());
        xlim_left_ = *mm.first;
        xlim_right_ = *mm.second;
        double ms = x_margin_surplus();
        xlim_left_ -= ms;
        xlim_right_ += ms;
      } else {
        xlim_left_ = *std::min_element(x.begin(), x.end());
        xlim_left_ -= x_margin_surplus();
      }
    } else if (autolimit_ & Borders::Right) {
      xlim_right_ = *std::max_element(x.begin(), x.end());
      xlim_right_ += x_margin_surplus();
    }

    // Bottom and Top
    if (autolimit_ & Borders::Bottom) {
      if (autolimit_ & Borders::Top) {
        auto mm = std::minmax_element(y.begin(), y.end());
        ylim_bottom_ = *(mm.first);
        ylim_top_ = *(mm.second);
        double ms = y_margin_surplus();
        ylim_bottom_ -= ms;
        ylim_top_ += ms;
      } else {
        ylim_bottom_ = *std::min_element(y.begin(), y.end());
        ylim_bottom_ += y_margin_surplus();
      }
    } else if (autolimit_ & Borders::Top) {
      ylim_top_ = *std::max_element(y.begin(), y.end());
      ylim_top_ += y_margin_surplus();
    }
  }
};

template<class Subtype>
class __IHistPlot : public __IPlot<Subtype> {
public:
};

class Plot final : public __IPlot<Plot> { };
class HistPlot final : public __IHistPlot<HistPlot> { };

} // namespace askiplot

#endif // ASKIPLOT_HPP_