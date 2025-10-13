#ifndef RTGEOMETRY_H
#define RTGEOMETRY_H

#include "Geom.h"
#include "Utilities.h"


inline GmVec<double, 3> randomOnHemisphere(const GmVec<double, 3>& normal) {
    GmVec<double, 3> on_unit_sphere = randomUnitVector();
    if (dot(on_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return on_unit_sphere;
    else
        return on_unit_sphere * (-1);
}


struct Ray {

    GmPoint<double, 3> origin;
    GmVec<double, 3> direction;

    Ray() {}

    Ray(const GmPoint<double, 3>& origin, const GmVec<double, 3> &direction) : origin(origin), direction(direction) {}

    GmPoint<double, 3> at(double t) const {
        return origin + direction * t;
    }    
};

struct HitRecord {
    GmPoint<double, 3> point = {};
    GmVec<double, 3> normal = {};
    bool frontFace = false;
    double time = 0;

    void setFaceNormal(const Ray& ray, const GmVec<double, 3>& outwardNormal) {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        frontFace = dot(ray.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : outwardNormal * (-1);
    }
};

struct Interval {
    double min, max;

    Interval() : min(+std::numeric_limits<double>::infinity()), max(-std::numeric_limits<double>::infinity()) {}

    Interval(double min, double max) : min(min), max(max) {}

    double size() const {
        return max - min;
    }

    bool contains(double x) const {
        return min <= x && x <= max;
    }

    bool surrounds(double x) const {
        return min < x && x < max;
    }

    double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    static const Interval empty, universe;
};


#endif // RTGEOMETRY_H