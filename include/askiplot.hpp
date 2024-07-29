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

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace askiplot {

//********************************* Version *********************************//

static constexpr struct {
  int major, minor, patch;
} version = {
  0, 2, 0
};

//********************** Free functions declaration *************************//

class Brush;

template<class T>
T BlankLike(const T& plot);
std::vector<Brush> StringToBrushes(const std::string& str);
std::vector<Brush> StringToBrushes(const char *str);


//************************** Defaults and constants *************************//

std::string DefaultBrushMain = "_";
std::string DefaultBrushArea = "#";
std::string DefaultBrushBlank = " ";
std::string DefaultBrushBorderTop = "_";
std::string DefaultBrushBorderBottom = "_";
std::string DefaultBrushBorderLeft = "|";
std::string DefaultBrushBorderRight = "|";
std::string DefaultBrushLineHorizontal = "-";
std::string DefaultBrushLineVertical = "|";
int BarValuePrecision = 0;
const std::vector<Brush> kLetterBrushes = StringToBrushes("abcdefghijklmnopqrstuvwxyz");
const std::vector<Brush> kNumberBrushes = StringToBrushes("0123456789");
const std::vector<Brush> kSymbolBrushes = StringToBrushes("@$*#.+&*=?,-%!^\"<~>'");

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

class BMPFormatNotSupported : public std::exception {
public:
  virtual const char* what() const noexcept override {
    return "BMP format not supported";
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

template<class Ty>
std::string FormatValue(Ty value) {
  return std::to_string(value);
}

template<>
std::string FormatValue<double>(double value) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(BarValuePrecision) << value;
  std::string formatted = oss.str();
  formatted.erase(formatted.find_last_not_of('0') + 1, std::string::npos);

  if (formatted.back() == '.') {
    formatted.pop_back();
  }

  return formatted;
}

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

class Position {
public:
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
      : name_("*") {
    SetValue(value);
  }

  Brush(const char *value)
      : name_("*") {
    SetValue(value);
  }

  Brush(char value)
      : name_("*") {
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
    name_ = name.size() == 0 ? "*" : name;
    return *this;
  }

  Brush& SetValue(const std::string& value) {
    if (value.size() == 0) {
      throw InvalidBrushValue();
    } else if (std::isprint(static_cast<uint8_t>(value[0]))) {
      value_.resize(1);
      value_[0] = value[0];
    } else if (value.size() == 1) {
      if (value[0] == '\t' || value[0] == '\n' || value[0] == '\r') {
        value_[0] = ' ';
      } else {
        throw InvalidBrushValue();
      }
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

//********************************* Palette *********************************//

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
      {"LineHorizontal", DefaultBrushLineHorizontal},
      {"LineVertical", DefaultBrushLineVertical},
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

  Palette& SetBrush(const std::string name, const std::string& value) {
    Brush brush(name, value);
    return SetBrush(brush);
  }

  Palette& SetBrush(const std::vector<std::string>& names, const std::string& value) {
    for (std::size_t i = 0; i < names.size(); ++i) {
      SetBrush(Brush(names[i], value));
    }
    return *this;
  }

private:
  std::unordered_map<std::string, std::string> brushes_;
};

//****************************** PlotMetaData *******************************//

template<class Subtype>
class __PlotMetadata {
public:
  Subtype& SetLabel(const std::string& label) {
    this->label = label;
    return static_cast<Subtype&>(*this);
  }
  
  Subtype& SetLength(std::size_t length) {
    this->length = length;
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetBrush(const Brush& brush) {
    this->brush = brush;
    return static_cast<Subtype&>(*this);
  }
  
  Brush brush;
  std::size_t length = 0;
  std::string label = "";
};

class PlotMetadata final : public __PlotMetadata<PlotMetadata> { };

//***************************** BarPlotMetadata *****************************//

template<class Subtype>
class __BarPlotMetadata : public __PlotMetadata<Subtype> {
public:
  Subtype& SetBarYdata(std::vector<double> ydata) {
    this->ydata = std::move(ydata);
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetInteger(bool integer) {
    is_integer = integer;
    return static_cast<Subtype&>(*this);
  }

  std::vector<double> ydata;
  bool is_integer;
};

class BarPlotMetadata final : public __BarPlotMetadata<BarPlotMetadata> { };

//******************************** Gamma ************************************//

template<class Subtype>
class __Gamma {
public:
  __Gamma() = default;
  virtual ~__Gamma() = default;
  virtual Brush operator()(uint8_t level) = 0;
};

//****************************** FixedGamma *********************************//

template<class Subtype>
class __FixedGamma : public __Gamma<Subtype> {
public:
  __FixedGamma(const std::string& gamma) {
    Set(gamma);
  }

  __FixedGamma() {
    Set("  ..oo00#@");
  }

  Brush operator()(uint8_t level) override {
    return Brush("*", recodedGamma_[level]);
  }

  Subtype& Shuffle() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(gamma_.begin(), gamma_.end(), g);
    return Set(gamma_);
  }

  std::string ToString() const { return gamma_; }

  Subtype& Set(const std::string& gamma) {
    std::size_t new_size = std::min<std::size_t>(gamma.size(), 256L);
    gamma_.resize(new_size);
    std::copy_n(gamma.begin(), new_size, gamma_.begin());

    recodedGamma_.resize(256);
    const int levels = gamma.size();
    const int div = 256 / levels;
    int rem = 256 % levels;

    auto rgIt = recodedGamma_.begin();
    for (int i = 0; i < levels; ++i) {
      const int ncopies = div + ((rem > 0) ? 1 : 0);
      rgIt = std::fill_n(rgIt, ncopies, gamma[i]);
      rem--;
    }

    return static_cast<Subtype&>(*this);
  }

protected:
  std::string gamma_;
  std::string recodedGamma_;
};

class FixedGamma final : public __FixedGamma<FixedGamma> { using __FixedGamma::__FixedGamma; };

//**************************** VariableGamma ********************************//

template<class Subtype>
class __VariableGamma : public __Gamma<Subtype> {
public:
  __VariableGamma() {
    SetZeroThreshold(128);
    SetZeroBrush(" ");
  }

  // Getters

  Brush GetZeroBrush() const { return zero_; }
  uint8_t GetZeroThreshold() const { return threshold_; }

  // Setters

  Subtype& SetZeroBrush(Brush brush) {
    brush.SetName("*");
    zero_ = brush;
    return static_cast<Subtype&>(*this);
  }

  Subtype& SetZeroThreshold(uint8_t t) {
    threshold_ = t;
    return static_cast<Subtype&>(*this);
  }
  
protected:
  uint8_t threshold_;
  Brush zero_;
};

//****************************** RandomGamma ********************************//

template<class Subtype>
class __RandomGamma : public __VariableGamma<Subtype> {
public:
  __RandomGamma(const std::string& gamma) {
    Set(gamma);
  }

  Brush operator()(uint8_t level) override {
    if (level < this->threshold_) {
      return this->zero_;
    }
    return gamma_[rand() % gamma_.size()];
  }

  std::string ToString() const { return gamma_; }
  
  Subtype& Set(const std::string& gamma) {
    std::size_t new_size = std::min<std::size_t>(gamma.size(), 256L);
    gamma_.resize(new_size);
    std::copy_n(gamma.begin(), new_size, gamma_.begin());
    return static_cast<Subtype&>(*this);
  }
  
protected:
  std::string gamma_;
};

class RandomGamma final : public __RandomGamma<RandomGamma> { using __RandomGamma::__RandomGamma; };

//******************************* TextGamma *********************************//

template<class Subtype>
class __TextGamma : public __VariableGamma<Subtype> {
public:
  __TextGamma() {
    text_ = "AskiPlot";
  }

  __TextGamma(const std::string& text) {
    SetText(text);
  }

  Brush operator()(uint8_t level) {
    if (level < this->threshold_) {
      return this->zero_;
    }
    return text_[use_count_++ % text_.size()];
  }

  // Getters
  std::string GetText() const { return text_; }

  // Setters
  Subtype& SetText(const std::string& text) {
    text_ = text;
    if (text.size() == 0) {
      text_ = " ";
    }
    return static_cast<Subtype&>(*this);
  }

protected:
  std::string text_;
  int use_count_ = 0;
  bool repeat_;
};

class TextGamma final : public __TextGamma<TextGamma> { using __TextGamma::__TextGamma; };

//******************************* BMPImage **********************************//

struct BMPImage {
  int size;
  int reserved;
  int offset;
  int header_length;
  int width;
  int height;
  int16_t planes;
  int16_t bits_per_pixel;
  int compression;
  int raw_size;
  int vertical_resolution;
  int horizontal_resolution;
  int colors;
  int important_colors;
};

//********************************* Image ***********************************//

class Image {
public:
  Image(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    input.unsetf(std::ios::skipws);
    raw_.insert(raw_.begin(), std::istream_iterator<uint8_t>(input), {});

    // Parsing
    std::string format = "XX";
    format[0] = raw_[0];
    format[1] = raw_[1];
    if (!CheckFormat(format)) {
      throw BMPFormatNotSupported();
    }

    data_ = reinterpret_cast<BMPImage*>(raw_.data() + 2);
    if (data_->height < 0 || data_->width < 0) {
      throw BMPFormatNotSupported();
    }
    width_ = data_->width;
    height_ = data_->height;
    img_.resize(width_ * height_);

    ParsePayload();
  }
  
  int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }

  int& At(int x, int y) { return img_[x + y * width_]; }
  const int& At(int x, int y) const { return img_[x + y * width_]; }

  Image& Invert() {
    std::transform(img_.begin(), img_.end(), img_.begin(),
      [](int level) { return 255 - level; });
    return *this;
  }

  Image& Resize(int new_width, int new_height) {
    if (new_width > width_ || new_height > height_) {
      return *this;
    }

    std::vector<double> sums(new_width * new_height);

    const int div_w = width_ / new_width;
    const int div_h = height_ / new_height;
    const int rem_w = width_ % new_width;
    const int rem_h = height_ % new_height;
    int block_i_offset = 0;

    for (int i = 0; i < new_height; ++i) {
      const int block_i_len = div_h + (i < rem_h);
      int block_j_offset = 0;
      for (int j = 0; j < new_width; ++j) {
        const int block_j_len = div_w + (j < rem_w);
        const int sums_idx = j + i * new_width;
        for (int block_i = 0; block_i < block_i_len; ++block_i) {
          for (int block_j = 0; block_j < block_j_len; ++block_j) {
            sums[sums_idx] += At(block_j_offset + block_j, block_i_offset + block_i);
          }
        }
        sums[sums_idx] /= static_cast<double>(block_i_len * block_j_len);
        block_j_offset += block_j_len;
      }
      block_i_offset += block_i_len;
    }

    img_.resize(sums.size());
    std::transform(sums.begin(),
                   sums.end(),
                   img_.begin(),
                   [](double x) -> int { return x;});

    width_ = new_width;
    height_ = new_height;

    return *this;
  }

  Image& Resize(double ratio) {
    if (ratio >= 1.0) {
      return *this;
    }
    return *this;
  }

private:
  std::vector<int> img_;
  std::vector<uint8_t> raw_;
  int width_, height_;
  BMPImage *data_ = nullptr;

  bool CheckFormat(const std::string& fmt) {
    if (fmt == "BM" || fmt == "BA" || fmt == "CI" || fmt == "CP" ||
        fmt == "IC" || fmt == "PC") {
      return true;
    }
    return false;
  }

  void ParsePayload() {
    switch (data_->bits_per_pixel) {
      case 1:
        ParsePayload_1();
        break;
      case 24:
        ParsePayload_24();
        break;
      case 32:
        ParsePayload_32();
        break;
      default:;
        throw BMPFormatNotSupported();
    }
  }

  void ParsePayload_1() {
    uint8_t *current = &(raw_.data()[data_->offset]);
    int img_idx = 0;
    const int full_bytes = width_ / 8;
    const int residual_bits = width_ - full_bytes * 8;
    const int skip = residual_bits / 8;
    for (int i = 0; i < height_; ++i) {
      uint8_t temp;
      for (int j = 0; j < full_bytes; ++j) {
        temp = *current;
        img_[img_idx++] = !!(temp & 0x80) * 255;
        img_[img_idx++] = !!(temp & 0x40) * 255;
        img_[img_idx++] = !!(temp & 0x20) * 255;
        img_[img_idx++] = !!(temp & 0x10) * 255;
        img_[img_idx++] = !!(temp & 0x08) * 255;
        img_[img_idx++] = !!(temp & 0x04) * 255;
        img_[img_idx++] = !!(temp & 0x02) * 255;
        img_[img_idx++] = !!(temp & 0x01) * 255;
        ++current;
      }
      temp = *current;
      uint8_t mask = 0x80;
      for (int j = 0; j < residual_bits; ++j) {
        img_[img_idx++] = !!(temp & mask) * 255;
        mask >>= 1;
      }
      current += skip;
    }
  }

  void ParsePayload_24() {
    uint8_t *current = reinterpret_cast<uint8_t*>(&(raw_.data()[data_->offset]));
    int img_idx = 0;
    const int skip = 4 - ((width_ * 3) % 4);
    for (int i = 0; i < height_; ++i) {
      for (int j = 0; j < width_; ++j) {
        img_[img_idx++] = (static_cast<uint32_t>(*(current + 0)) +
                           static_cast<uint32_t>(*(current + 1)) +
                           static_cast<uint32_t>(*(current + 2))) / 3.0;
        current += 3;
      }
      current += skip;
    }
  }

  void ParsePayload_32() {
    uint8_t *current = reinterpret_cast<uint8_t*>(&(raw_.data()[data_->offset]));
    int img_idx = 0;
    for (int i = 0; i < height_; ++i) {
      for (int j = 0; j < width_; ++j) {
        img_[img_idx++] = (static_cast<uint32_t>(*(current + 0)) +
                           static_cast<uint32_t>(*(current + 1)) +
                           static_cast<uint32_t>(*(current + 2))) / 3.0;
        current += 4;
      }
    }
  }
};

//********************************** IPlot **********************************//

class IPlot {
public:
  virtual Brush& At(int col, int row) = 0;
  virtual const Brush& At(int col, int row) const = 0;
  virtual std::string Serialize() const = 0;
  virtual int GetWidth() const = 0;
  virtual int GetHeight() const = 0;
protected:
  int width_;
  int height_;
  std::vector<Brush> canvas_;
};

//********************************** Plot ***********************************//

// Forward declarations
template<class T>
class PlotFusion;

template<class T>
class __Gamma;

template<class Subtype>
class __Plot : public IPlot {
public:
  __Plot(int width = 0, int height = 0) {
    if (width < 0 || height < 0) {
      throw InvalidPlotSize();
    }
    if (width == 0 || height == 0) {
      struct winsize w;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
      if (width == 0) width_ = w.ws_col;
      if (height == 0) height_ = w.ws_row - 1;
    } else {
      width_ = width;
      height_ = height;
    }
    autolimit_ = Borders::All;
    xlim_margin_ = 0.01;
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

  virtual Brush& At(int col, int row) override {
    return canvas_[row + height_*col];
  }

  virtual const Brush& At(int col, int row) const override {
    return canvas_[row + height_*col];
  }

  Subtype& AutoLimit(Borders borders) {
    autolimit_ = borders;
    return static_cast<Subtype&>(*this);
  }

  Subtype& Clear() {
    Fill(palette_.GetBrush("Blank"));
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawBorders(Borders borders = Borders::All) {
    if (borders & Borders::Left) {
      const auto brush_left = palette_.GetBrush("BorderLeft");
      for (int j = 0; j < height_; ++j) {
        At(0, j) = brush_left;
      }
    }
    if (borders & Borders::Right) {
      const auto brush_right = palette_.GetBrush("BorderRight");
      for (int j = 0; j < height_; ++j) {
        At(width_ - 1, j) = brush_right;
      }
    }
    if (borders & Borders::Bottom) {
      const auto brush_bottom = palette_.GetBrush("BorderBottom");
      for (int i = 0; i < width_; ++i) {
        At(i, 0) = brush_bottom;
      }
    }
    if (borders & Borders::Top) {
      const auto brush_top = palette_.GetBrush("BorderTop");
      for (int i = 0; i < width_; ++i) {
        At(i, height_ - 1) = brush_top;
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawBox(const Position& corner1,
                   const Position& corner2,
                   const Brush& brush) {
    const auto pos_abs1 = GetAbsolutePosition(corner1);
    const auto pos_abs2 = GetAbsolutePosition(corner2);

    const int col_beg = std::min(pos_abs1.offset.GetCol(), pos_abs2.offset.GetCol());
    const int row_beg = std::min(pos_abs1.offset.GetRow(), pos_abs2.offset.GetRow());
    const int w_beg = -std::min(0, col_beg);
    const int h_beg = -std::min(0, row_beg);
    const int len_w = std::abs(pos_abs2.offset.GetCol() - pos_abs1.offset.GetCol()) + 1 - w_beg;
    const int len_h = std::abs(pos_abs2.offset.GetRow() - pos_abs1.offset.GetRow()) + 1 - h_beg;
    const int w_end = std::min(w_beg + len_w, width_);
    const int h_end = std::min(h_beg + len_h, height_);

    for (int i = w_beg; i < w_end; ++i) {
      for (int j = h_beg; j < h_end; ++j) {
        At(i + col_beg, j + row_beg) = brush;
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawBox(const Position& corner1, const Position& corner2) {
    return DrawBox(corner1, corner2, palette_.GetBrush("Area"));
  }

  template<class T>
  Subtype& DrawImage(const Image& img,
                     T gamma,
                     const Position& position,
                     int img_width,
                     int img_height) {
    static_assert(std::is_base_of<__Gamma<T>, T>::value, "Template type T must be a subtype of __Gamma<T>.");

    Image img_fit = img;
    if (img.GetWidth() > img_width || img.GetHeight() > img_height) {
      img_fit.Resize(img_width, img_height);
    }

    const int len_w = std::min(img.GetWidth(), img_width);
    const int len_h = std::min(img.GetHeight(), img_height);
    __Plot subplot(len_w, len_h);

    for (int j = len_h - 1; j >= 0; --j) {
      for (int i = 0; i < len_w; ++i) {
        subplot.At(i, j) = gamma(img_fit.At(i, j));
      }
    }

    return Fuse(subplot, position);
  }

  template<class T = FixedGamma>
  Subtype& DrawImage(const Image& img,
                     T gamma = {},
                     const Position& position = {0,0}) {
    return DrawImage(img, gamma, position, width_, height_);
  }

  Subtype& DrawLegend(const Position& position = NorthEast) {
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
      At(i, pos_row) = brush_bottom;                 // Bottom
      At(i, pos_row + box_height - 1) = brush_top;   // Top
    }
    for (int j = pos_row; j < pos_row + box_height - 1; ++j) {
      At(pos_col, j) = brush_left;                    // Left
      At(pos_col + box_width - 1, j) = brush_right;   // Right
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
    auto brush = palette_.GetBrush("Main");

    const double xstep = (xlim_right_ - xlim_left_) / width_;
    const double ystep = (ylim_top_ - ylim_bottom_) / height_;
   
    auto to_col = [&](double x) -> int { return (x - xlim_left_  ) / xstep; };
    auto to_row = [&](double y) -> int { return (y - ylim_bottom_) / ystep; };
   
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
        At(to_col(x_current), row_beg + j) = brush;
        x_current += x_adv;
      }
    } else {
      const double y_adv = (y_end - y_begin) / n;
      const int i_adv = (col_beg < col_end) ? +1 : -1;
      double y_current = y_begin;
      for (int i = 0; std::abs(i) < n; i += i_adv) {
        At(col_beg + i, to_row(y_current)) = brush;
        y_current += y_adv;
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawLine(const Position& begin, const Position& end) {
    const double xstep = (xlim_right_ - xlim_left_) / width_;
    const double ystep = (ylim_top_ - ylim_bottom_) / height_;
    
    auto to_x = [&](int col) -> double { return col * xstep + xlim_left_; };
    auto to_y = [&](int row) -> double { return row * ystep + ylim_bottom_; };

    auto pos_beg_abs = GetAbsolutePosition(begin);
    auto pos_end_abs = GetAbsolutePosition(end);

    DrawLine(to_x(pos_beg_abs.offset.GetCol()), to_y(pos_beg_abs.offset.GetRow()),
             to_x(pos_end_abs.offset.GetCol()), to_y(pos_end_abs.offset.GetRow()));
    
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawLineHorizontalAtRow(int row) {
    if (row < height_) {
      auto brush = palette_.GetBrush("LineHorizontal");
      for (int i = 0; i < width_; ++i) {
        At(i, row) = brush;
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
      auto brush = palette_.GetBrush("LineVertical");
      for (int j = 0; j < height_; ++j) {
        At(col, j) = brush;
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
      At(static_cast<int>((x - xlim_left_  ) / xstep),
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
    const auto brush = palette_.GetBrush("Main");

    for (std::size_t i = 0; i < n; ++i) {
      if (xlim_left_   < x[i] && x[i] < xlim_right_ &&
          ylim_bottom_ < y[i] && y[i] < ylim_top_) {
        At(static_cast<int>((x[i] - xlim_left_  ) / xstep),
           static_cast<int>((y[i] - ylim_bottom_) / ystep)) = brush;
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
      const int n = std::min<int>(width_ - col, text.size());
      const int cut_out = -std::min(0, col);
      for (int i = cut_out; i < n; ++i) {
        At(i + col, row) = Brush("*", text[i]);
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
      const int n = std::min<int>(row + 1, text.size());
      const int cut_out = std::max(row - height_ + 1, 0);
      for (int j = cut_out; j < n; ++j) {
        At(col, row - j) = Brush("*", text[j]);
      }
    } 
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
    const auto pos_abs1 = GetAbsolutePosition(corner1);
    const auto pos_abs2 = GetAbsolutePosition(corner2);

    const int col_beg = std::min(pos_abs1.offset.GetCol(), pos_abs2.offset.GetCol());
    const int row_beg = std::min(pos_abs1.offset.GetRow(), pos_abs2.offset.GetRow());
    const int w_beg = -std::min(0, col_beg);
    const int h_beg = -std::min(0, row_beg);
    const int len_w = std::abs(pos_abs2.offset.GetCol() - pos_abs1.offset.GetCol()) + 1 - w_beg;
    const int len_h = std::abs(pos_abs2.offset.GetRow() - pos_abs1.offset.GetRow()) + 1 - h_beg;
    const int w_end = std::min(w_beg + len_w, width_);
    const int h_end = std::min(h_beg + len_h, height_);

    Subtype extracted(w_end - w_beg + 1, h_end - h_beg + 1);
    for (int i = w_beg; i < w_end; ++i) {
      for (int j = h_beg; j < h_end; ++j) {
        extracted.At(i - w_beg, j - h_beg) = this->At(i + col_beg, j + row_beg);
      }
    }
    return extracted;
  }

  Subtype& Fill(const Brush& brush) {
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        At(i, j) = brush;
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& Fill() {
    return Fill(palette_.GetBrush("Main"));
  }

  template<class U>
  Subtype& Fuse(const U& other,
                const Position& position = SouthWest,
                BlankFusion keep_blanks = KeepBlanks,
                AdjustPosition adjust = Adjust) {
    auto pos_abs = GetAbsolutePosition(position);
    if (adjust) {
      AdjustAbsolutePosition(pos_abs, other.GetWidth(), other.GetHeight(), true);
    }
    
    const auto offset = pos_abs.offset;
    const int off_c = offset.GetCol();
    const int off_r = offset.GetRow();
    const int col_beg = std::max(0, -off_c);
    const int col_end = std::min(other.GetWidth(), GetWidth() - off_c);
    const int row_beg = std::max(0, -off_r);
    const int row_end = std::min(other.GetHeight(), GetHeight() - off_r);

    if (keep_blanks) {
      for (int i = col_beg; i < col_end; ++i) {
        for (int j = row_beg; j < row_end; ++j) {
          At(i + off_c, j + off_r) = other.At(i, j);
        }
      }
    } else {
      for (int i = col_beg; i < col_end; ++i) {
        for (int j = row_beg; j < row_end; ++j) {
          if (!(other.At(i, j).GetName() == "Blank")) {
            At(i + off_c, j + off_r) = other.At(i, j);
          }
        }
      }
    }
    return static_cast<Subtype&>(*this);
  }

  bool IsLike(const IPlot& other) const {
    return (width_ == other.GetWidth() && height_ == other.GetHeight());
  }

  Subtype& Move(const Offset& offset) {
    auto copy = *this;
    Clear();
    Fuse(copy, offset, DontAdjust);
    return static_cast<Subtype&>(*this);
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
    for (int i = 0; i < width_; ++i) {
      for (int j = 0; j < height_; ++j) {
        Brush& brush = At(i, j);
        if (!brush.IsGeneral()) {
          brush = palette_.GetBrush(brush.GetName());
        }
      }
    }
    return static_cast<Subtype&>(*this);
  }

  virtual std::string Serialize() const override {
    std::stringstream ss("");
    for (int j = height_ - 1; j >= 0; --j) {
      for (int i = 0; i < width_; ++i) {
        ss << At(i, j).GetValue();
      }
      ss << "\n";
    }
    return ss.str();
  }

  template<class Tx, class Ty>
  Subtype& SetAutoLimits(const std::vector<Tx>& x,
                         const std::vector<Ty>& y) {
    auto x_margin_surplus = [this](){ return std::abs((xlim_right_ - xlim_left_) * xlim_margin_); };
    auto y_margin_surplus = [this](){ return std::abs((ylim_top_ - ylim_bottom_) * ylim_margin_); };
    
    // Left and Right
    if (x.size() > 0) {
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
    }

    // Bottom and Top
    if (y.size() > 0) {
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
    return static_cast<Subtype&>(*this);
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

  Subtype& SetBrush(const std::string& name, const std::string& value) {
    palette_(name, value);
    return static_cast<Subtype&>(*this);
  }
  
  Subtype& SetBrush(const Brush& brush) {
    return SetBrush(brush.GetName(), brush.GetValue());
  }

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
    return static_cast<Subtype&>(*this);
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

  Subtype& Shift(const Offset& offset) {
    auto shifted_plot = BlankLike(static_cast<Subtype&>(*this));
    shifted_plot.Fuse(*this, offset, KeepBlanks, DontAdjust);
    *this = shifted_plot;
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

//*********************************** Bar ***********************************//

class Bar {
public:
  bool IsEmpty() const { return empty_; }
  
  // Getters
  Brush GetBrush() const { return brush_; }
  int GetColumn() const { return col_; }
  int GetHeight() const { return height_; }
  std::string GetName() const { return name_; }
  int GetWidth() const { return width_; }

  // Setters
  Bar& SetBrush(const Brush& brush) { brush_ = brush; return *this; }
  Bar& SetColumn(int col) { col_ = col; return *this; }
  Bar& SetEmpty(bool empty) { empty_ = empty; return *this; }
  Bar& SetHeight(int height) { height_ = height; return *this; }
  Bar& SetName(const std::string& name) { name_ = name; return *this; }
  Bar& SetWidth(int width) { width_ = width; return *this; }

private:
  int col_;
  bool empty_;
  int width_;
  int height_;
  std::string name_;
  Brush brush_;
};

//********************************* BarPlot *********************************//

// Forward declaration
class BarGrouper;

template<class Subtype>
class __BarPlot : public __Plot<Subtype> {
friend class BarGrouper;
public:
  __BarPlot(int width = 0, int height = 0)
      : __Plot<Subtype>(width, height) {
  }

  Subtype& DrawBar(int col, int width, int height, const Brush& brush) {
    if (width == 0) {
      return static_cast<Subtype&>(*this);
    }

    const auto brush_area = brush;
    const auto brush_top = this->palette_.GetBrush("BorderTop");

    if (width < 3) {
      for (int k = 0; k < width; ++k) {
        for (int j = 0; j < height; ++j) {
          this->At(col + k, j) = brush_area;
        }
        this->At(col + k, height) = brush_top;
      }
      return static_cast<Subtype&>(*this);
    }

    const auto brush_left = this->palette_.GetBrush("BorderLeft");
    const auto brush_right = this->palette_.GetBrush("BorderRight");

    for (int k = 0; k < width; ++k) {
      for (int j = 0; j < height + 1; ++j) {
        if (k == 0) {
          if (j != height) {
            this->At(col + k, j) = brush_left;
          }
        } else if (k == width - 1) {
          if (j != height) {
            this->At(col + k, j) = brush_right;
          }
        } else if (j == height) {
          this->At(col + k, j) = brush_top;
        } else {
          this->At(col + k, j) = brush_area;
        }
      }
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& DrawBar(const Bar& bar) {
    if (bar.IsEmpty()) {
      return static_cast<Subtype&>(*this);
    }
    return DrawBar(bar.GetColumn(), bar.GetWidth(), bar.GetHeight(), bar.GetBrush());
  }

  Subtype& DrawBar(int col, int width, int height) {
    return DrawBar(col, width, height, this->palette_.GetBrush("Area"));
  }

  Subtype& DrawBars(const std::vector<Bar>& bars) {
    for (const auto& bar : bars) {
      DrawBar(bar);
    }
    return static_cast<Subtype&>(*this);
  }  

  Subtype& DrawBarLabels(const Offset& text_offset = {0, 0}) {
    for (const auto& bar : bars_) {
      this->DrawTextCentered(bar.GetName(),
                             Offset(bar.GetColumn() + bar.GetWidth() / 2,
                                    bar.GetHeight()) + text_offset,
                             DontAdjust);
    }
    return static_cast<Subtype&>(*this);
  }

  Subtype& PlotBars(const std::vector<Bar>& bars) {
    bars_.clear();
    std::copy_if(bars.begin(), bars.end(), std::back_inserter(bars_),
                 [](const auto& b) { return !b.IsEmpty(); });
    return DrawBars(bars);
  }

  template<class Tx, class Ty>
  Subtype& PlotBars(const std::vector<Tx>& xdata,
                    const std::vector<Ty>& ydata,
                    const std::string& label = "",
                    const Brush& brush = Brush("Area", DefaultBrushArea)) {
    auto xdata_s = xdata;
    std::sort(xdata_s.begin(), xdata_s.end(), std::less<Tx>());
    std::vector<Tx> diffs(xdata_s.size());
    std::adjacent_difference(xdata_s.begin(), xdata_s.end(), diffs.begin());
    auto min_diff = *std::min_element(diffs.begin(), diffs.end());
    
    const auto minmax_x = std::minmax_element(xdata_s.begin(), xdata_s.end());
    const auto min_x = *minmax_x.first;
    const auto max_x = *minmax_x.second;
    const auto minmax_y = std::minmax_element(ydata.begin(), ydata.end());
    const auto min_y = *minmax_y.first;
    const auto max_y = *minmax_y.second;

    this->SetXlimits(min_x - min_diff, max_x + min_diff);
    this->SetYlimits(std::min(static_cast<Ty>(0), min_y), max_y * 1.05);

    const double xlim_left = this->GetXlimLeft();
    const double xlim_right = this->GetXlimRight();
    const double ylim_top = this->GetYlimTop();
    const double ylim_bottom = this->GetYlimBottom();

    const double xstep = (xlim_right - xlim_left) / this->GetWidth();
    const double ystep = (ylim_top - ylim_bottom) / this->GetHeight();
    
    const int bar_width = min_diff / xstep;
    const std::size_t n = std::min(xdata.size(), ydata.size());

    std::vector<Bar> bars;
    bars.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      bars.push_back(
        Bar{}.SetName(FormatValue(ydata[i]))
             .SetHeight((ydata[i] - ylim_bottom) / ystep)
             .SetColumn((xdata[i] - xlim_left  ) / xstep - bar_width / 2.0)
             .SetWidth(bar_width)
             .SetBrush(brush)
             .SetEmpty(false)
      );
    }
    this->metadata_.push_back(
      PlotMetadata{}.SetBrush(brush)
                    .SetLabel(label)
    );
    return PlotBars(std::move(bars));
  }

  template<class Tx, class Ty>
  Subtype& PlotBars(const std::map<Tx, Ty>& data,
                    const std::string& label = "",
                    const Brush& brush = Brush("Area", DefaultBrushArea)) {
    std::vector<Tx> xdata;
    std::vector<Ty> ydata;
    xdata.reserve(data.size());
    ydata.reserve(data.size());

    for (auto &[x, y] : data) {
      xdata.push_back(x);
      ydata.push_back(y);
    }

    return PlotBars(xdata, ydata, label, brush);

  }
  template<class Ty>
  Subtype& PlotBars(const std::vector<Ty>& ydata,
                    const std::string& label = "",
                    const Brush& brush = Brush("Area", DefaultBrushArea)) {
    auto xdata = std::vector<Ty>(ydata.size());
    std::iota(xdata.begin(), xdata.end(), 1);
    return PlotBars(xdata, ydata, label, brush);
  }

protected:
  std::vector<Bar> bars_;
};

class BarPlot final : public __BarPlot<BarPlot> { using __BarPlot::__BarPlot; };

//******************************* BarGrouper ********************************//

class BarGrouper {
public:
  BarGrouper(BarPlot& baseplot, std::vector<Brush> brushes = kSymbolBrushes)
      : baseplot_(baseplot)
      , group_size_(0)
      , ngroups_(0)
      , brushes_(brushes)
      , brush_index_(0)
      , group_names_(true) {
  }

  template<class Ty>
  BarGrouper& Add(const std::vector<Ty>& ydata,
                   const std::string& label,
                   const Brush& brush) {
    if ((group_size_ + 1) * ngroups_ - 1 <= baseplot_.GetWidth()) {
      ++group_size_;
    } else {
      return *this;
    }
    
    ngroups_ = std::max<int>(ngroups_, ydata.size());
    std::vector<double> ydata_double(ngroups_);
    std::transform(ydata.begin(), ydata.begin() + ngroups_, ydata_double.begin(),
                   [](const auto& y) -> double { return y; });

    auto minmax = std::minmax_element(ydata_double.begin(), ydata_double.end());
    const double current_min = *minmax.first;
    const double current_max = *minmax.second;
    baseplot_.SetYlimits(std::min(baseplot_.GetYlimBottom(), current_min),
                         std::max(baseplot_.GetYlimTop(), current_max));

    metadata_.push_back(
      BarPlotMetadata{}.SetLabel(label)
                       .SetBrush(brush)
                       .SetLength(ngroups_)
                       .SetBarYdata(std::move(ydata_double))
                       .SetInteger(std::is_integral<Ty>::value)
    );

    baseplot_.metadata_.push_back(
      PlotMetadata{}.SetBrush(brush)
                    .SetLabel(label)
    );
    return *this;
  }

  template<class Ty>
  BarGrouper& Add(const std::vector<Ty>& ydata,
                   const std::string& label) {
    return Add(ydata, label, brushes_[brush_index_++ % brushes_.size()]); 
  }

  BarPlot& Commit(double height_resize = 0.8) {
    const int n_bars = ngroups_ * group_size_ + (ngroups_ - 1);
    const int width = baseplot_.GetWidth() / n_bars;
    const double ylim_top = baseplot_.GetYlimTop();
    const double ylim_bottom = baseplot_.GetYlimBottom();
    const double ystep = (ylim_top - ylim_bottom) / baseplot_.GetHeight();

    auto to_height = [=](double y) -> int { return (y - ylim_bottom) / ystep; };

    std::vector<Bar> bars;
    int current_col = 0;
    for (int i = 0; i < ngroups_; ++i) {
      for (int j = 0; j < group_size_; ++j) {
        std::string name;
        if (metadata_[j].is_integer) {
          name = FormatValue(static_cast<long int>(metadata_[j].ydata[i]));
        } else {
          name = FormatValue(metadata_[j].ydata[i]);
        }
        bars.push_back(
          Bar{}.SetName(name)
               .SetHeight(to_height(metadata_[j].ydata[i]) * height_resize)
               .SetBrush(metadata_[j].brush)
               .SetColumn(current_col)
               .SetWidth(width)
        );
        current_col += width;
      }
      if (i != ngroups_ - 1) {
        bars.push_back(Bar{}.SetEmpty(true));
        current_col += width;
      }
    }

    if (group_names_) {
    }

    return baseplot_.PlotBars(std::move(bars));
  }

  BarGrouper& GroupNames(bool on) {
    group_names_ = on;
    return *this;
  }

private:
  BarPlot& baseplot_;
  int group_size_;
  int ngroups_;
  std::vector<std::string> xdata_;
  std::vector<BarPlotMetadata> metadata_;
  std::vector<Brush> brushes_;
  int brush_index_;
  bool group_names_;
};

//******************************** HistPlot *********************************//

template<class Subtype>
class __HistPlot : public __BarPlot<Subtype> { 
public:
  __HistPlot(int width = 0, int height = 0)
      : __BarPlot<Subtype>(width, height) {
    nbins_ = this->GetWidth();
  }

  template<class T>
  Subtype& PlotHistogram(const std::vector<T>& data,
                         const std::string& label,
                         double height_resize = 0.8) {
    static_assert(std::is_arithmetic<T>::value,
      "PlotHistogram only supports vectors of arithmetic types.");

    const int distinct = std::set<T>(data.begin(), data.end()).size();
    nbins_ = std::min(nbins_, distinct);

    auto minmax = std::minmax_element(data.begin(), data.end());
    const T min = *minmax.first;
    const T max = *minmax.second;
    const T step = (max - min) / (nbins_ - 1);
    this->SetXlimits(min - step / 2, max + step / 2);

    std::vector<int> bar_counts_;
    bar_counts_.resize(nbins_);
    for (const auto& i : data) {
      int idx = (i - this->GetXlimLeft()) / step;
      ++bar_counts_[idx];
    }

    const int max_bar_height = *std::max_element(bar_counts_.begin(), bar_counts_.end());
    std::vector<int> bar_heights_ = bar_counts_;
    const double factor = std::min(1.0, height_resize);
    for (auto& i : bar_heights_) {
      i = i / static_cast<double>(max_bar_height) * this->GetHeight() * factor;
    }
    const auto brush = this->palette_.GetBrush("Area");
    const int bin_width = this->GetWidth() / nbins_;

    std::vector<Bar> bars;
    bars.resize(nbins_);
    for (int i = 0; i < nbins_; ++i) {
      bars.push_back(
        Bar{}.SetName(FormatValue(bar_heights_[i]))
             .SetHeight(bar_heights_[i])
             .SetBrush(brush)
             .SetColumn(i * bin_width)
             .SetWidth(bin_width)
      );
    }
    this->metadata_.push_back(
      PlotMetadata{}.SetBrush(brush)
                    .SetLabel(label)
    );
    
    return this->PlotBars(bars);
  }
  
protected:
  int nbins_;
};

class HistPlot final : public __HistPlot<HistPlot> { using __HistPlot::__HistPlot; };

//******************************** GridPlot *********************************//

// Forward declaration
template<class T>
class RowMajorGridSetter;

// Forward declaration
template<class T>
class ColumnMajorGridSetter;

template<class Subtype>
class __GridPlot : public __Plot<Subtype> {
public:
  __GridPlot(int grid_rows, int grid_cols,
             int width = 0, int height = 0)
      : __Plot<Subtype>(width, height) {
    grid_rows_ = std::max(0, grid_rows);
    grid_cols_ = std::max(0, grid_cols);
    nplots_ = grid_rows_ * grid_cols_;
    plots_.resize(nplots_, nullptr);

    subp_widths_.resize(grid_cols_);
    subp_heights_.resize(grid_rows_);
    const int div_w = this->width_ / grid_cols_;
    const int div_h = this->height_ / grid_rows_;
    const int rem_w = this->width_ % grid_cols_;
    const int rem_h = this->height_ % grid_rows_;
    for (int i = 0; i < grid_cols_; ++i) {
      subp_widths_[i] = div_w + ((i < rem_w) ? 1 : 0);
    }
    for (int i = 0; i < grid_rows; ++i) {
      subp_heights_[i] = div_h + ((i < rem_h) ? 1 : 0);
    }
  }

  __GridPlot(std::vector<int> subplot_widths, std::vector<int> subplot_heights,
             int width = 0, int height = 0)
      : __Plot<Subtype>(width, height)
      , subp_widths_(subplot_widths)
      , subp_heights_(subplot_heights) {
    int ww = std::accumulate(subp_widths_.begin(), subp_widths_.end(), 0);
    int hh = std::accumulate(subp_heights_.begin(), subp_heights_.end(), 0);
    grid_rows_ = subp_heights_.size();
    grid_cols_ = subp_widths_.size();
    if (ww != this->width_ || hh != this->height_) {
      __GridPlot(grid_rows_, grid_cols_, width, height);
    } else {
      nplots_ = grid_rows_ * grid_cols_;
      plots_.resize(nplots_, nullptr);
    }
  }

  virtual Brush& At(int col, int row) override {
    return const_cast<Brush&>(
      static_cast<const Subtype*>(this)->At(col, row)
    );
  }

  virtual const Brush& At(int col, int row) const override {
    int ii = 0, i;
    for (i = 0; i < grid_cols_; ++i) {
      if (ii + subp_widths_[i] > col) {
        break;
      }
      ii += subp_widths_[i];
    }
    
    int jj = 0, j;
    for (j = grid_rows_ - 1; j >= 0; --j) {
      if (jj + subp_heights_[j] > row) {
        break;
      }
      jj += subp_heights_[j];
    }

    if (plots_[i + j * grid_cols_] == nullptr) {
      return this->canvas_[row + col * this->height_];
    }
    return plots_[i + j * grid_cols_]->At(col - ii, row - jj);
  }

  template<class T>
  T& Get(int grid_row, int grid_col) {
    return *dynamic_cast<T*>(plots_[grid_row * grid_cols_ + grid_col]);
  }

  template<class T>
  const T& Get(int grid_row, int grid_col) const {
    return *dynamic_cast<T*>(plots_[grid_row * grid_cols_ + grid_col]);
  }

  int GetGridRows() const {
    return grid_rows_;
  }

  int GetGridColumns() const {
    return grid_cols_;
  }

  ColumnMajorGridSetter<Subtype> SetInColumnMajor() {
    return ColumnMajorGridSetter<Subtype>(static_cast<Subtype&>(*this));
  }

  RowMajorGridSetter<Subtype> SetInRowMajor() {
    return RowMajorGridSetter<Subtype>(static_cast<Subtype&>(*this));
  }

  Subtype& SetPlotAt(int grid_row, int grid_col, IPlot& plot) {
    plots_[grid_row * grid_cols_ + grid_col] = &plot;
    return static_cast<Subtype&>(*this);
  }

protected:
  int nplots_;
  int grid_rows_;
  int grid_cols_;
  std::vector<IPlot*> plots_;
  std::vector<int> subp_widths_;
  std::vector<int> subp_heights_;
};

class GridPlot final : public __GridPlot<GridPlot> { using __GridPlot::__GridPlot; };

//***************************** RowMajorSetter ******************************//

template<class T>
class RowMajorGridSetter {
public:
  RowMajorGridSetter(T& baseplot)
      : baseplot_(baseplot)
      , idx_(0)
      , grid_cols_(baseplot_.GetGridColumns())
      , nplots_(baseplot_.GetGridRows() * grid_cols_) {
    static_assert(std::is_base_of<__GridPlot<T>, T>::value,
      "Template type T must be a subtype of __GridPlot<T>.");
  }

  RowMajorGridSetter& operator()(IPlot& plot) {
    if (idx_ < nplots_) {
      baseplot_.SetPlotAt(idx_ / grid_cols_, idx_ % grid_cols_, plot);
      ++idx_;
    }
    return *this;
  }

  T& Set() {
    return baseplot_;
  }
  
private:
  T& baseplot_;
  int idx_;
  int grid_cols_;
  int nplots_;
};

//************************** ColumnMajorGridSetter **************************//

template<class T>
class ColumnMajorGridSetter {
public:
  ColumnMajorGridSetter(T& baseplot)
      : baseplot_(baseplot)
      , idx_(0)
      , grid_rows_(baseplot_.GetGridRows())
      , nplots_(grid_rows_ * baseplot_.GetGridColumns()) {
    static_assert(std::is_base_of<__GridPlot<T>, T>::value,
      "Template type T must be a subtype of __GridPlot<T>.");
  }

  ColumnMajorGridSetter& operator()(IPlot& plot) {
    if (idx_ < nplots_) {
      baseplot_.SetPlotAt(idx_ % grid_rows_, idx_ / grid_rows_, plot);
      ++idx_;
    }
    return *this;
  }

  T& Set() {
    return baseplot_;
  }
  
private:
  T& baseplot_;
  int idx_;
  int grid_rows_;
  int nplots_;
};

//***************************** Free functions ******************************//

template<class T>
T BlankLike(const T& plot) {
  static_assert(std::is_base_of<__Plot<T>, T>::value,
    "Template type T must be a subtype of __Plot<T>.");
  return T(plot.GetWidth(), plot.GetHeight());
}

std::vector<Brush> StringToBrushes(const std::string& str) {
  std::vector<Brush> brushes;
  for (auto c : str) {
    brushes.push_back(Brush("*", c));
  }
  return brushes;
}

std::vector<Brush> StringToBrushes(const char *str) {
  return StringToBrushes(std::string(str));
}

} // namespace askiplot

#endif // ASKIPLOT_HPP_
