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

struct SceneObject {
    const SceneManager *parent_;
    GmPoint<double, 3> position_; 

    SceneObject(const SceneManager *parent=nullptr): parent_(parent) {}

public:
    virtual ~SceneObject() = default;
    virtual double rayHit(const Ray& ray) const = 0;
};

class SphereObject : public SceneObject {
    double radius_;

public:
    SphereObject(double radius, const SceneManager *parent=nullptr): SceneObject(parent), radius_(radius) {}

    double rayHit(const Ray& ray) const override {
        GmVec<double, 3> oc = position_ - ray.origin;
    
        auto a = dot(ray.direction, ray.direction);
        auto b = -2.0 * dot(ray.direction, oc);
        auto c = dot(oc, oc) - radius_*radius_;
        auto discriminant = b*b - 4*a*c;

        if (discriminant < 0) {
            return -1.0;
        } else {
            return (-b - std::sqrt(discriminant) ) / (2.0*a);
        }
    }
};


#endif // OBJECTS_H