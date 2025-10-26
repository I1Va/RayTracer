#ifndef UTILITIES_H
#define UTILITIES_H

#include "Geom.h"
#include "IntinsicGeom.h"

inline gm::IVec3 randomUnitVector() {
    return gm::IVec3::random().normalized();

}

#endif // UTILITIES_H