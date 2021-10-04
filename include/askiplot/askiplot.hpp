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
  Plot(std::string label = "", int height = 0, int width = 0) 
      : label_(label) {
    if (height < 0 || width < 0) {
      throw InvalidPlotSize();
    }
    if (height == 0 || width == 0) {
      struct winsize w;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
      if (height == 0) height_ = w.ws_row;
      if (width == 0) width_ = w.ws_col;
    }
    canvas_.resize(height * width);
  }

  std::string GetName() const { return name_; }
  std::string GetLabel() const { return label_; }

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

  Plot& SetName(std::string name) { name_ = name; return *this; }
  Plot& SetLabel(std::string label) { label_ = label; return *this; }
  Plot& SetPen(Pen *pen) { pen_ = pen; return *this; }
  
protected:
  std::string name_;
  std::string label_;
  int height_;
  int width_;
  std::vector<bool> canvas_;
  Pen *pen_ = nullptr;
private:
};



} // namespace askiplot

#endif // ASKIPLOT_HPP_