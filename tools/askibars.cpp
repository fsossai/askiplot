// This program parses a text file containing numerical data in one of two
// formats:
//
// 1. Single-column format:
//    - Each non-empty line contains a single number.
//    - The first column (`x`) is automatically generated as a sequence of
// progressive integers starting from zero.
//    - The second column (`y`) contains the parsed numbers from the file.
//
// 2. Two-column format:
//    - Each non-empty line contains two numbers separated by a comma, tab, or
// space.
//    - The first column (`x`) and the second column (`y`) are populated with
// the parsed numbers from each line.
//
// Parsing rules:
// - Columns are separated by any combination of commas, tabs, or spaces.
// - Empty lines are ignored.
// - Lines with more than two columns are considered invalid and will cause an
// error.
// - The parsed data is stored in two vectors: `x` (first column) and `y`
// (second column).
//
// clang-format off
// $ cat a.txt
// 1
// 2
// 3
// 4
// 5
// 6
// 5
// 4
// 3
// 2
// 1
// 2
// 3
// 4
// 3
// 2
// 1
// 2
// 1
// $ cat a.txt | askibars.out
//                           ___                                                                              
//                          |###|                                                                             
//                          |###|                                                                             
//                          |###|                                                                             
//                      ___ |###| ___                                                                         
//                     |###||###||###|                                                                        
//                     |###||###||###|                                                                        
//                     |###||###||###|                                                                        
//                 ___ |###||###||###| ___                           ___                                      
//                |###||###||###||###||###|                         |###|                                     
//                |###||###||###||###||###|                         |###|                                     
//                |###||###||###||###||###|                         |###|                                     
//            ___ |###||###||###||###||###| ___                 ___ |###| ___                                 
//           |###||###||###||###||###||###||###|               |###||###||###|                                
//           |###||###||###||###||###||###||###|               |###||###||###|                                
//           |###||###||###||###||###||###||###|               |###||###||###|                                
//       ___ |###||###||###||###||###||###||###| ___       ___ |###||###||###| ___       ___                  
//      |###||###||###||###||###||###||###||###||###|     |###||###||###||###||###|     |###|                 
//      |###||###||###||###||###||###||###||###||###|     |###||###||###||###||###|     |###|                 
//      |###||###||###||###||###||###||###||###||###|     |###||###||###||###||###|     |###|                 
//  ___ |###||###||###||###||###||###||###||###||###| ___ |###||###||###||###||###| ___ |###| ___             
// |###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###|            
// |###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###|            
// |###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###|            
// |###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###||###|
// clang-format on

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "askiplot.hpp"

using namespace std;

void trim(string &s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  size_t end = s.find_last_not_of(" \t\r\n");
  if (start == string::npos) {
    s.clear();
  } else {
    s = s.substr(start, end - start + 1);
  }
}

void split(const string &line, vector<string> &tokens) {
  tokens.clear();
  string token;
  for (size_t i = 0; i < line.size(); ++i) {
    char c = line[i];
    if (c == ',' || c == '\t' || isspace(static_cast<unsigned char>(c))) {
      if (!token.empty()) {
        tokens.push_back(token);
        token.clear();
      }
    } else {
      token += c;
    }
  }
  if (!token.empty())
    tokens.push_back(token);
}

int main(int argc, char *argv[]) {
  istream *in = &cin;
  ifstream file;
  if (argc > 1) {
    file.open(argv[1]);
    if (!file) {
      cerr << "Error: Cannot open file " << argv[1] << endl;
      return 1;
    }
    in = &file;
  }

  vector<double> x, y;
  string line;
  int row = 0;
  while (getline(*in, line)) {
    trim(line);
    if (line.empty())
      continue;
    vector<string> tokens;
    split(line, tokens);
    if (tokens.size() == 1) {
      try {
        x.push_back(row);
        y.push_back(stod(tokens[0]));
        ++row;
      } catch (...) {
        cerr << "Error: Invalid number in line: " << line << endl;
        return 1;
      }
    } else if (tokens.size() == 2) {
      try {
        x.push_back(stod(tokens[0]));
        y.push_back(stod(tokens[1]));
      } catch (...) {
        cerr << "Error: Invalid number in line: " << line << endl;
        return 1;
      }
    } else {
      cerr << "Error: Invalid line (expect 1 or 2 columns): " << line << endl;
      return 1;
    }
  }

  askiplot::BarPlot bp;
  cout << bp.PlotBars(x, y, "data").Serialize() << "\n";

  return 0;
}
