# AskiPlot

```C++
cout <<
  Plot(80,20)
    .DrawImage(Image("examples/logo.bmp").Invert(), TextGamma("askiplot"))
    .Serialize();
```

```
              a                                   s                             
           kiplo                        ta    skiplotas                         
          kiplot           aski        plot  as      kip  lota                  
         sk iplo           tas         kipl ot  ask   ip  lota             skip 
        lo  tas            kip          lotas  kipl   ot  ask              ipl  
       ot   ask           iplo            tas  kipl   ot  ask              ipl  
       ot  aski    plot   aski   pl    otaski  plo   tas kipl     otask   iplot 
  askiplo  task   iplot   ask   ip    lota sk  ipl   ota skip    lot as   kipl  
 ota ski plota    skipl   ota sk      iplo    task  ipl  ota    ski  plo  tas   
kip  lo    tas    kiplo   taskiplo    tas     kipl ota   ski   plot  askiplot   
 as kip    lot   a skip  lotaskipl    ota     skiplo     tas   kip   lot aski   
    pl    otas   k iplo  task  iplo   tas     kip       lota  skip   lo  task   
   ipl    ota   s   kip  lot   aski  plot    aski       plo   task   ip  lota   
   ski    plo  tas  kip lota   ski   plo     task       ipl   otas   ki  plo    
   ta     ski  p   lotaskipl   ota  skipl    ota        ski  plota  sk   ipl    
  ota    skip  lotaskip lota   skiplotaski   plo        taskiplotaski    plot   
  ask    iplo   taski   plo     task  ipl    ota        skipl  otask     iplo   
  tas                                                                           
  kip                                                                           
```

AskiPlot is a single-header library written in C++17 for creating plots with ASCII characters only.
With AskiPlot you can plot grouped bars, histograms, boxes, lines and arrange text by specifying positions
relative to the current console.
Unless otherwise specified, the library will infere a canvas size that fits the console at run-time.

