#ifndef RTGEOMETRY_H
#define RTGEOMETRY_H

#include "IVec3f.hpp"
class RTMaterial;
class Primitives;
#include "GmUtilities.hpp"


inline gm::IVec3f randomOnHemisphere(const gm::IVec3f& normal) {
    gm::IVec3f on_unit_sphere = gm::IVec3f::randomUnit();
    if (dot(on_unit_sphere, normal) > 0.0)
        return on_unit_sphere;
    else
        return on_unit_sphere * (-1);
}

inline gm::IVec3f reflect(const gm::IVec3f& v, const gm::IVec3f& n) {
    return v - n * 2 * dot(v,n) ;
}

inline gm::IVec3f refract(const gm::IVec3f& uv, const gm::IVec3f& n, double refractionCoef) {
    gm::IVec3f unitUv = uv.normalized();
    double cosTheta = std::fmin(dot(unitUv * (-1), n), 1.0);

    gm::IVec3f rOutPerp = (unitUv + n * cosTheta) * refractionCoef;

    if (1.0 - rOutPerp.length2() < 0) rOutPerp = rOutPerp * (1.0 / (refractionCoef * refractionCoef));

    assert(1.0 - rOutPerp.length2() > 0);
    gm::IVec3f rOutParallel = n * (-std::sqrt(1.0 - rOutPerp.length2()));

    return rOutPerp + rOutParallel;
}

struct Ray {
    gm::IPoint3 origin;
    gm::IVec3f direction;

    Ray() {}

    Ray(const gm::IPoint3& origin, const gm::IVec3f &direction) : origin(origin), direction(direction) {}

    gm::IPoint3 at(double t) const {
        return origin + direction * t;
    }    
};

struct HitRecord {
    gm::IPoint3 point = {};
    gm::IVec3f normal = {};
    const RTMaterial *material = nullptr;
    const Primitives *object = nullptr;
    bool frontFace = false;
    double time = 0;
    bool hitExpanded = false;

    void setFaceNormal(const Ray& ray, const gm::IVec3f& outwardNormal) {
        frontFace = dot(ray.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal.normalized() : outwardNormal.normalized() * (-1);
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