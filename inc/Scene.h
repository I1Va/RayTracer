#ifndef OBJECTS_H
#define OBJECTS_H

#include <vector>

#include "Geom.h"
#include "RTGeometry.h"
class SceneManager;


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