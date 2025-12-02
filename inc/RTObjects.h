#ifndef RTOBJECTS_H
#define RTOBJECTS_H

#include <vector>
#include <string>

#include "Geom.hpp"
#include "IVec3f.hpp"
#include "RTMaterial.h"
#include "RTGeometry.h"

inline constexpr float SELECTED_DELTA_X = 0.1;
inline constexpr float SELECTED_DELTA_Y = 0.1;
inline constexpr float SELECTED_DELTA_Z = 0.1;

class SceneManager;

struct Primitives {
    const SceneManager *parent_;
    
    RTMaterial *material_;

    gm::IPoint3 position_{};
    bool selectFlag_ = false;

    Primitives(RTMaterial *material, const SceneManager *parent=nullptr): parent_(parent), material_(material) {
        assert(material);
    }

public:
    virtual ~Primitives() = default;

    virtual bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const = 0;

    virtual std::string typeString() const { return "Primitive"; }

    const gm::IPoint3& position() const { return position_; }
    gm::IPoint3& position() { return position_; }

    const RTMaterial* material() const { return material_; }
    RTMaterial* material() { return material_; }


    void setSelectFlag(bool val) {selectFlag_ = val; }
    bool selected() const { return selectFlag_; }
};

class SphereObject : public Primitives {
    double radius_;

public:
    SphereObject(double radius, RTMaterial *material, const SceneManager *parent=nullptr): Primitives(material, parent), radius_(radius) {}

    bool hit(const Ray& ray, Interval rayTime, HitRecord& rec) const override {
        gm::IVec3f oc = ray.origin - position_;
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

        gm::IVec3f outwardNormal = (rec.point - position_) / radius_; 
        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material_;
        rec.object = this;
        return true;
    }

    std::string typeString() const override { return "Sphere"; }
};

class PlaneObject : public Primitives {
    gm::IPoint3 point_;
    gm::IVec3f normal_;

public:
    PlaneObject
    (
        const gm::IPoint3 point, gm::IVec3f normal,
        RTMaterial *material, const SceneManager *parent=nullptr
    ):
        Primitives(material, parent), point_(point), normal_(normal.normalized()) {}

    bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const override {
        double time = dot(normal_, point_ - ray.origin) * (1.0 / dot(normal_, ray.direction));

        if (!rayTime.surrounds(time)) return false;

        hitRecord.material = material_;
        hitRecord.object = this;
        hitRecord.time = time;
        hitRecord.point = ray.origin + ray.direction * time;
        
        gm::IVec3f outwardNormal = normal_;
        if (dot(normal_, ray.direction) < 0) outwardNormal = normal_ * (-1);  

        hitRecord.setFaceNormal(ray, outwardNormal);
         

        return true;
    }

    std::string typeString() const override { return "Plane"; }
};




class Light {
    gm::IVec3f ambientIntensity_;
    gm::IVec3f defuseIntensity_;
    gm::IVec3f specularIntensity_;
    gm::IPoint3 center_;
    double viewLightPow_;

    const SceneManager *parent_;

public:
    Light
    (
        const gm::IVec3f ambientIntensity,
        const gm::IVec3f defuseIntensity,
        const gm::IVec3f specularIntensity,
        double viewLightPow,
        const SceneManager *parent=nullptr
    ) : 
    ambientIntensity_(ambientIntensity), 
    defuseIntensity_(defuseIntensity), 
    specularIntensity_(specularIntensity),
    viewLightPow_(viewLightPow),
    parent_(parent) {}

    virtual RTColor getDirectLighting(const gm::IVec3f toView, const HitRecord &rec, bool hitted) 
    {
        gm::IVec3f toLight = (center_ - rec.point).normalized();
        
        gm::IVec3f ambientIntensity  = ambientIntensity_ * rec.material->diffuse();
        gm::IVec3f defuseIntensity   = defuseLightIntensity(toLight, rec.normal) * rec.material->diffuse();
        gm::IVec3f specularIntensity = specularLightIntesity(toLight, toView.normalized(), rec.normal);
        double    shadowFactor      = (hitted ? 0.0 : 1.0);

        return ambientIntensity + (defuseIntensity + specularIntensity) * shadowFactor;
    }

    gm::IPoint3 center() const { return center_; }
    const SceneManager *parent() const { return parent_; }
    void setParent(const SceneManager *parent) { parent_ = parent; }
    void setPosition(const gm::IPoint3 center) { center_ = center; }
    virtual std::string typeString() const { return "Light"; }

private:
    // Lambert
    gm::IVec3f defuseLightIntensity(const gm::IVec3f toLight, const gm::IVec3f surfNormal) {
        double lightRefAngleCos = dot(toLight.normalized(), surfNormal.normalized());

        if (lightRefAngleCos > 0)
            return defuseIntensity_ * lightRefAngleCos;
        return gm::IVec3f(0.0, 0.0, 0.0);
    }

    // Phong
    gm::IVec3f specularLightIntesity(const gm::IVec3f toLight, const gm::IVec3f toView, const gm::IVec3f surfNormal) {
        gm::IVec3f surfToReflLight = toLight - getOrtogonal(toLight, surfNormal) * 2;
        
        double view_angle_cos = dot(surfToReflLight.normalized(), toView.normalized());
        
        if (view_angle_cos > 0) {
            double resCoef = std::pow(view_angle_cos, viewLightPow_);
            return gm::IVec3f(resCoef); 
        }

        return gm::IVec3f(0, 0, 0);        
    }
};


#endif // RTOBJECTS_H