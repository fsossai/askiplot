# Examples

Compilation example from this directory:
```
g++ -I../include -o example example.cpp
```

## Histograms

[gaussian.cpp]() uses the class `HistPlot` to plot a standard normal distribution.
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

## Grouped bars plots

[groupedbars.cpp]() uses the class `GroupedBars` to group multiple sources in a single `BarPlot`.
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

## Text and lines
[textlines.cpp]() demonstrates how to draw horizontal or vertical text and lines. At every cardinal point, text is adjusted so to fit the console's boundaries.

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