#ifndef RAY_H
#define RAY_H

#include "Geom.h"

class ray {

    GmPoint<double, 3> orig_;
    GmVec<double, 3> dir_;

public:
    ray() {}

    ray(const GmPoint<double, 3>& origin, const GmVec<double, 3> &direction) : orig_(origin), dir_(direction) {}

    const point3& origin() const  { return orig; }
    const vec3& direction() const { return dir; }

    point3 at(double t) const {
        return orig + t*dir;
    }    
};


#endif // RAY_H