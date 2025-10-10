#ifndef UTILITIES_H
#define UTILITIES_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <random>


inline double degrees_to_radians(double degrees) {
    return degrees * M_PI / 180.0;
}

inline double randomDouble() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double randomDouble(double min, double max) {
    return min + (max-min)*randomDouble();
}


#endif // UTILITIES_H