# AskiPlot

AskiPlot is a lightweight tool+library written in C++14 for creating plots by means of ASCII characters only.
Users can plot CSV streams from the standard input or files.
The library makes extensive use of [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) (Curiously Recurring Template Pattern) and is designed to have a [Fluent interface](https://en.wikipedia.org/wiki/Fluent_interface).

The initial idea was to allow user to write plots with chained member functions, this natually led to
CRTP and static polymorphism as concepts embracing most of the classes.

[Learn by example](examples)

## Getting started

### Cloning
The AskiPlot header-only libary comes with a **command-line tool** for quick plots.
If you want to use this tool please clone this repository with the submodules since
the command-line parsing utility is provided by the external tool [cxxopts](https://github.com/jarro2783/cxxopts).
```
git clone --recurse-submodules https://github.com/fsossai/AskiPlot
```
Otherwise, if you only want to use the **library**, there is no need for external dependencies:
```
git clone https://github.com/fsossai/AskiPlot
```
or directly download the C++ header file:
```
wget https://github.com/fsossai/AskiPlot/raw/master/include/AskiPlot/AskiPlot.hpp
```
### Compiling
Compilation is required only for being able to the command-line tool.
```
cd AskiPlot
mkdir build && cd build
cmake .. -B.
make
```

## Features / Implementation status

- [x] Plotting points
- [x] Histograms
- [x] Bar plots
- [x] Grids (some bugs yet to be fixed)
- [ ] Input from standard input
- [ ] Input from CSV file
- [ ] Input from binary file
- [x] Drawing images
