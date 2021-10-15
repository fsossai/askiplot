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
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

#define ASKIPLOT_VERSION_MAJOR 0
#define ASKIPLOT_VERSION_MINOR 1
#define ASKIPLOT_VERSION_PATCH 0

namespace askiplot {

//************************** Defauts and constants **************************//

std::string DefaultBrushMain = "_";
std::string DefaultBrushArea = "#";
std::string DefaultBrushBlank = " ";
std::string DefaultBrushBorderTop = "_";
std::string DefaultBrushBorderBottom = "_";
std::string DefaultBrushBorderLeft = "|";
std::string DefaultBrushBorderRight = "|";
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

class InvalidBrushValue : public std::exception {
public:
  virtual const char* what() const noexcept override {
    return
      "The value of a Brush must be a single ASCII character or a single UTF16 char "
      "and cannot be 0x00 (string termination character).";
  }
};

//******************************* Enumerators *******************************//

enum Borders : char {
  None = 0x00, Left = 0x01, Right = 0x02, Bottom = 0x04, Top = 0x08, All = 0x0F
};

enum RelativePosition : char {
  North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest, Center
};

enum AdjustPosition : bool {
  Adjust = true, DontAdjust = false
};

enum BlankFusion : bool {
  KeepBlanks = true, IgnoreBlanks = false
};

//******************** Namespace-private free functions *********************//

namespace {

} // private namespace

//**************************** Offset & Position ****************************//

using offset_t = std::pair<int, int>;

class Offset {
public:
  Offset()
      : col_(0)
      , row_(0) { }

  Offset(int col, int row)
      : col_(col)
      , row_(row) { }

  Offset(const offset_t& cr_pair)
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

