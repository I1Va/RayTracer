#ifndef UTILITIES_H
#define UTILITIES_H

#include "Geom.h"

inline GmVec<double, 3> randomUnitVector() {
    return GmVec<double, 3>::random().normalized();

}
#endif // UTILITIES_H