//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2007 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#include <iostream>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/Visualization/CreateStandardColorMapsAlg.h>
#include <Core/Geom/ColorMap.h>

namespace SCIRun {

CreateStandardColorMapsAlg* get_create_standard_colormaps_alg()
{
  return new CreateStandardColorMapsAlg();
}


static int gray[2][3] = // "Gray"
  {{0, 0, 0}, {255, 255, 255}};

static int rainbow[28][3] = // "Rainbow"
  {{0, 0, 255}, {0, 52, 255},
   {1, 80, 255}, {3, 105, 255},
   {5, 132, 255}, {9, 157, 243},
   {11, 177, 213}, {15, 193, 182},
   {21, 210, 152}, {30, 225, 126},
   {42, 237, 102}, {60, 248, 82},
   {87, 255, 62}, {116, 255, 49},
   {148, 252, 37}, {178, 243, 27},
   {201, 233, 19}, {220, 220, 14},
   {236, 206, 10}, {247, 185, 8},
   {253, 171, 5}, {255, 151, 3},
   {255, 130, 2}, {255, 112, 1},
   {255, 94, 0}, {255, 76, 0},
   {255, 55, 0}, {255, 0, 0}};

static int old_rainbow[12][3] = // "Old Rainbow"
  {{0, 0, 255},   {0, 102, 255},
   {0, 204, 255},  {0, 255, 204},
   {0, 255, 102},  {0, 255, 0},
   {102, 255, 0},  {204, 255, 0},
   {255, 234, 0},  {255, 204, 0},
   {255, 102, 0},  {255, 0, 0}};

static int darkhue[19][3] = // "Darkhue" 
  {{0, 0, 0}, {0, 28, 39},
   {0, 30, 55}, {0, 15, 74},
   {1, 0, 76}, {28, 0, 84},
   {32, 0, 85}, {57, 1, 92},
   {108, 0, 114}, {135, 0, 105},
   {158, 1, 72}, {177, 1, 39},
   {220, 10, 10}, {229, 30, 1},
   {246, 72, 1}, {255, 175, 36},
   {255, 231, 68}, {251, 255, 121},
   {239, 253, 174}};

static int lighthue[19][3] = // "Lighthue"
  {{64, 64, 64}, {64, 80, 84},
   {64, 79, 92}, {64, 72, 111},
   {64, 64, 102}, {80, 64, 108},
   {80, 64, 108}, {92, 64, 110},
   {118, 64, 121}, {131, 64, 116},
   {133, 64, 100}, {152, 64, 84},
   {174, 69, 69}, {179, 79, 64},
   {189, 100, 64}, {192, 152, 82},
   {192, 179, 98}, {189, 192, 124},
   {184, 191, 151}};
	

static int blackbody[13][3] = // "Blackbody"
  {{0, 0, 0}, {52, 0, 0},
   {102, 2, 0}, {153, 18, 0},
   {200, 41, 0}, {230, 71, 0},
   {255, 120, 0}, {255, 163, 20},
   {255, 204, 55}, {255, 228, 80},
   {255, 247, 120}, {255, 255, 180},
   {255, 255, 255}};


static int don[10][3] = // "Don"
  {{0, 90, 255}, {51, 104, 255},
   {103, 117, 255}, {166, 131, 245},
   {181, 130, 216}, {192, 129, 186},
   {197, 128, 172}, {230, 126, 98},
   {240, 126, 49}, {255, 133, 0}};

static int bp_seismic[3][3] = // "BP Seismic"
  {{0, 0, 255}, {255, 255, 255}, {255, 0, 0}};

static int dark_gray[4][3] = // "Dark Gray"
  {{0, 0, 0}, {0, 0, 0},
   {128, 128, 128}, {255, 255, 255}};

static int red_tint[2][3] = // "Red Tint"
  {{20, 0, 0}, {255, 235, 235}};

static int orange_tint[2][3] = // "Orange Tint"
  {{20, 10, 0}, {255, 245, 235}};

static int yellow_tint[2][3] = // "Yellow Tint"
  {{20, 20, 0}, {255, 255, 235}};

static int green_tint[2][3] = // "Green Tint"
  {{0, 20, 0}, {235, 255, 235}};

static int cyan_tint[2][3] = // "Cyan Tint"
  {{0, 20, 20}, {235, 255, 255}};

static int blue_tint[2][3] = // "Blue Tint"
  {{0, 0, 20}, {235, 235, 255}};

static int purple_tint[2][3] = // "Purple Tint"
  {{10, 0, 20}, {245, 235, 255}};

static int pink_white_blue[29][3] = // "Pink,White,Blue"
  {{204, 0, 153}, {206, 15, 159},
   {208, 32, 164}, {210, 48, 170},
   {211, 65, 175}, {213, 81, 181},
   {215, 98, 186}, {217, 114, 192},
   {219, 131, 197}, {221, 147, 203},
   {223, 164, 208}, {224, 180, 214},
   {226, 196, 219}, {228, 213, 225},
   {255, 255, 255}, {213, 213, 255},
   {196, 196, 255}, {180, 180, 255},
   {164, 164, 255}, {147, 147, 255},
   {131, 131, 255}, {114, 114, 255},
   {98, 98, 255}, {81, 81, 255},
   {65, 65, 255}, {48, 48, 255},
   {32, 32, 255}, {15, 15, 255},
   {0, 0, 255}};

static int orange_black_lime[29][3] = // "Orange,Black,Lime"
  {{255, 153, 0}, {240, 145, 0},
   {224, 137, 0}, {208, 128, 0},
   {192, 119, 0}, {176, 111, 0},
   {160, 102, 0}, {144, 93, 0},
   {129, 85, 0}, {113, 76, 0},
   {97, 68, 0}, {81, 59, 0},
   {65, 50, 0}, {49, 42, 0},
   {32, 32, 32}, {42, 49, 0},
   {50, 65, 0}, {59, 81, 0},
   {68, 97, 0}, {76, 113, 0},
   {85, 129, 0}, {93, 144, 0},
   {102, 160, 0}, {111, 176, 0},
   {119, 192, 0}, {128, 208, 0},
   {137, 224, 0}, {145, 240, 0},
   {153, 255, 0}};

static int mixed_rainbow[256][3] = // "Mixed Rainbow"
  {{0, 0, 131}, {0, 68, 255}, {4, 255, 251}, {195, 255, 60}, {255, 124, 0},
   {187, 0, 0}, {0, 0, 135}, {0, 72, 255}, {8, 255, 247}, {199, 255, 56},
   {255, 120, 0}, {183, 0, 0}, {0, 0, 139}, {0, 76, 255}, {12, 255, 243},
   {203, 255, 52}, {255, 116, 0}, {179, 0, 0}, {0, 0, 143}, {0, 80, 255},
   {16, 255, 239}, {207, 255, 48}, {255, 112, 0}, {175, 0, 0}, {0, 0, 147},
   {0, 84, 255}, {20, 255, 235}, {211, 255, 44}, {255, 108, 0}, {171, 0, 0},
   {0, 0, 151}, {0, 88, 255}, {24, 255, 231}, {215, 255, 40}, {255, 104, 0},
   {167, 0, 0}, {0, 0, 155}, {0, 92, 255}, {28, 255, 227}, {219, 255, 36},
   {255, 100, 0}, {163, 0, 0}, {0, 0, 159}, {0, 96, 255}, {32, 255, 223},
   {223, 255, 32}, {255, 96, 0}, {159, 0, 0}, {0, 0, 163}, {0, 100, 255},
   {36, 255, 219}, {227, 255, 28}, {255, 92, 0}, {155, 0, 0}, {0, 0, 167},
   {0, 104, 255}, {40, 255, 215}, {231, 255, 24}, {255, 88, 0}, {151, 0, 0},
   {0, 0, 171}, {0, 108, 255}, {44, 255, 211}, {235, 255, 20}, {255, 84, 0},
   {147, 0, 0}, {0, 0, 175}, {0, 112, 255}, {48, 255, 207}, {239, 255, 16},
   {255, 80, 0}, {143, 0, 0}, {0, 0, 179}, {0, 116, 255}, {52, 255, 203},
   {243, 255, 12}, {255, 76, 0}, {139, 0, 0}, {0, 0, 183}, {0, 120, 255},
   {56, 255, 199}, {247, 255, 8}, {255, 72, 0}, {135, 0, 0}, {0, 0, 187},
   {0, 124, 255}, {60, 255, 195}, {251, 255, 4}, {255, 68, 0}, {131, 0, 0},
   {0, 0, 191}, {0, 128, 255}, {64, 255, 191}, {255, 255, 0}, {255, 64, 0},
   {128, 0, 0}, {0, 0, 195}, {0, 131, 255}, {68, 255, 187}, {255, 251, 0},
   {255, 60, 0}, {0, 0, 199}, {0, 135, 255}, {72, 255, 183}, {255, 247, 0},
   {255, 56, 0}, {0, 0, 203}, {0, 139, 255}, {76, 255, 179}, {255, 243, 0},
   {255, 52, 0}, {0, 0, 207}, {0, 143, 255}, {80, 255, 175}, {255, 239, 0},
   {255, 48, 0}, {0, 0, 211}, {0, 147, 255}, {84, 255, 171}, {255, 235, 0},
   {255, 44, 0}, {0, 0, 215}, {0, 151, 255}, {88, 255, 167}, {255, 231, 0},
   {255, 40, 0}, {0, 0, 219}, {0, 155, 255}, {92, 255, 163}, {255, 227, 0},
   {255, 36, 0}, {0, 0, 223}, {0, 159, 255}, {96, 255, 159}, {255, 223, 0},
   {255, 32, 0}, {0, 0, 227}, {0, 163, 255}, {100, 255, 155}, {255, 219, 0},
   {255, 28, 0}, {0, 0, 231}, {0, 167, 255}, {104, 255, 151}, {255, 215, 0},
   {255, 24, 0}, {0, 0, 235}, {0, 171, 255}, {108, 255, 147}, {255, 211, 0},
   {255, 20, 0}, {0, 0, 239}, {0, 175, 255}, {112, 255, 143}, {255, 207, 0},
   {255, 16, 0}, {0, 0, 243}, {0, 179, 255}, {116, 255, 139}, {255, 203, 0},
   {255, 12, 0}, {0, 0, 247}, {0, 183, 255}, {120, 255, 135}, {255, 199, 0},
   {255, 8, 0}, {0, 0, 251}, {0, 187, 255}, {124, 255, 131}, {255, 195, 0},
   {255, 4, 0}, {0, 0, 255}, {0, 191, 255}, {128, 255, 128}, {255, 191, 0},
   {255, 0, 0}, {0, 4, 255}, {0, 195, 255}, {131, 255, 124}, {255, 187, 0},
   {251, 0, 0}, {0, 8, 255}, {0, 199, 255}, {135, 255, 120}, {255, 183, 0},
   {247, 0, 0}, {0, 12, 255}, {0, 203, 255}, {139, 255, 116}, {255, 179, 0},
   {243, 0, 0}, {0, 16, 255}, {0, 207, 255}, {143, 255, 112}, {255, 175, 0},
   {239, 0, 0}, {0, 20, 255}, {0, 211, 255}, {147, 255, 108}, {255, 171, 0},
   {235, 0, 0}, {0, 24, 255}, {0, 215, 255}, {151, 255, 104}, {255, 167, 0},
   {231, 0, 0}, {0, 28, 255}, {0, 219, 255}, {155, 255, 100}, {255, 163, 0},
   {227, 0, 0}, {0, 32, 255}, {0, 223, 255}, {159, 255, 96}, {255, 159, 0},
   {223, 0, 0}, {0, 36, 255}, {0, 227, 255}, {163, 255, 92}, {255, 155, 0},
   {219, 0, 0}, {0, 40, 255}, {0, 231, 255}, {167, 255, 88}, {255, 151, 0},
   {215, 0, 0}, {0, 44, 255}, {0, 235, 255}, {171, 255, 84}, {255, 147, 0},
   {211, 0, 0}, {0, 48, 255}, {0, 239, 255}, {175, 255, 80}, {255, 143, 0},
   {207, 0, 0}, {0, 52, 255}, {0, 243, 255}, {179, 255, 76}, {255, 139, 0},
   {203, 0, 0}, {0, 56, 255}, {0, 247, 255}, {183, 255, 72}, {255, 135, 0},
   {199, 0, 0}, {0, 60, 255}, {0, 251, 255}, {187, 255, 68}, {255, 131, 0},
   {195, 0, 0}, {0, 64, 255}, {0, 255, 255}, {191, 255, 64}, {255, 128, 0},
   {191, 0, 0}};

static int mixed_grayscale[256][3] = // "Mixed GrayScale"
  {{0, 0, 0}, {48, 48, 48}, {96, 96, 96}, {144, 144, 144},
   {192, 192, 192}, {240, 240, 240}, {1, 1, 1}, {49, 49, 49},
   {97, 97, 97}, {145, 145, 145}, {193, 193, 193}, {241, 241, 241},
   {2, 2, 2}, {50, 50, 50}, {98, 98, 98}, {146, 146, 146},
   {194, 194, 194}, {242, 242, 242}, {3, 3, 3}, {51, 51, 51},
   {99, 99, 99}, {147, 147, 147}, {195, 195, 195}, {243, 243, 243},
   {4, 4, 4}, {52, 52, 52}, {100, 100, 100}, {148, 148, 148},
   {196, 196, 196}, {244, 244, 244}, {5, 5, 5}, {53, 53, 53},
   {101, 101, 101}, {149, 149, 149}, {197, 197, 197}, {245, 245, 245},
   {6, 6, 6}, {54, 54, 54}, {102, 102, 102}, {150, 150, 150},
   {198, 198, 198}, {246, 246, 246}, {7, 7, 7}, {55, 55, 55},
   {103, 103, 103}, {151, 151, 151}, {199, 199, 199}, {247, 247, 247},
   {8, 8, 8}, {56, 56, 56}, {104, 104, 104}, {152, 152, 152},
   {200, 200, 200}, {248, 248, 248}, {9, 9, 9}, {57, 57, 57},
   {105, 105, 105}, {153, 153, 153}, {201, 201, 201}, {249, 249, 249},
   {10, 10, 10}, {58, 58, 58}, {106, 106, 106}, {154, 154, 154},
   {202, 202, 202}, {250, 250, 250}, {11, 11, 11}, {59, 59, 59},
   {107, 107, 107}, {155, 155, 155}, {203, 203, 203}, {251, 251, 251},
   {12, 12, 12}, {60, 60, 60}, {108, 108, 108}, {156, 156, 156},
   {204, 204, 204}, {252, 252, 252}, {13, 13, 13}, {61, 61, 61},
   {109, 109, 109}, {157, 157, 157}, {205, 205, 205}, {253, 253, 253},
   {14, 14, 14}, {62, 62, 62}, {110, 110, 110}, {158, 158, 158},
   {206, 206, 206}, {254, 254, 254}, {15, 15, 15}, {63, 63, 63},
   {111, 111, 111}, {159, 159, 159}, {207, 207, 207}, {255, 255, 255},
   {16, 16, 16}, {64, 64, 64}, {112, 112, 112}, {160, 160, 160},
   {208, 208, 208}, {17, 17, 17}, {65, 65, 65}, {113, 113, 113},
   {161, 161, 161}, {209, 209, 209}, {18, 18, 18}, {66, 66, 66},
   {114, 114, 114}, {162, 162, 162}, {210, 210, 210}, {19, 19, 19},
   {67, 67, 67}, {115, 115, 115}, {163, 163, 163}, {211, 211, 211},
   {20, 20, 20}, {68, 68, 68}, {116, 116, 116}, {164, 164, 164},
   {212, 212, 212}, {21, 21, 21}, {69, 69, 69}, {117, 117, 117},
   {165, 165, 165}, {213, 213, 213}, {22, 22, 22}, {70, 70, 70},
   {118, 118, 118}, {166, 166, 166}, {214, 214, 214}, {23, 23, 23},
   {71, 71, 71}, {119, 119, 119}, {167, 167, 167}, {215, 215, 215},
   {24, 24, 24}, {72, 72, 72}, {120, 120, 120}, {168, 168, 168},
   {216, 216, 216}, {25, 25, 25}, {73, 73, 73}, {121, 121, 121},
   {169, 169, 169}, {217, 217, 217}, {26, 26, 26}, {74, 74, 74},
   {122, 122, 122}, {170, 170, 170}, {218, 218, 218}, {27, 27, 27},
   {75, 75, 75}, {123, 123, 123}, {171, 171, 171}, {219, 219, 219},
   {28, 28, 28}, {76, 76, 76}, {124, 124, 124}, {172, 172, 172},
   {220, 220, 220}, {29, 29, 29}, {77, 77, 77}, {125, 125, 125},
   {173, 173, 173}, {221, 221, 221}, {30, 30, 30}, {78, 78, 78},
   {126, 126, 126}, {174, 174, 174}, {222, 222, 222}, {31, 31, 31},
   {79, 79, 79}, {127, 127, 127}, {175, 175, 175}, {223, 223, 223},
   {32, 32, 32}, {80, 80, 80}, {128, 128, 128}, {176, 176, 176},
   {224, 224, 224}, {33, 33, 33}, {81, 81, 81}, {129, 129, 129},
   {177, 177, 177}, {225, 225, 225}, {34, 34, 34}, {82, 82, 82},
   {130, 130, 130}, {178, 178, 178}, {226, 226, 226}, {35, 35, 35},
   {83, 83, 83}, {131, 131, 131}, {179, 179, 179}, {227, 227, 227},
   {36, 36, 36}, {84, 84, 84}, {132, 132, 132}, {180, 180, 180},
   {228, 228, 228}, {37, 37, 37}, {85, 85, 85}, {133, 133, 133},
   {181, 181, 181}, {229, 229, 229}, {38, 38, 38}, {86, 86, 86},
   {134, 134, 134}, {182, 182, 182}, {230, 230, 230}, {39, 39, 39},
   {87, 87, 87}, {135, 135, 135}, {183, 183, 183}, {231, 231, 231},
   {40, 40, 40}, {88, 88, 88}, {136, 136, 136}, {184, 184, 184},
   {232, 232, 232}, {41, 41, 41}, {89, 89, 89}, {137, 137, 137},
   {185, 185, 185}, {233, 233, 233}, {42, 42, 42}, {90, 90, 90},
   {138, 138, 138}, {186, 186, 186}, {234, 234, 234}, {43, 43, 43},
   {91, 91, 91}, {139, 139, 139}, {187, 187, 187}, {235, 235, 235},
   {44, 44, 44}, {92, 92, 92}, {140, 140, 140}, {188, 188, 188},
   {236, 236, 236}, {45, 45, 45}, {93, 93, 93}, {141, 141, 141},
   {189, 189, 189}, {237, 237, 237}, {46, 46, 46}, {94, 94, 94},
   {142, 142, 142}, {190, 190, 190}, {238, 238, 238}, {47, 47, 47},
   {95, 95, 95}, {143, 143, 143}, {191, 191, 191}, {239, 239, 239}};


ColorMapHandle
generate_cmap(int c[][3], int sz, int res, double gamma)
{
  //! Slurp in the rgb values.
  vector< Color > rgbs(sz);
  vector< float > rgbT(sz);
  for (int i = 0; i < sz; i++) {
    int *col = c[i];
    rgbs[i] = Color(col[0] / 255.0, col[1] / 255.0, col[2] / 255.0);
    rgbT[i] = i / float(sz - 1);
  }

  //! Shift RGB
  const double bp = 1.0 / tan( M_PI_2 * (0.5 + gamma * 0.5));
  for (size_t i=0; i < rgbT.size(); i++) {
    rgbT[i] = pow((double)rgbT[i], bp);
  }
    

  //! Read in the alpha values.  This is the screen space curve.
  unsigned int asize = 0;
  float w = 390;
  float h = 40;
  vector< float > alphas(asize);
  vector< float > alphaT(asize);

  for (unsigned int i = 0; i < asize; i++) {
    int x, y;
    alphaT[i] = x / w;
    alphas[i] = 1.0 - y / h;
  }
    
  //! Correct for not being able to perfectly align the
  //! widgets
  if (alphaT.size() > 0)
  {
    if ( fabs(alphaT[0]) < 0.005) alphaT[0] = 0.0;
    if ( fabs(alphaT[alphaT.size()-1]-1.0) < 0.005) alphaT[alphaT.size()-1] = 1.0;
  }
    
  //! The screen space curve may not contain the default endpoints.
  //! Add them in here if needed.
  if (alphaT.empty() || alphaT.front() != 0.0) {
    alphas.insert(alphas.begin(), 0.5);
    alphaT.insert(alphaT.begin(), 0.0);
  }
  if (alphaT.back() != 1.0) {
    alphas.push_back(0.5);
    alphaT.push_back(1.0);
  }
    
  return new ColorMap(rgbs, rgbT, alphas, alphaT, res);

}


//! Algorithm Interface.
size_t
CreateStandardColorMapsAlg::execute()
{
  DataManager *dm = DataManager::get_dm();

  if (get_p_mapName() == "Gray") {
    static size_t gray_id = 0;
    if (! gray_id) {
      ColorMapHandle cmh = generate_cmap(gray, 2, 
					 get_p_resolution(), 
					 get_p_gamma());
      gray_id = dm->add_colormap(cmh);
    }
    return gray_id;
  }

  if (get_p_mapName() == "Old Rainbow") {
    static size_t old_rainbow_id = 0;
    if (! old_rainbow_id) {
      ColorMapHandle cmh = generate_cmap(old_rainbow, 12, 
					 get_p_resolution(), 
					 get_p_gamma());
      old_rainbow_id = dm->add_colormap(cmh);
    }
    return old_rainbow_id;
  }

  if (get_p_mapName() == "Darkhue") {
    static size_t darkhue_id = 0;
    if (! darkhue_id) {
      ColorMapHandle cmh = generate_cmap(darkhue, 19, 
					 get_p_resolution(), 
					 get_p_gamma());
      darkhue_id = dm->add_colormap(cmh);
    }
    return darkhue_id;
  }

  if (get_p_mapName() == "Lighthue") {
    static size_t lighthue_id = 0;
    if (! lighthue_id) {
      ColorMapHandle cmh = generate_cmap(lighthue, 19, 
					 get_p_resolution(), 
					 get_p_gamma());
      lighthue_id = dm->add_colormap(cmh);
    }
    return lighthue_id;
  }

  if (get_p_mapName() == "Blackbody") {
    static size_t blackbody_id = 0;
    if (! blackbody_id) {
      ColorMapHandle cmh = generate_cmap(blackbody, 13, 
					 get_p_resolution(), 
					 get_p_gamma());
      blackbody_id = dm->add_colormap(cmh);
    }
    return blackbody_id;
  }

  if (get_p_mapName() == "Don") {
    static size_t don_id = 0;
    if (! don_id) {
      ColorMapHandle cmh = generate_cmap(don, 10, 
					 get_p_resolution(), 
					 get_p_gamma());
      don_id = dm->add_colormap(cmh);
    }
    return don_id;
  }

  if (get_p_mapName() == "BP Seismic") {
    static size_t bp_seismic_id = 0;
    if (! bp_seismic_id) {
      ColorMapHandle cmh = generate_cmap(bp_seismic, 3, 
					 get_p_resolution(), 
					 get_p_gamma());
      bp_seismic_id = dm->add_colormap(cmh);
    }
    return bp_seismic_id;
  }

  if (get_p_mapName() == "Dark Gray") {
    static size_t dark_gray_id = 0;
    if (! dark_gray_id) {
      ColorMapHandle cmh = generate_cmap(dark_gray, 4, 
					 get_p_resolution(), 
					 get_p_gamma());
      dark_gray_id = dm->add_colormap(cmh);
    }
    return dark_gray_id;
  }

  if (get_p_mapName() == "Red Tint") {
    static size_t red_tint_id = 0;
    if (! red_tint_id) {
      ColorMapHandle cmh = generate_cmap(red_tint, 2, 
					 get_p_resolution(), 
					 get_p_gamma());
      red_tint_id = dm->add_colormap(cmh);
    }
    return red_tint_id;
  }

  if (get_p_mapName() == "Orange Tint") {
    static size_t orange_tint_id = 0;
    if (! orange_tint_id) {
      ColorMapHandle cmh = generate_cmap(orange_tint, 2, 
					 get_p_resolution(), 
					 get_p_gamma());
      orange_tint_id = dm->add_colormap(cmh);
    }
    return orange_tint_id;
  }

  if (get_p_mapName() == "Yellow Tint") {
    static size_t yellow_tint_id = 0;
    if (! yellow_tint_id) {
      ColorMapHandle cmh = generate_cmap(yellow_tint, 2, 
					 get_p_resolution(), 
					 get_p_gamma());
      yellow_tint_id = dm->add_colormap(cmh);
    }
    return yellow_tint_id;
  }

  if (get_p_mapName() == "Green Tint") {
    static size_t green_tint_id = 0;
    if (! green_tint_id) {
      ColorMapHandle cmh = generate_cmap(green_tint, 2, 
					 get_p_resolution(), 
					 get_p_gamma());
      green_tint_id = dm->add_colormap(cmh);
    }
    return green_tint_id;
  }

  if (get_p_mapName() == "Cyan Tint") {
    static size_t cyan_tint_id = 0;
    if (! cyan_tint_id) {
      ColorMapHandle cmh = generate_cmap(cyan_tint, 2, 
					 get_p_resolution(), 
					 get_p_gamma());
      cyan_tint_id = dm->add_colormap(cmh);
    }
    return cyan_tint_id;
  }

  if (get_p_mapName() == "Blue Tint") {
    static size_t blue_tint_id = 0;
    if (! blue_tint_id) {
      ColorMapHandle cmh = generate_cmap(blue_tint, 2, 
					 get_p_resolution(), 
					 get_p_gamma());
      blue_tint_id = dm->add_colormap(cmh);
    }
    return blue_tint_id;
  }

  if (get_p_mapName() == "Purple Tint") {
    static size_t purple_tint_id = 0;
    if (! purple_tint_id) {
      ColorMapHandle cmh = generate_cmap(purple_tint, 2, 
					 get_p_resolution(), 
					 get_p_gamma());
      purple_tint_id = dm->add_colormap(cmh);
    }
    return purple_tint_id;
  }

  if (get_p_mapName() == "Pink,White,Blue") {
    static size_t pink_white_blue_id = 0;
    if (! pink_white_blue_id) {
      ColorMapHandle cmh = generate_cmap(pink_white_blue, 29, 
					 get_p_resolution(), 
					 get_p_gamma());
      pink_white_blue_id = dm->add_colormap(cmh);
    }
    return pink_white_blue_id;
  }

  if (get_p_mapName() == "Orange,Black,Lime") {
    static size_t orange_black_lime_id = 0;
    if (! orange_black_lime_id) {
      ColorMapHandle cmh = generate_cmap(orange_black_lime, 29, 
					 get_p_resolution(), 
					 get_p_gamma());
      orange_black_lime_id = dm->add_colormap(cmh);
    }
    return orange_black_lime_id;
  }

  if (get_p_mapName() == "Mixed Rainbow") {
    static size_t mixed_rainbow_id = 0;
    if (! mixed_rainbow_id) {
      ColorMapHandle cmh = generate_cmap(mixed_rainbow, 256, 
					 get_p_resolution(), 
					 get_p_gamma());
      mixed_rainbow_id = dm->add_colormap(cmh);
    }
    return mixed_rainbow_id;
  }

  if (get_p_mapName() == "Mixed Grayscale") {
    static size_t mixed_grayscale_id = 0;
    if (! mixed_grayscale_id) {
      ColorMapHandle cmh = generate_cmap(mixed_grayscale, 256, 
					 get_p_resolution(), 
					 get_p_gamma());
      mixed_grayscale_id = dm->add_colormap(cmh);
    }
    return mixed_grayscale_id;
  }
  
  // default
  static size_t rbow_id = 0;
  if (! rbow_id) {
    ColorMapHandle cmh = generate_cmap(rainbow, 28, 
				       get_p_resolution(), 
				       get_p_gamma());
    rbow_id = dm->add_colormap(cmh);
  }
  return rbow_id;
}

} //end namespace SCIRun

