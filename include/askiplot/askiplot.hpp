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
#include <sstream>
#include <string>
#include <vector>

#define ASKIPLOT_VERSION_MAJOR 0
#define ASKIPLOT_VERSION_MINOR 1
#define ASKIPLOT_VERSION_PATCH 0

namespace askiplot {

//******************************** Constants ********************************//

const std::string kDefaultPenLine = "=";
const std::string kDefaultPenArea = "#";
const std::string kDefaultPenEmpty = " ";
const int kConsoleHeight = 0;
const int kConsoleWidth = 0;

//******************************* Exceptions ********************************//

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

//******************************* Enumerators *******************************//

enum class CellStatus : uint16_t { Line, Empty, Area };

enum Borders : char {
  None = 0x00, Left = 0x01, Right = 0x02, Bottom = 0x04, Top = 0x08, All = 0x0F
};

enum RelativePosition : char {
  North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest, Center
};

enum AdjustPosition : bool {
  Adjust = true, DontAdjust = false
};

//**************************** Offset & Position ****************************//

class Offset {
public:
  Offset()
      : col_(0)
      , row_(0) { }

  Offset(int col, int row)
      : col_(col)
      , row_(row) { }

  Offset(const std::pair<int, int>& cr_pair)
      : col_(cr_pair.first)
      , row_(cr_pair.second) { }

  int GetCol() const { return col_; }
  int GetRow() const { return row_; }
  Offset& SetCol(int col) { col_ = col; return *this; }
  Offset& SetRow(int row) { row_ = row; return *this; }

private:
  int col_, row_;
};

struct Position {
  Position(int col, int row, const RelativePosition& relative = SouthWest)
      : offset(col, row)
      , relative(relative) { }
  
  Position(const Offset& offset, const RelativePosition& relative = SouthWest)
      : offset(offset)
      , relative(relative) { }

  Position(const RelativePosition& relative = SouthWest)
      : offset(0, 0)
      , relative(relative) { }

  bool IsAbsolute() const { return relative == SouthWest; }

  Offset offset;
  RelativePosition relative;
};

//******************************** Operators ********************************//

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

Offset operator+(const Offset& a, const Offset& b) {
  return Offset(a.GetCol() + b.GetCol(), a.GetRow() + b.GetRow());
}

Offset operator-(const Offset& a, const Offset& b) {
  return Offset(a.GetCol() - b.GetCol(), a.GetRow() - b.GetRow());
}

Position operator+(const RelativePosition& relative, const Offset& offset) {
  return Position(offset, relative);
}

Position operator+(const Offset& offset, const RelativePosition& relative) {
  return Position(offset, relative);
}

Position operator+(const Position& position, const Offset& offset) {
  return Position(position.offset + offset, position.relative);
}

Position operator-(const Position& position, const Offset& offset) {
  return Position(position.offset - offset, position.relative);
}

double operator ""_percent(long double p) {
  return p / 100.0;
}

//*********************************** Pen ***********************************//

class PenStyle {
public:
  PenStyle() {
    SetLineStyle(kDefaultPenLine);
    SetEmptyStyle(kDefaultPenEmpty);
    SetAreaStyle(kDefaultPenArea);
  }

  PenStyle(const std::string& line) : PenStyle() {
    SetLineStyle(line);
  }
  
  PenStyle(const char *line) : PenStyle() {
    SetLineStyle(line);
  }

  PenStyle(char line) : PenStyle() {
    SetLineStyle(std::string(1, line));
  }

  PenStyle(const PenStyle&) = default;
  PenStyle(PenStyle&&) = default;
  PenStyle& operator=(const PenStyle&) = default;
  PenStyle& operator=(PenStyle&&) = default;
  ~PenStyle() = default;
  
  // Getters

  std::string GetLineStyle() const { return line_; }
  std::string GetEmptyStyle() const { return empty_; }
  std::string GetAreaStyle() const { return area_; }

  // Setters

  PenStyle& SetLineStyle(const std::string& line) {
    line_ = CheckAndReformat(line.size() == 0 ? kDefaultPenLine : line);
    return *this;
  }

