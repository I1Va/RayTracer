#ifndef RTOBJECTS_H
#define RTOBJECTS_H

#include <vector>

#include "Geom.h"
#include "RTMaterial.h"
#include "RTGeometry.h"

class SceneManager;

struct Primitives {
    const RTMaterial *material_;
    const SceneManager *parent_;
    GmPoint<double, 3> position_; 

    Primitives(const RTMaterial *material, const SceneManager *parent=nullptr): material_(material), parent_(parent) {
        assert(material);
    }

public:
    virtual ~Primitives() = default;

    virtual bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const = 0;
};

class SphereObject : public Primitives {
    double radius_;

public:
    SphereObject(double radius, const RTMaterial *material, const SceneManager *parent=nullptr): Primitives(material, parent), radius_(radius) {}

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

class PlaneObject : public Primitives {
    GmPoint<double, 3> point_;
    GmVec<double, 3> normal_;

public:
    PlaneObject
    (
        const GmPoint<double, 3> point, GmVec<double, 3> normal,
        const RTMaterial *material, const SceneManager *parent=nullptr
    ):
        Primitives(material, parent), point_(point), normal_(normal.normalized()) {}

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




class Light {
    GmVec<double, 3> ambientIntensity_;
    GmVec<double, 3> defuseIntensity_;
    GmVec<double, 3> specularIntensity_;
    GmPoint<double, 3> center_;
    double viewLightPow_;

    const SceneManager *parent_;

public:
    Light
    (
        const GmVec<double, 3> ambientIntensity,
        const GmVec<double, 3> defuseIntensity,
        const GmVec<double, 3> specularIntensity,
        double viewLightPow,
        const SceneManager *parent=nullptr
    ) : 
    ambientIntensity_(ambientIntensity), 
    defuseIntensity_(defuseIntensity), 
    specularIntensity_(specularIntensity),
    viewLightPow_(viewLightPow),
    parent_(parent) {}

    virtual RTColor getDirectLighting(const GmPoint<double, 3> onSurfPoint, const GmVec<double, 3> surfNormal,
                                      const GmVec<double, 3> toView, const RTMaterial *surfMaterial) 
    {
        assert(surfMaterial);

        GmVec<double, 3> ambientIntensity = ambientIntensity_ * surfMaterial->albedo();

        GmVec<double, 3> toLight = center_ - onSurfPoint;
    
        GmVec<double, 3> defuseIntensity = defuseLightIntensity(toLight, surfNormal) * surfMaterial->albedo();

        GmVec<double, 3> specularIntensity = specularLightIntesity(toLight, toView, surfNormal);

        return ambientIntensity + defuseIntensity + specularIntensity;
    }

    GmPoint<double, 3> center() const { return center_; }
    const SceneManager *parent() const { return parent_; }
    void setParent(const SceneManager *parent) { parent_ = parent; }
    void setPosition(const GmPoint<double, 3> center) { center_ = center; }

private:
    // Lambert
    GmVec<double, 3> defuseLightIntensity(const GmVec<double, 3> toLight, const GmVec<double, 3> surfNormal) {
        double lightRefAngleCos = dot(toLight.normalized(), surfNormal.normalized());

        if (lightRefAngleCos > 0)
            return defuseIntensity_ * lightRefAngleCos;
        return GmVec<double, 3>(0.0, 0.0, 0.0);
    }

    // Phong
    GmVec<double, 3> specularLightIntesity(const GmVec<double, 3> toLight, const GmVec<double, 3> toView, const GmVec<double, 3> surfNormal) {
        GmVec<double, 3> surfToReflLight = toLight - getOrtogonal(toLight, surfNormal) * 2;
        
        double view_angle_cos = dot(surfToReflLight.normalized(), toView.normalized());
        
        if (view_angle_cos > 0) {
            double resCoef = std::pow(view_angle_cos, viewLightPow_);
            return GmVec<double, 3>(resCoef); 
        }

        return GmVec<double, 3>(0, 0, 0);        
    }
};


#endif // RTOBJECTS_H