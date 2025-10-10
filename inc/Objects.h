#ifndef OBJECTS_H
#define OBJECTS_H


#include "Geom.h"

class SceneManager;

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

    static const Interval empty, universe;
};

const Interval Interval::empty    = Interval(+std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
const Interval Interval::universe = Interval(-std::numeric_limits<double>::infinity(), +std::numeric_limits<double>::infinity());

struct SceneObject {
    const SceneManager *parent_;
    GmPoint<double, 3> position_; 

    SceneObject(const SceneManager *parent=nullptr): parent_(parent) {}

public:
    virtual ~SceneObject() = default;

    virtual bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const = 0;
};

class SphereObject : public SceneObject {
    double radius_;

public:
    SphereObject(double radius, const SceneManager *parent=nullptr): SceneObject(parent), radius_(radius) {}

    bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const override {
        GmVec<double, 3> oc = position_ - ray.origin;
        double a = ray.direction.length2();
        double h = dot(ray.direction, oc);
        double c = oc.length2() - radius_ * radius_;

        double discriminant = h * h - a * c;
        if (discriminant < 0)
            return false;

        double sqrtDiscriminant = std::sqrt(discriminant);

        double root = (h - sqrtDiscriminant) / a;
        if (!rayTime.surrounds(root)) {
            root = (h + sqrtDiscriminant) / a;
            if (!rayTime.surrounds(root))
                return false;
        }

        hitRecord.time = root;
        hitRecord.point = ray.origin + ray.direction * root;
        
        GmVec<double, 3> outwardNormal = (hitRecord.point - position_) / radius_;
        hitRecord.setFaceNormal(ray, outwardNormal);

        return true;
    }
};


#endif // OBJECTS_H