  PenStyle& SetLineStyle(char line) {
    return SetLineStyle(std::string(1, line));
  }

  PenStyle& SetEmptyStyle(const std::string& empty) {
    empty_ = CheckAndReformat(empty.size() == 0 ? kDefaultPenEmpty : empty);
    return *this;
  }

  PenStyle& SetEmptyStyle(char empty) {
    return SetEmptyStyle(std::string(1, empty));
  }

  PenStyle& SetAreaStyle(const std::string& area) {
    area_ = CheckAndReformat(area.size() == 0 ? kDefaultPenArea : area);
    return *this;
  }

  PenStyle& SetAreaStyle(char area) {
    return SetAreaStyle(std::string(1, area));
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

//********************************** Cell ***********************************//

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

  Cell& SetValue(const PenStyle& pen, const CellStatus& status) {
    status_ = status;
    switch (status) { 
    case CellStatus::Line:
      SetValueRaw(pen.GetLineStyle());
      break;
    case CellStatus::Empty:
      SetValueRaw(pen.GetEmptyStyle());
      break;
    case CellStatus::Area:
      SetValueRaw(pen.GetAreaStyle());
      break;
    }
    return *this;
  }

  Cell& SetValue(const PenStyle& pen) {
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

//****************************** PlotMetaData *******************************//

struct PlotMetadata {
  PlotMetadata& SetLabel(const std::string& l) { label = l; return *this; }
  PlotMetadata& SetLength(std::size_t l) { length = l; return *this; }
  PlotMetadata& SetPenStyle(const PenStyle& p) { penstyle = p; return *this; }
  
  PenStyle penstyle;
  std::size_t length = 0;
  std::string label = "";
};

class __IPlot {
public:
  virtual Cell& at(int col, int row) = 0;
  virtual const Cell& at(int col, int row) const = 0;
  virtual std::string Serialize() const = 0;
protected:
  int height_;
  int width_;
  std::vector<Cell> canvas_;
};

//********************************** Plot ***********************************//

template<class Subtype>
class __Plot : public __IPlot {
public:
  __Plot(int width = kConsoleWidth, int height = kConsoleHeight) {
    if (width < 0 || height < 0) {
      throw InvalidPlotSize();
    }
    if (width == kConsoleWidth || height == kConsoleHeight) {
      struct winsize w;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
      if (width == kConsoleWidth) width_ = w.ws_col;
      if (height == kConsoleHeight) height_ = w.ws_row - 1;
    } else {
      width_ = width;
      height_ = height;
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

  virtual ~__Plot() = default;

  virtual Cell& at(int col, int row) override {
    return canvas_[row + height_*col];
  }

  virtual const Cell& at(int col, int row) const override {
    return canvas_[row + height_*col];
  }

  Subtype& AutoLimit(Borders borders) {
    autolimit_ = borders;
    return static_cast<Subtype&>(*this);
  }

  Subtype& Clear() {
    auto cell_empty = Cell{}.SetValue(penstyle_, CellStatus::Empty);
    std::fill(canvas_.begin(), canvas_.end(), cell_empty);
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawBorders(Borders borders) {
    const auto cell_line = Cell{}.SetValue(penstyle_, CellStatus::Line);
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

  Subtype& DrawLegend(const Position& position = {}) {
    if (metadata_.size() == 0) return static_cast<Subtype&>(*this);

    int text_width =
      std::max_element(metadata_.begin(), metadata_.end(),
                       [](const auto& a, const auto& b)
                       { return a.label.size() < b.label.size(); }
      )->label.size();
    const int box_width = text_width + 6;
    const int box_height = metadata_.size() + 2;

    auto box_rel_pos = CalcBoxPosition(position, box_width, box_height);
    auto box_abs_pos = GetAbsolutePosition(box_rel_pos);
    const int pos_col = box_abs_pos.offset.GetCol();
    const int pos_row = box_abs_pos.offset.GetRow();

    // Setting up Cells
    auto cell_bottom = Cell{}.SetValue(PenStyle("_"), CellStatus::Line);
    auto cell_top = Cell{}.SetValue(PenStyle("_"), CellStatus::Line);
    auto cell_left = Cell{}.SetValue(PenStyle("|"), CellStatus::Line);
    auto cell_right = Cell{}.SetValue(PenStyle("|"), CellStatus::Line);

    // Drawing box borders
    for (int i = pos_col; i < pos_col + box_width; ++i) {
      at(i, pos_row) = cell_bottom;                 // Bottom
      at(i, pos_row + box_height - 1) = cell_top;   // Top
    }
    for (int j = pos_row; j < pos_row + box_height - 1; ++j) {
      at(pos_col, j) = cell_left;                    // Left
      at(pos_col + box_width - 1, j) = cell_right;   // Right
    }

    // Writing labels
    for (std::size_t i = 0; i < metadata_.size(); ++i) {
      DrawText(metadata_[i].penstyle.GetLineStyle() + " " + metadata_[i].label,
               Position(pos_col + 2, pos_row + box_height - 2 - i)
      );
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawLine(double x_begin, double y_begin, double x_end, double y_end) {
    auto cell_line = Cell{}.SetValue(penstyle_, CellStatus::Line);

    const double xstep = (xlim_right_ - xlim_left_) / width_;
    const double ystep = (ylim_top_ - ylim_bottom_) / height_;
   
    auto to_col = [=](double x) -> int { return (x - xlim_left_  ) / xstep; };
    auto to_row = [=](double y) -> int { return (y - ylim_bottom_) / ystep; };
   
    const int col_beg = to_col(x_begin);
    const int row_beg = to_row(y_begin);
    const int col_end = to_col(x_end);
    const int row_end = to_row(y_end);
   
    const int delta_col = col_end - col_beg;
    const int delta_row = row_end - row_beg;
    const int n = std::max(std::abs(delta_col), std::abs(delta_row)) + 1;
    
    if (std::abs(delta_col) < std::abs(delta_row)) {
      const double x_adv = (x_end - x_begin) / n;
      const int j_adv = (row_beg < row_end) ? +1 : -1;
      double x_current = x_begin;
      for (int j = 0; std::abs(j) < n; j += j_adv) {
        at(to_col(x_current), row_beg + j) = cell_line;
        x_current += x_adv;
      }
    } else {
      const double y_adv = (y_end - y_begin) / n;
      const int i_adv = (col_beg < col_end) ? +1 : -1;
      double y_current = y_begin;
      for (int i = 0; std::abs(i) < n; i += i_adv) {
        at(col_beg + i, to_row(y_current)) = cell_line;
        y_current += y_adv;
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawLine(const Position& begin, const Position& end) {
    const double xstep = (xlim_right_ - xlim_left_) / width_;
    const double ystep = (ylim_top_ - ylim_bottom_) / height_;
    
    auto to_x = [=](int col) -> double { return col * xstep + xlim_left_; };
    auto to_y = [=](int row) -> double { return row * ystep + ylim_bottom_; };

    auto pos_beg_abs = GetAbsolutePosition(begin);
    auto pos_end_abs = GetAbsolutePosition(end);

    DrawLine(to_x(pos_beg_abs.offset.GetCol()), to_y(pos_beg_abs.offset.GetRow()),
             to_x(pos_end_abs.offset.GetCol()), to_y(pos_end_abs.offset.GetRow()));
    
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawLineHorizontalAtRow(int row) {
    if (row < height_) {
      auto cell_line = Cell{}.SetValue(penstyle_, CellStatus::Line);
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
      auto cell_line = Cell{}.SetValue(penstyle_, CellStatus::Line);
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
        .SetValue(penstyle_, CellStatus::Line);
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
    const auto cell_line = Cell{}.SetValue(penstyle_, CellStatus::Line);

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

  Subtype& DrawText(const std::string& text,
                    const Position& position,
                    AdjustPosition adjust = Adjust) {
    auto pos_abs = GetAbsolutePosition(position);

    if (adjust) {
      AdjustAbsolutePosition(pos_abs, text.size(), 1);
    }

    const int col = pos_abs.offset.GetCol();
    const int row = pos_abs.offset.GetRow();

    if (0 <= row && row < height_) {
      int n = std::min<int>(width_ - col, text.size());
      for (int i = 0; i < n; ++i) {
        at(i + col, row).SetValue(PenStyle(&text[i]), CellStatus::Line);
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawTextCentered(const std::string& text,
                            const Position& position,
                            AdjustPosition adjust = Adjust) {
    return DrawText(text, position - Offset(text.size() / 2, 0), adjust);
  }

  Subtype& DrawTextVertical(const std::string& text,
                            const Position& position,
                            AdjustPosition adjust = Adjust) {
    auto pos_abs = GetAbsolutePosition(position);

    if (adjust) {
      AdjustAbsolutePosition(pos_abs, 1, text.size());
    }

    const int col = pos_abs.offset.GetCol();
    const int row = pos_abs.offset.GetRow();

    if (0 <= col && col < width_) {
      int n = std::min<int>(row + 1, text.size());
      for (int j = 0; j < n; ++j) {
        at(col, row - j).SetValue(PenStyle(&text[j]), CellStatus::Line);
      }
    } else { std::cout << col << "\t" << row << std::endl; }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawTitle() {
    return DrawTextCentered(title_, North, DontAdjust);
  }

  Subtype& Fill(const PenStyle& pen) {
    auto cell_new = Cell{}.SetValue(pen, CellStatus::Line);
    std::fill(canvas_.begin(), canvas_.end(), cell_new);
    return static_cast<Subtype&>(*this);
  }

  Subtype& Fill() {
    return Fill(penstyle_);
  }

  Subtype& Move(const Offset& offset) {
    std::vector<Cell> new_canvas(canvas_.size());
    std::fill(new_canvas.begin(), new_canvas.end(),
              Cell{}.SetValue(penstyle_.GetEmptyStyle()));

    const int off_c = offset.GetCol();
    const int off_r = offset.GetRow();
    auto dest_idx = [=](int col, int row) {
      return (row + off_r) + height_ * (col + off_c);
    };

    const int col_beg = std::max(0, -off_c);
    const int col_end = std::min(width_, width_ - off_c);
    const int row_beg = std::max(0, -off_r);
    const int row_end = std::min(height_, height_ - off_r);

    for (int i = col_beg; i < col_end; ++i) {
      for (int j = row_beg; j < row_end; ++j) {
        new_canvas[dest_idx(i, j)] = at(i,j);
      }
    }
    canvas_ = move(new_canvas);
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
                    .SetPenStyle(penstyle_)
    );
    return static_cast<Subtype&>(*this);
  }

  template<class Tx, class Ty>
  Subtype& PlotData(const std::vector<Tx>& x,
                    const std::vector<Ty>& y,
                    const std::string& label) {
    return PlotData(x, y, label, std::numeric_limits<std::size_t>::max());
  }

  std::string Serialize() const override {
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

  Position GetAbsolutePosition(const Position& position) const {
    switch (position.relative) {
    case North:
      return Position(position.offset + Offset(width_ / 2, height_ - 1), SouthWest);
    case NorthEast:
      return Position(position.offset + Offset(width_ - 1, height_ - 1), SouthWest);
    case East:
      return Position(position.offset + Offset(width_ - 1, height_ / 2), SouthWest);
    case SouthEast:
      return Position(position.offset + Offset(width_ - 1, 0), SouthWest);
    case South:
      return Position(position.offset + Offset(width_ / 2, 0), SouthWest);
    default:
    case SouthWest:
      return Position(position.offset + Offset(0, 0), SouthWest);
    case West:
      return Position(position.offset + Offset(0, height_ / 2), SouthWest);
    case NorthWest:
      return Position(position.offset + Offset(0, height_ - 1), SouthWest);
    case Center:
      return Position(position.offset + Offset(width_  / 2, height_ / 2), SouthWest);
    }
  }

  std::string GetName() const { return name_; }
  std::string GetTitle() const { return title_; }
  int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }
  double GetXlimLeft() const { return xlim_left_; }
  double GetXlimRight() const { return xlim_right_; }
  double GetYlimBottom() const { return ylim_bottom_; }
  double GetYlimTop() const { return ylim_top_; }
  PenStyle& GetPenStyle() { return penstyle_; }
  const PenStyle& GetPenStyle() const { return penstyle_; }

  // Setters

  Subtype& SetName(std::string name) {
    name_ = name;
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetTitle(std::string title) {
    title_ = title;
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetPenStyle(const PenStyle &pen) {
    penstyle_ = pen;
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
  Position CalcBoxPosition(const Position& position, int box_width, int box_height) {
    switch (position.relative) {
    case North:
      return position - Offset(box_width / 2, box_height);
    default:
    case NorthEast:
      return position - Offset(box_width, box_height);
    case East:
      return position - Offset(box_width, box_height / 2);
    case SouthEast:
      return position - Offset(box_width, 0);
    case South:
      return position - Offset(box_width / 2, 0);
    case SouthWest:
      return position - Offset(0, 0);
    case West:
      return position - Offset(0, box_height / 2);
    case NorthWest:
      return position - Offset(0, box_height);
    case Center:
      return position - Offset(box_width  / 2, box_height / 2);
    }
  }

  void AdjustAbsolutePosition2(Position& position, int box_width, int box_height) const {
    if (!position.IsAbsolute()) {
      position = GetAbsolutePosition(position);
    }
    int new_col = std::min<int>(position.offset.GetCol(), width_ - box_width);
    int new_row = std::min<int>(position.offset.GetRow(), height_ - 1);
    new_col = std::max(new_col, 0);
    new_row = std::max(new_row, box_height - 1);
    position.offset = Offset(new_col, new_row);
  }

  void AdjustAbsolutePosition(Position& position, int box_width, int box_height) const {
    if (!position.IsAbsolute()) {
      position = GetAbsolutePosition(position);
    }
    
    int new_col = std::max(0, position.offset.GetCol());
    int new_row = std::min(height_ - 1, position.offset.GetRow());
    int free_cols = width_ - new_col;
    int free_rows = new_row + 1;
    
    if (free_cols < box_width) {
      new_col = std::max(0, width_ - box_width);
    }
    if (free_rows < box_height) {
      new_row = std::min(height_ - 1, box_height - 1);
    }
    
    position.offset = Offset(new_col, new_row);
  }

  template<class Tx, class Ty>
  void SetAutoLimits(const std::vector<Tx>& x,
                     const std::vector<Ty>& y) {
    auto x_margin_surplus = [this](){ return std::abs((xlim_right_ - xlim_left_) * xlim_margin_); };
    auto y_margin_surplus = [this](){ return std::abs((ylim_top_ - ylim_bottom_) * ylim_margin_); };
    
    // Left and Right
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

  std::string name_;
  std::string title_;
  PenStyle penstyle_;
  Borders autolimit_;
  double xlim_margin_;
  double xlim_left_;
  double xlim_right_;
  double ylim_margin_;
  double ylim_bottom_;
  double ylim_top_;
  std::vector<PlotMetadata> metadata_;
};

class Plot final : public __Plot<Plot> { using __Plot::__Plot; };

//******************************** HistPlot *********************************//

template<class Subtype>
class __HistPlot : public __Plot<Subtype> { 
public:
  __HistPlot(int width = kConsoleWidth, int height = kConsoleHeight)
        : __Plot<Subtype>(width, height) { }
};

class HistPlot final : public __HistPlot<HistPlot> { using __HistPlot::__HistPlot; };

//***************************** Free functions ******************************//

template<class T>
T EmptyLike(const T& plot) {
  static_assert(std::is_base_of<__Plot<T>, T>::value, "Template type T must be a subtype of __Plot<T>.");
  return T(plot.GetWidth(), plot.GetHeight());
}

} // namespace askiplot

#endif // ASKIPLOT_HPP_