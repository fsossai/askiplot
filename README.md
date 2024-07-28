# AskiPlot

AskiPlot is a single-header library written in C++17 for creating plots with ASCII characters only.
The library encourage chaining programming as all API classes have a [fluent interface](https://en.wikipedia.org/wiki/Fluent_interface).
Static polymorhism is achieved through the use of the [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)
(Curiously Recurring Template Pattern).

With AskiPlot you can plot grouped bars, histograms, boxes, lines and arrange text by specifying positions
relative to the current console.
Unless otherwise specified, the library will infere a canvas size that fits the console at run-time.

## Usage examples

Run `make` in the [examples](examples) directory to compile all the examples.

### Grouped bars

[groupedbars.cpp](examples/groupedbars.cpp) uses the class `GroupedBars` to group multiple sources in a single `BarPlot`.
```C++
BarPlot bp;
auto gb = GroupedBars<BarPlot>(bp, 2);
gb
  (vector<int>{80,40}, "Data Source 1", Brush('@'))
  (vector<int>{20,50}, "Data Source 2", Brush('$'))
  (vector<int>{10,20}, "Data Source 3", Brush('.'))
  .Plot()
;
bp
  .DrawBarLabels(Offset(0,1))
  .DrawLegend()
  .SetBrush("BorderTop", "/")
  .DrawBorders(Top + Right)
;
cout << bp.Serialize();
```
Produces the following:
```
////////////////////////////////////////////////////////////////////////////////
                                                            ___________________|
                                                            | @ Data Source 1 ||
    80                                                      | $ Data Source 2 ||
 ________                                                   | . Data Source 3 ||
|@@@@@@@@|                                                  |_________________||
|@@@@@@@@|                                                                     |
|@@@@@@@@|                                                                     |
|@@@@@@@@|                                                                     |
|@@@@@@@@|                                                                     |
|@@@@@@@@|                                            50                       |
|@@@@@@@@|                                         ________                    |
|@@@@@@@@|                                        |$$$$$$$$|                   |
|@@@@@@@@|                                  40    |$$$$$$$$|                   |
|@@@@@@@@|                               ________ |$$$$$$$$|                   |
|@@@@@@@@|                              |@@@@@@@@||$$$$$$$$|                   |
|@@@@@@@@|                              |@@@@@@@@||$$$$$$$$|                   |
|@@@@@@@@|    20                        |@@@@@@@@||$$$$$$$$|    20             |
|@@@@@@@@| ________                     |@@@@@@@@||$$$$$$$$| ________          |
|@@@@@@@@||$$$$$$$$|                    |@@@@@@@@||$$$$$$$$||........|         |
|@@@@@@@@||$$$$$$$$|    10              |@@@@@@@@||$$$$$$$$||........|         |
|@@@@@@@@||$$$$$$$$| ________           |@@@@@@@@||$$$$$$$$||........|         |
|@@@@@@@@||$$$$$$$$||........|          |@@@@@@@@||$$$$$$$$||........|         |
```

### Histograms

[gaussian.cpp](examples/gaussian.cpp) uses the class `HistPlot` to plot a standard normal distribution.
```C++
HistPlot hp;
hp
  .DrawBorders(All)
  .SetTitle("Gaussian distribution")
  .DrawTitle()
  .SetBrush("Area", "@")
  .SetBrush("BorderTop", " ")
  .PlotHistogram(gsamples, "Normal (0,1)")
  .DrawText("Number of samples: " + to_string(N), NorthWest + Offset(2, -2))
  .DrawLegend()
;
cout << hp.Serialize();
```
Produces the following:
```
______________________________Gaussian distribution_____________________________
|                                                            __________________|
| Number of samples: 10000                                   | @ Normal (0,1) ||
|                                                            |________________||
|                                                                              |
|                                      @                                       |
|                                      @  @                                    |
|                                  @  @@@@@                                    |
|                                  @ @@@@@@@ @                                 |
|                                 @@@@@@@@@@@@                                 |
|                                 @@@@@@@@@@@@                                 |
|                               @ @@@@@@@@@@@@@@                               |
|                              @@@@@@@@@@@@@@@@@                               |
|                             @@@@@@@@@@@@@@@@@@@@                             |
|                            @@@@@@@@@@@@@@@@@@@@@@                            |
|                           @@@@@@@@@@@@@@@@@@@@@@@@@@                         |
|                          @@@@@@@@@@@@@@@@@@@@@@@@@@@                         |
|                          @@@@@@@@@@@@@@@@@@@@@@@@@@@@                        |
|                       @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                       |
|                     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                     |
|                    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                     |
|                  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                   |
                @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ @              
```
### Text and lines
[textlines.cpp](examples/textlines.cpp) demonstrates how to draw horizontal or vertical text and lines.
At every cardinal point, text is adjusted so to fit the console's boundaries.

```C++
Plot p;
p
 .SetBrush("LineVertical", DefaultBrushLineVertical) // Optional
 .DrawLineVerticalAtCol(13)
 .DrawLineVerticalAtCol(15)
 .SetBrush("LineVertical", "!")
 .DrawLineVerticalAtCol(0.5)
 .SetBrush("LineHorizontal", ".")
 .DrawLineHorizontalAtRow(p.GetHeight() - 2)
 .DrawLineHorizontalAtRow(1)
 .DrawText("North", North)
 .DrawText("South", South)
 .DrawText("East", East)
 .DrawText("West", West)
 .DrawText("NorthEast", NorthEast)
 .DrawText("NorthWest", NorthWest)
 .DrawText("SouthEast", SouthEast)
 .DrawText("SouthWest", SouthWest)
 .DrawText("Center", Center)
 .DrawTextCentered("Centered text at South + Offset(0,2)", South + Offset(0,2))
 .DrawTextCentered("Centered text at South", South)
 .SetBrush("LineHorizontal", ">")
 .DrawLineHorizontalAtRow(0.66)
 .SetBrush("LineHorizontal", "<")
 .DrawLineHorizontalAtRow(0.33)
 .DrawTextVerticalCentered("Vertical text", East - Offset(10,0))
 .DrawText("{3,3}", {3,3})
;
cout << p.Serialize();
```
Produces the following:
```
NorthWest    | |                        North                          NorthEast
................................................................................
             | |                        !                                       
             | |                        !                                       
             | |                        !                                       
             | |                        !                            V          
             | |                        !                            e          
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>r>>>>>>>>>>
             | |                        !                            t          
             | |                        !                            i          
             | |                        !                            c          
West         | |                        Center                       a      East
             | |                        !                            l          
             | |                        !                                       
             | |                        !                            t          
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<e<<<<<<<<<<
             | |                        !                            x          
             | |                        !                            t          
             | |                        !                                       
   {3,3}     | |                        !                                       
             | |      Centered text at South + Offset(0,2)                      
................................................................................
SouthWest    | |             Centered text at South                    SouthEast
```

### Fusion

[fusion.cpp](examples/fusion.cpp) demostrates the usefulness of the `Fusion` function and its flexibility given by the chained `operator()`.
```C++
auto box1 = Plot(10,5).Fill(".").DrawTextCentered("BOX1", Center);
auto box2 = Plot(box1).DrawTextCentered("BOX2", Center);

Plot p(60,15);
p
  .Fuse(box1, NorthWest)
  .Fuse(box1, SouthEast)
  .Fuse(box2, NorthEast)
  .Fuse(box2, SouthWest)
  .DrawLineHorizontalAtRow(0.5)
  .DrawLineVerticalAtCol(0.5)
;
cout << p.Serialize();
```
Produces the following:
```
..........                    |                   ..........
..........                    |                   ..........
...BOX1...                    |                   ...BOX2...
..........                    |                   ..........
..........                    |                   ..........
                              |
                              |
------------------------------|-----------------------------
                              |
                              |
..........                    |                   ..........
..........                    |                   ..........
...BOX2...                    |                   ...BOX1...
..........                    |                   ..........
..........                    |                   ..........
```

### Grid
[grid.cpp](examples/grid.cpp) shows how to merge multiple plots into a simple grid.
```C++
int width = 10, height = 5;
GridPlot gp(2, 3, 3 * width, 2 * height); // Rows, Columns, Width, Height
Plot base = Plot(width, height).Fill().DrawBorders(All - Bottom);

Plot sp1 = Plot(base).SetMainBrush("1").Redraw();
Plot sp2 = Plot(base).SetMainBrush("2").Redraw();
Plot sp3 = Plot(base).SetMainBrush("3").Redraw();

gp.SetInRowMajor()(sp1)(sp2)(sp3)(sp3)(sp2)(sp1).Set();
gp.Get<Plot>(1,2).DrawTextCentered("--", Center);

cout << gp.Serialize();
```
Produces the following:
```
______________________________
|11111111||22222222||33333333|
|111--111||22222222||33333333|
|11111111||22222222||33333333|
|11111111||22222222||33333333|
______________________________
|33333333||22222222||11111111|
|33333333||22222222||111--111|
|33333333||22222222||11111111|
|33333333||22222222||11111111|
```