  Position(const offset_t& pair, const RelativePosition& relative = SouthWest)
      : offset(pair)
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

Offset operator-(const Offset& a) {
  return Offset(-a.GetCol(), -a.GetRow());
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

Position operator+(const RelativePosition& relative, const offset_t& offset) {
  return Position(Offset(offset), relative);
}

Position operator+(const offset_t& offset, const RelativePosition& relative) {
  return Position(Offset(offset), relative);
}

Position operator+(const Position& position, const offset_t& offset) {
  return Position(position.offset + Offset(offset), position.relative);
}

Position operator-(const Position& position, const offset_t& offset) {
  return Position(position.offset - Offset(offset), position.relative);
}

double operator ""_percent(long double p) {
  return p / 100.0;
}

//********************************** Brush **********************************//

class Brush {
public:
  Brush()
      : name_("Blank") {
    SetValue(DefaultBrushBlank);
  }

  Brush(const std::string& value)
      : name_("Main") {
    SetValue(value);
  }

  Brush(const char *value)
      : name_("Main") {
    SetValue(value);
  }

  Brush(char value)
      : name_("Main") {
    SetValue(std::string(1, value));
  }

  Brush(const std::string& name, const std::string& value) {
    SetName(name);
    SetValue(value);
  }

  Brush(const std::string& name, char value)
      : Brush(name, std::string(1, value)) { }

  Brush(const Brush& other) {
    name_ = other.name_;
    value_ = other.value_;
  }

  Brush(Brush&& other) {
    name_ = std::move(other.name_);
    value_ = std::move(other.value_);
  }

  Brush& operator=(const Brush& other) {
    name_ = other.name_;
    value_ = other.value_;
    return *this;
  }

  Brush& operator=(Brush&& other) {
    name_ = std::move(other.name_);
    value_ = std::move(other.value_);
    return *this;
  }

  bool IsGeneral() const {
    return name_ == "*";
  }

  // Getters

  std::string GetName() const { return name_; }
  std::string GetValue() const { return value_; }

  // Setters

  Brush& SetName(const std::string& name) {
    name_ = name.size() == 0 ? "Unnamed" : name;
    return *this;
  }

  Brush& SetValue(const std::string& value) {
    if (value.size() == 0) {
      throw InvalidBrushValue();
    } else if (std::isprint(static_cast<unsigned char>(value[0]))) {
      value_.resize(1);
      value_[0] = value[0];
    } else if (value.size() == 1) {
      throw InvalidBrushValue();
    } else {
      value_.resize(2);
      value_[0] = value[0];
      value_[1] = value[1];
    }
    return *this;  
  }

private:
  std::string name_;
  std::string value_;
};

class Palette {
public:
  Palette() {
    Reset();
  }

  Palette& operator()(const Brush& brush) {
    brushes_[brush.GetName()] = brush.GetValue();
    return *this;
  }

  template<class T>
  Palette& operator()(const std::string& brush_name, T brush_value) {
    Brush brush(brush_name, brush_value);
    this->operator()(brush);
    return *this;
  }

  std::string operator[](const std::string& brush_name) const {
    auto it = brushes_.find(brush_name);
    if (it == brushes_.end()) {
      return DefaultBrushBlank;
    }
    return it->second;
  }

  bool HasBrush(const std::string& name) const {
    return brushes_.find(name) != brushes_.end();
  }

  Palette& Reset() {
    brushes_.clear();
    brushes_.insert({
      {"Main", DefaultBrushMain},
      {"Blank", DefaultBrushBlank},
      {"Area", DefaultBrushArea},
      {"BorderTop", DefaultBrushBorderTop},
      {"BorderBottom", DefaultBrushBorderBottom},
      {"BorderLeft", DefaultBrushBorderLeft},
      {"BorderRight", DefaultBrushBorderRight},
    });
    return *this;
  }

  // Getters

  Brush GetBrush(const std::string& brush_name) const {
    auto it = brushes_.find(brush_name);
    if (it == brushes_.end()) {
      return {};
    }
    return Brush(brush_name, it->second);
  }

  // Setters

  Palette& SetBrush(const Brush& brush) {
    brushes_[brush.GetName()] = brush.GetValue();
    return *this;
  }

  Palette& SetBrush(const std::string& name, const std::string& value) {
    Brush brush(name, value);
    return SetBrush(brush);
  }

private:
  std::unordered_map<std::string, std::string> brushes_;
};

//****************************** PlotMetaData *******************************//

struct PlotMetadata {
  PlotMetadata& SetLabel(const std::string& label) { this->label = label; return *this; }
  PlotMetadata& SetLength(std::size_t length) { this->length = length; return *this; }
  PlotMetadata& SetBrush(const Brush& brush) { this->brush = brush; return *this; }
  
  Brush brush;
  std::size_t length = 0;
  std::string label = "";
};

class IPlot {
public:
  virtual Brush& at(int col, int row) = 0;
  virtual const Brush& at(int col, int row) const = 0;
  virtual std::string Serialize() const = 0;
  virtual int GetWidth() const = 0;
  virtual int GetHeight() const = 0;
protected:
  int width_;
  int height_;
  std::vector<Brush> canvas_;
};

//********************************** Plot ***********************************//

// Forward declaration
template<class T>
class PlotFusion;

template<class Subtype>
class __Plot : public IPlot {
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

  void AdjustAbsolutePosition(Position& position,
                              int box_width,
                              int box_height,
                              bool drawing_upwards) const {
    if (!position.IsAbsolute()) {
      position = GetAbsolutePosition(position);
    }

    int new_col = std::max(0, position.offset.GetCol());
    int new_row = std::min(height_ - 1, position.offset.GetRow());
    int free_cols = width_ - new_col;
    int free_rows = drawing_upwards ? (height_ - new_row) : (new_row + 1);

    if (free_cols < box_width) {
      new_col = std::max(0, width_ - box_width);
    }
    if (free_rows < box_height) {
      new_row = drawing_upwards ? std::max(0, height_ - box_height)
                                : std::min(height_ - 1, box_height - 1);
    }
    
    position.offset = Offset(new_col, new_row);
  }

  virtual Brush& at(int col, int row) override {
    return canvas_[row + height_*col];
  }

  virtual const Brush& at(int col, int row) const override {
    return canvas_[row + height_*col];
  }

  Subtype& AutoLimit(Borders borders) {
    autolimit_ = borders;
    return static_cast<Subtype&>(*this);
  }

  Subtype& Clear() {
    auto brush_blank = Brush();
    std::fill(canvas_.begin(), canvas_.end(), brush_blank);
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawBorders(Borders borders) {
    if (borders & Borders::Left) {
      const auto brush_left = palette_.GetBrush("BorderLeft");
      for (int j = 0; j < height_; ++j) {
        at(0, j) = brush_left;
      }
    }
    if (borders & Borders::Right) {
      const auto brush_right = palette_.GetBrush("BorderRight");
      for (int j = 0; j < height_; ++j) {
        at(width_ - 1, j) = brush_right;
      }
    }
    if (borders & Borders::Bottom) {
      const auto brush_bottom = palette_.GetBrush("BorderBottom");
      for (int i = 0; i < width_; ++i) {
        at(i, 0) = brush_bottom;
      }
    }
    if (borders & Borders::Top) {
      const auto brush_top = palette_.GetBrush("BorderTop");
      for (int i = 0; i < width_; ++i) {
        at(i, height_ - 1) = brush_top;
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

    // Setting up Brushes
    auto brush_top = Brush("BorderTop", DefaultBrushBorderTop);
    auto brush_bottom = Brush("BorderBottom", DefaultBrushBorderBottom);
    auto brush_left = Brush("BorderLeft", DefaultBrushBorderLeft);
    auto brush_right = Brush("BorderRight", DefaultBrushBorderRight);

    // Drawing box borders
    for (int i = pos_col; i < pos_col + box_width; ++i) {
      at(i, pos_row) = brush_bottom;                 // Bottom
      at(i, pos_row + box_height - 1) = brush_top;   // Top
    }
    for (int j = pos_row; j < pos_row + box_height - 1; ++j) {
      at(pos_col, j) = brush_left;                    // Left
      at(pos_col + box_width - 1, j) = brush_right;   // Right
    }

    // Writing labels
    for (std::size_t i = 0; i < metadata_.size(); ++i) {
      DrawText(metadata_[i].brush.GetValue() + " " + metadata_[i].label,
               Position(pos_col + 2, pos_row + box_height - 2 - i)
      );
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawLine(double x_begin, double y_begin, double x_end, double y_end) {
    auto brush_line = palette_.GetBrush("Main");

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
        at(to_col(x_current), row_beg + j) = brush_line;
        x_current += x_adv;
      }
    } else {
      const double y_adv = (y_end - y_begin) / n;
      const int i_adv = (col_beg < col_end) ? +1 : -1;
      double y_current = y_begin;
      for (int i = 0; std::abs(i) < n; i += i_adv) {
        at(col_beg + i, to_row(y_current)) = brush_line;
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
      auto brush_line = palette_.GetBrush("Main");
      for (int i = 0; i < width_; ++i) {
        at(i, row) = brush_line;
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
      auto brush_line = palette_.GetBrush("Main");
      for (int j = 0; j < height_; ++j) {
        at(col, j) = brush_line;
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
        = palette_.GetBrush("Main");
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
    const auto brush_line = palette_.GetBrush("Main");

    for (std::size_t i = 0; i < n; ++i) {
      if (xlim_left_   < x[i] && x[i] < xlim_right_ &&
          ylim_bottom_ < y[i] && y[i] < ylim_top_) {
        at(static_cast<int>((x[i] - xlim_left_  ) / xstep),
           static_cast<int>((y[i] - ylim_bottom_) / ystep)) = brush_line;
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
      AdjustAbsolutePosition(pos_abs, text.size(), 1, false);
    }

    const int col = pos_abs.offset.GetCol();
    const int row = pos_abs.offset.GetRow();

    if (0 <= row && row < height_) {
      int n = std::min<int>(width_ - col, text.size());
      for (int i = 0; i < n; ++i) {
        at(i + col, row) = Brush("*", text[i]);
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
      AdjustAbsolutePosition(pos_abs, 1, text.size(), false);
    }

    const int col = pos_abs.offset.GetCol();
    const int row = pos_abs.offset.GetRow();

    if (0 <= col && col < width_) {
      int n = std::min<int>(row + 1, text.size());
      for (int j = 0; j < n; ++j) {
        at(col, row - j) = Brush("*", text[j]);
      }
    } else { std::cout << col << "\t" << row << std::endl; }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawTextVerticalCentered(const std::string& text,
                                    const Position& position,
                                    AdjustPosition adjust = Adjust) {
    return DrawTextVertical(text, position + Offset(0, text.size() / 2), adjust);
  }

  Subtype& DrawTitle() {
    return DrawTextCentered(title_, North, DontAdjust);
  }

  Subtype Extract(const Position& corner1, const Position& corner2) const {
    auto pos_abs1 = GetAbsolutePosition(corner1);
    auto pos_abs2 = GetAbsolutePosition(corner2);

    const int col_beg = std::min(pos_abs1.offset.GetCol(), pos_abs2.offset.GetCol());
    const int row_beg = std::min(pos_abs1.offset.GetRow(), pos_abs2.offset.GetRow());
    const int w = std::abs(pos_abs2.offset.GetCol() - pos_abs1.offset.GetCol()) + 1;
    const int h = std::abs(pos_abs2.offset.GetRow() - pos_abs1.offset.GetRow()) + 1;

    Subtype extracted(w, h);
    for (int i = 0; i < w; ++i) {
      for (int j = 0; j < h; ++j) {
        extracted.at(i, j) = this->at(i + col_beg, j + row_beg);
      }
    }
    return extracted;
  }

  Subtype& Fill(const Brush& brush) {
    std::fill(canvas_.begin(), canvas_.end(), brush);
    return static_cast<Subtype&>(*this);
  }

  Subtype& Fill() {
    return Fill(palette_.GetBrush("Main"));
  }

  PlotFusion<Subtype> Fusion() {
    return PlotFusion<Subtype>(static_cast<Subtype&>(*this));
  }

  bool IsLike(const IPlot& other) const {
    return (width_ == other.GetWidth() && height_ == other.GetHeight());
  }

  Subtype& Move(const Offset& offset) {
    auto copy = *this;
    Clear();
    Fusion()(copy, offset, DontAdjust).Fuse();
    return static_cast<Subtype&>(*this);
  }

  Subtype& NewBrush(const Brush& brush) {
    palette_(brush.GetName(), brush.GetValue());
    return static_cast<Subtype&>(*this);
  }

  Subtype& NewBrush(const std::string& name, const std::string& value) {
    return NewBrush(Brush(name, value));
  }

  Palette& NewBrushes() {
    return palette_;
  }

  const Palette& NewBrushes() const {
    return palette_;
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
                    .SetBrush(palette_.GetBrush("Main"))
    );
    return static_cast<Subtype&>(*this);
  }

  template<class Tx, class Ty>
  Subtype& PlotData(const std::vector<Tx>& x,
                    const std::vector<Ty>& y,
                    const std::string& label) {
    return PlotData(x, y, label, std::numeric_limits<std::size_t>::max());
  }

  Subtype& Redraw() {
    for (auto& brush : canvas_) {
      if (!brush.IsGeneral()) {
        brush = palette_.GetBrush(brush.GetName());
      }
    }
    return static_cast<Subtype&>(*this);
  }

  std::string Serialize() const override {
    std::stringstream ss("");
    for (int j = height_ - 1; j >= 0; --j) {
      for (int i = 0; i < width_; ++i) {
        //std::cout << "[" << at(i, j).GetValue() << "]";
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
  int GetWidth() const override { return width_; }
  int GetHeight() const override { return height_; }
  double GetXlimLeft() const { return xlim_left_; }
  double GetXlimRight() const { return xlim_right_; }
  double GetYlimBottom() const { return ylim_bottom_; }
  double GetYlimTop() const { return ylim_top_; }
  Palette& GetPalette() { return palette_; }
  const Palette& GetPalette() const { return palette_; }

  // Setters

  Subtype& SetMainBrush(const std::string& value) {
    palette_("Main", value);
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetMainBrush(char value) {
    return SetMainBrush(std::string(1, value));
  }

  Subtype& SetName(std::string name) {
    name_ = name;
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetTitle(std::string title) {
    title_ = title;
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
  Palette palette_;
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

//******************************* PlotFusion ********************************//

template<class T>
class PlotFusion {
public:
  PlotFusion(T& baseplot)
      : baseplot_(baseplot) {
    static_assert(std::is_base_of<__Plot<T>, T>::value, "Template type T must be a subtype of __Plot<T>.");
  }

  template<class U>
  PlotFusion& operator()(const U& other,
                         const Position& position = SouthWest,
                         AdjustPosition adjust = Adjust) {
    auto pos_abs = baseplot_.GetAbsolutePosition(position);
    if (adjust) {
      baseplot_.AdjustAbsolutePosition(pos_abs, other.GetWidth(), other.GetHeight(), true);
    }
    plots_offsets_.push_back({&other, pos_abs.offset});
    return *this;
  }

  T& Fuse(BlankFusion keep_blanks = KeepBlanks) {
    const int base_width = baseplot_.GetWidth();
    const int base_height = baseplot_.GetHeight();
    for (const auto& po : plots_offsets_) {
      const auto plot = std::get<const IPlot*>(po);
      const auto offset = std::get<Offset>(po);

      const int off_c = offset.GetCol();
      const int off_r = offset.GetRow();
      const int col_beg = std::max(0, -off_c);
      const int col_end = std::min(plot->GetWidth(), base_width - off_c);
      const int row_beg = std::max(0, -off_r);
      const int row_end = std::min(plot->GetHeight(), base_height - off_r);

      if (keep_blanks) {
        for (int i = col_beg; i < col_end; ++i) {
          for (int j = row_beg; j < row_end; ++j) {
            baseplot_.at(i + off_c, j + off_r) = plot->at(i, j);
          }
        }
      } else {
        for (int i = col_beg; i < col_end; ++i) {
          for (int j = row_beg; j < row_end; ++j) {
            if (!(plot->at(i, j).GetName() == "Blank")) {
              baseplot_.at(i + off_c, j + off_r) = plot->at(i, j);
            }
          }
        }
      }
    }
    return baseplot_;
  }

private:
  T& baseplot_;
  std::vector<std::pair<const IPlot*, Offset>> plots_offsets_;
};

//******************************** HistPlot *********************************//

template<class Subtype>
class __HistPlot : public __Plot<Subtype> { 
public:
  __HistPlot(int width = kConsoleWidth, int height = kConsoleHeight)
      : __Plot<Subtype>(width, height) {
    nbins_ = this->GetWidth();
  }

  template<class T>
  Subtype& PlotHistogram(const std::vector<T>& data, double height_resize = 0.8) {
    static_assert(std::is_arithmetic<T>::value,
      "PlotHistogram only supports vectors of arithmetic types.");
    
    const int distinct = std::set<T>(data.begin(), data.end()).size();
    nbins_ = std::min(nbins_, distinct);

    auto mm = std::minmax_element(data.begin(), data.end());
    const T min = *mm.first;
    const T max = *mm.second;
    const T step = (max - min) / (nbins_ - 1);
    this->xlim_left_ = min - step / 2;
    this->xlim_right_ = max + step / 2;

    std::vector<int> counts(nbins_);
    for (const auto& i : data) {
      int idx = (i - this->xlim_left_) / step;
      ++counts[idx];
    }

    const int max_bar_height = *std::max_element(counts.begin(), counts.end());
    std::vector<int> bars = counts;
    const double factor = std::min(1.0, height_resize);
    for (auto& i : bars) {
      i = i / static_cast<double>(max_bar_height) * this->GetHeight() * factor;
    }
  
    const auto brush_top = this->palette_.GetBrush("BorderTop");
    const auto brush_left = this->palette_.GetBrush("BorderLeft");
    const auto brush_right = this->palette_.GetBrush("BorderRight");
    const auto brush_area = this->palette_.GetBrush("Area");
    const int bin_width = this->GetWidth() / nbins_;

    for (int i = 0; i < nbins_; ++i) {
      for (int k = 0; k < bin_width; ++k) {
        for (int j = 0; j < bars[i] + 1; ++j) {
          if (k == 0) {
            if (j != bars[i]) {
              this->at((i * bin_width) + k, j) = brush_left;
            }
          } else if (k == bin_width - 1) {
            if (j != bars[i]) {
              this->at((i * bin_width) + k, j) = brush_right;
            }
          } else if (j == bars[i]) {
            this->at((i * bin_width) + k, j) = brush_top;
          } else {
            this->at((i * bin_width) + k, j) = brush_area;
          }
        }
      }
    }
    return static_cast<Subtype&>(*this);
  }
  
protected:
  int nbins_;
};

class HistPlot final : public __HistPlot<HistPlot> { using __HistPlot::__HistPlot; };

//***************************** Free functions ******************************//

template<class T>
T BlankLike(const T& plot) {
  static_assert(std::is_base_of<__Plot<T>, T>::value, "Template type T must be a subtype of __Plot<T>.");
  return T(plot.GetWidth(), plot.GetHeight());
}

} // namespace askiplot

#endif // ASKIPLOT_HPP_