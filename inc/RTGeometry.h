#ifndef RTGEOMETRY_H
#define RTGEOMETRY_H

#include "Geom.h"
class RTMaterial;
#include "Utilities.h"


inline GmVec<double, 3> randomOnHemisphere(const GmVec<double, 3>& normal) {
    GmVec<double, 3> on_unit_sphere = randomUnitVector();
    if (dot(on_unit_sphere, normal) > 0.0)
        return on_unit_sphere;
    else
        return on_unit_sphere * (-1);
}

inline GmVec<double, 3> reflect(const GmVec<double, 3>& v, const GmVec<double, 3>& n) {
    return v - n * 2 * dot(v,n) ;
}

inline GmVec<double, 3> refract(const GmVec<double, 3>& uv, const GmVec<double, 3>& n, double refractionCoef) {
    GmVec<double, 3> unitUv = uv.normalized();
    double cosTheta = std::fmin(dot(unitUv * (-1), n), 1.0);

    GmVec<double, 3> rOutPerp = (unitUv + n * cosTheta) * refractionCoef;

    if (1.0 - rOutPerp.length2() < 0) rOutPerp = rOutPerp * (1.0 / (refractionCoef * refractionCoef));

    assert(1.0 - rOutPerp.length2() > 0);
    GmVec<double, 3> rOutParallel = n * (-std::sqrt(1.0 - rOutPerp.length2()));

    return rOutPerp + rOutParallel;
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
    const RTMaterial *material = nullptr;
    bool frontFace = false;
    double time = 0;

    void setFaceNormal(const Ray& ray, const GmVec<double, 3>& outwardNormal) {
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