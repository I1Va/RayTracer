#ifndef OUTPUT_H
#define OUTPUT_H

#include <iostream>
#include "RTGeometry.h"

std::ostream &operator<< (std::ostream &stream, const Ray &ray) {
    stream << "ray{" << ray.origin << ", " << ray.direction << "}";
    return stream;
}


#endif // OUTPUT_H