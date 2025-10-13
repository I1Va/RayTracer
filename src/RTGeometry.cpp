#include "RTGeometry.h"


const Interval Interval::empty    = Interval(+std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
const Interval Interval::universe = Interval(-std::numeric_limits<double>::infinity(), +std::numeric_limits<double>::infinity());
