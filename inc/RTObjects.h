#ifndef RTOBJECTS_H
#define RTOBJECTS_H

#include <vector>

#include "Geom.h"
#include "RTMaterial.h"
#include "RTGeometry.h"
class SceneManager;


struct SceneObject {
    const RTMaterial *material_;
    const SceneManager *parent_;
    GmPoint<double, 3> position_; 

    SceneObject(const RTMaterial *material, const SceneManager *parent=nullptr): material_(material), parent_(parent) {
        assert(material);
    }

public:
    virtual ~SceneObject() = default;

    virtual bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const = 0;
};

class SphereObject : public SceneObject {
    double radius_;

public:
    SphereObject(double radius, const RTMaterial *material, const SceneManager *parent=nullptr): SceneObject(material, parent), radius_(radius) {}

    bool hit(const Ray& ray, Interval rayTime, HitRecord& rec) const override {
        GmVec<double,3> oc = ray.origin - position_; 
        double a = dot(ray.direction, ray.direction);         
        double half_b = dot(oc, ray.direction);              
        double c = dot(oc, oc) - radius_ * radius_;

        double discriminant = half_b*half_b - a*c;
        if (discriminant < 0.0) return false;

        double sqrtd = std::sqrt(discriminant);
        
        double root = (-half_b - sqrtd) / a;
        if (!rayTime.surrounds(root)) {
            root = (-half_b + sqrtd) / a;
            if (!rayTime.surrounds(root)) return false;
        }

        rec.time = root;
        rec.point = ray.origin + ray.direction * root;

        GmVec<double,3> outwardNormal = (rec.point - position_) / radius_; 
        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material_;
        return true;
    }
};

class PlaneObject : public SceneObject {
    GmPoint<double, 3> point_;
    GmVec<double, 3> normal_;

public:
    PlaneObject
    (
        const GmPoint<double, 3> point, GmVec<double, 3> normal,
        const RTMaterial *material, const SceneManager *parent=nullptr
    ):
        SceneObject(material, parent), point_(point), normal_(normal.normalized()) {}

    bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const override {
        double time = dot(normal_, point_ - ray.origin) * (1.0 / dot(normal_, ray.direction));

        if (!rayTime.surrounds(time)) return false;

        hitRecord.material = material_;
        hitRecord.time = time;
        hitRecord.point = ray.origin + ray.direction * time;
        
        GmVec<double, 3> outwardNormal = normal_;
        if (dot(normal_, ray.direction) < 0) outwardNormal = normal_ * (-1);  

        hitRecord.setFaceNormal(ray, outwardNormal);

        return true;
    }
};

#endif // RTOBJECTS_H