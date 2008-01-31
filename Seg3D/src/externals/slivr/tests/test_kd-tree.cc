#include "../KDTree.h"

using namespace SLIVR;

int main() {
  Point min(0.0, 0.0, 0.0);
  Point max(1.0, 1.0, 1.0);
  BBox V(min, max);

  vector<BBox> sub;
  Point min1(0.1, 0.1, 0.1);
  Point max1(0.4, 0.4, 0.4);
  BBox b1(min1, max1);
  sub.push_back(b1);

  Point min2(0.6, 0.6, 0.6);
  Point max2(0.9, 0.9, 0.9);
  BBox b2(min2, max2);
  sub.push_back(b2);

  KDTree kd(sub, V);

};
