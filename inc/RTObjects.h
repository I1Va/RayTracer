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
    gm::IPoint3 position_; 

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
        gm::IVec3 oc = ray.origin - position_;
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

        gm::IVec3 outwardNormal = (rec.point - position_) / radius_; 
        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material_;
        return true;
    }
};

class PlaneObject : public Primitives {
    gm::IPoint3 point_;
    gm::IVec3 normal_;

public:
    PlaneObject
    (
        const gm::IPoint3 point, gm::IVec3 normal,
        const RTMaterial *material, const SceneManager *parent=nullptr
    ):
        Primitives(material, parent), point_(point), normal_(normal.normalized()) {}

    bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const override {
        double time = dot(normal_, point_ - ray.origin) * (1.0 / dot(normal_, ray.direction));

        if (!rayTime.surrounds(time)) return false;

        hitRecord.material = material_;
        hitRecord.time = time;
        hitRecord.point = ray.origin + ray.direction * time;
        
        gm::IVec3 outwardNormal = normal_;
        if (dot(normal_, ray.direction) < 0) outwardNormal = normal_ * (-1);  

        hitRecord.setFaceNormal(ray, outwardNormal);

        return true;
    }
};




class Light {
    gm::IVec3 ambientIntensity_;
    gm::IVec3 defuseIntensity_;
    gm::IVec3 specularIntensity_;
    gm::IPoint3 center_;
    double viewLightPow_;

    const SceneManager *parent_;

public:
    Light
    (
        const gm::IVec3 ambientIntensity,
        const gm::IVec3 defuseIntensity,
        const gm::IVec3 specularIntensity,
        double viewLightPow,
        const SceneManager *parent=nullptr
    ) : 
    ambientIntensity_(ambientIntensity), 
    defuseIntensity_(defuseIntensity), 
    specularIntensity_(specularIntensity),
    viewLightPow_(viewLightPow),
    parent_(parent) {}

    virtual RTColor getDirectLighting(const gm::IPoint3 onSurfPoint, const gm::IVec3 surfNormal,
                                      const gm::IVec3 toView, const RTMaterial *surfMaterial) 
    {
        assert(surfMaterial);

        gm::IVec3 ambientIntensity = ambientIntensity_ * surfMaterial->albedo();

        gm::IVec3 toLight = center_ - onSurfPoint;
    
        gm::IVec3 defuseIntensity = defuseLightIntensity(toLight, surfNormal) * surfMaterial->albedo();

        gm::IVec3 specularIntensity = specularLightIntesity(toLight, toView, surfNormal);

        return ambientIntensity + defuseIntensity + specularIntensity;
    }

    gm::IPoint3 center() const { return center_; }
    const SceneManager *parent() const { return parent_; }
    void setParent(const SceneManager *parent) { parent_ = parent; }
    void setPosition(const gm::IPoint3 center) { center_ = center; }

private:
    // Lambert
    gm::IVec3 defuseLightIntensity(const gm::IVec3 toLight, const gm::IVec3 surfNormal) {
        double lightRefAngleCos = dot(toLight.normalized(), surfNormal.normalized());

        if (lightRefAngleCos > 0)
            return defuseIntensity_ * lightRefAngleCos;
        return gm::IVec3(0.0, 0.0, 0.0);
    }

    // Phong
    gm::IVec3 specularLightIntesity(const gm::IVec3 toLight, const gm::IVec3 toView, const gm::IVec3 surfNormal) {
        gm::IVec3 surfToReflLight = toLight - getOrtogonal(toLight, surfNormal) * 2;
        
        double view_angle_cos = dot(surfToReflLight.normalized(), toView.normalized());
        
        if (view_angle_cos > 0) {
            double resCoef = std::pow(view_angle_cos, viewLightPow_);
            return gm::IVec3(resCoef); 
        }

        return gm::IVec3(0, 0, 0);        
    }
};


#endif // RTOBJECTS_H