The library is designed for method chainining. All classes have a [fluent interface](https://en.wikipedia.org/wiki/Fluent_interface).
Static polymorhism is achieved through the use of the [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)
(Curiously Recurring Template Pattern).

## Compiling

Run `make` from the [tools](tools) or [examples](examples) directory to compile.

### Grouped bars

[bar_grouper.cpp](examples/bar_grouper.cpp) uses the class `BarGrouper` to group multiple sources in a single `BarPlot`.
```C++
BarPlot bp;
BarGrouper(bp, askiplot::kSymbolBrushes)
  .Add(vector<int>{80, 40}, Scaled, "Data Source 1")
  .Add(vector<int>{20, 50}, Scaled, "Data Source 2", Brush('x'))
  .Add(vector<int>{30, 15}, NotScaled, "Data Source 3 (not scaled)")
  .SetGroupNames({"Group 1", "Group 2"})
  .ShowGroupNames(true)
  .Commit();
bp
  .DrawBarLabels(Offset(0, 1))
  .DrawLegend()
  .SetBrush("BorderTop", "/")
  .DrawBorders(Top + Right)
;
cout << bp.Serialize();
```
Produces the following:
```
////////////////////////////////////////////////////////////////////////////////
                                                | @ Data Source 1              |
    80                    30                    | x Data Source 2              |
 _________             _________                | $ Data Source 3 (not scaled) |
|@@@@@@@@@|           |$$$$$$$$$|               |______________________________|
|@@@@@@@@@|           |$$$$$$$$$|                                              |
|@@@@@@@@@|           |$$$$$$$$$|                                              |
|@@@@@@@@@|           |$$$$$$$$$|                                              |
|@@@@@@@@@|           |$$$$$$$$$|                                              |
|@@@@@@@@@|           |$$$$$$$$$|                                              |
|@@@@@@@@@|           |$$$$$$$$$|                          50                  |
|@@@@@@@@@|           |$$$$$$$$$|                       _________              |
|@@@@@@@@@|           |$$$$$$$$$|               40     |xxxxxxxxx|    15       |
|@@@@@@@@@|           |$$$$$$$$$|            _________ |xxxxxxxxx| _________   |
|@@@@@@@@@|           |$$$$$$$$$|           |@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|  |
|@@@@@@@@@|           |$$$$$$$$$|           |@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|  |
|@@@@@@@@@|           |$$$$$$$$$|           |@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|  |
|@@@@@@@@@|    20     |$$$$$$$$$|           |@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|  |
|@@@@@@@@@| _________ |$$$$$$$$$|           |@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|  |
|@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|           |@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|  |
|@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|           |@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|  |
|@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|           |@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|  |
|@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|           |@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|  |
|@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|           |@@@@@@@@@||xxxxxxxxx||$$$$$$$$$|  |
<___________ Group 1 ___________>           <___________ Group 2 ___________>  |
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
### Images

[turing.cpp](examples/turing.cpp) shows that plotting BMP images is as simple as:
```C++
cout << Plot{}.DrawImage(Image("turing.bmp")).Serialize();
```

```
@@@@@@@@@@@@@@@@@@@@@@@@#######0000oo............... ...o0##@##########00000000000
@@@@@@@@@@@@@@@@@@@@@@@@####0o..oo........       .       ..0#########0000000000000
@@@@@@@@@@@@@@@@@@@@@@@#@#0.  ...     ..                    o#########000000000000
@@@@@@@@@@@@@@@@@@@@@@@##o          .....................   .0#######0000000000000
@@@@@@@@@@@@@@@@@@@@@@@#0   .  ..oo0000000000000000o....     .0######0000000000000
@@@@@@@@@@@@@@@@@@@@@@@#.  ..o00###@#@########000000o.        .0#####0000000000000
@@@@@@@@@@@@@@@@@@@@@@@o   o0##@@@@#############00000..        .0###00000000000000
@@@@@@@@@@@@@@@@@@@@@@#.  o#@@@@@@@@###########00000o..         o###0000000000000o
@@@@@@@@@@@@@@@@@@@@@@@o.0##@@@@##@@@######000000000o...        o###0000000000000o
@@@@@@@@@@@@@@@@@@@@@@#o0###@@@@@@@@####00000000000ooo...       o###0000000000000o
@@@@@@@@@@@@@@@@@@@@@@@#00000######000oo..oooooo0000oo.......   0###0000000000000o
@@@@@@@@@@@@@@@@@@@@####o....oo0000o..   .......ooo0ooooo.oo....0###0000000000000o
@@@@@@@@@@@@@@@@@@@@@@@#0......0##0o.............oooooooooo00oo.o0##0000000000000o
@@@@@@@@@@@@@@@@@@@######000000##00000000oooooooo0oooooooo0oo00ooo##0000000000000o
@@@@@@@@@@@@@@@@@@@@@@#@#0#######000000000000000000oooooooo.oo0oo0##0000000000000o
@@@@@@@@@@@@@@@@@@@####@########000000#######0000oooooooo0o0oooo0###0000000000000o
@@@@@@@@@@@@@@@@@######@####0ooo..ooo0000###00000ooooooo0o00ooo0####0000000000000o
@@@@@@@@@@@@@@@@@@###@@@#000000ooooo0000000000ooooooooooo000o0#####00000000000000o
@@@@@@#@@@@@@@@@@@#@##@@@#00000000000000000oooooooooooooooo0#######0000000000000oo
@@@@@@@@@@@@@@@@@@@@@#####00oo0ooooooo..oooooooooooooooo..0#######00000000000000oo
@@@@@@@@@@@@@@@@@@@@@@@@@#00000ooooooooooooooooooooooo...o0#######00000000000000oo
@@@@@@@@@@@@@@@@@@@@@@@@@@#0##00ooooo0000ooooooooooo....o..o#####000000000000000oo
@@@@@@@@@@@@@@@@@@@@@@@@@@@000000000000ooo....oo.......ooo...0###0000000000000000o
#@@@@@@@@@@@@@@@@@@@@@@@@@@#000000ooooo........... ...oooo....o00000000000000000oo
@@@@@@@@@@@@@@@@@@@@@@@@@@@@##0oo.....   ..      ...ooo0o.........ooooo000000000oo
@@@@@@@@@@@@@@@@@@@@@@@@##00000o0o0oo..     .....ooo000o.... ............ooooooooo
@@@@@@@@@@@@@@@@@###000ooooo0oo.00o000oo......ooo000000... .......................
@@@@@@@@@@@@#0000oooooooooooooo.o##0oo00oooooo00000000..............  ............
@@@@@@###00oooooooo.ooooo0ooooo..0#@##000000#0##0000o........ .... ... ...........
@@@@#0o.oooooooo....oooooooooo...o#@@@#...0####00000o................. ...........
@@@#oo..ooooo........oooooooooo.o##@@#..   0000###0o................  ............
@@#oo...oooo......oooooooooooo..#@@@0.. ....0##0o.................  ........... ..
@#0oo....oo.....oooooooooooooo. oooo.   .0#0000o0o.....o..........................
0o.oo....oo....ooooooooooooooo...o0...  .0000#@@0o..........o................. . .
ooo.....oo......ooooooooooooo..o00o      .00##@0o................... .......   ...
..ooo...oo....o.ooooooooooooo...00o       o0#@0oo.................. .......    ...
o...o..oooo..oo.ooooooooooooo.. o0.        0##......oo............ ........   ...

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

[fusion.cpp](examples/fusion.cpp) demonstrates how to fuse (compose) any plot together.
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

## Brushes

- **Main**: generic drawing pen (used by lines/points). Default: "_"
- **Blank**: background/clears. Default: " "
- **Area**: fills for bars/boxes/hist bins. Default: "#"
- **LineHorizontal**: horizontal helper lines. Default: "-"
- **LineVertical**: vertical helper lines. Default: "|"
- **BorderTop**/**BorderBottom**: top/bottom frame. Default: "_"
- **BorderLeft**/**BorderRight**: left/right frame. Default "|"

## Build on AskiPlot

- [askibench](https://github.com/fsossai/askibench): plotting benchmark results with grouped bars.
