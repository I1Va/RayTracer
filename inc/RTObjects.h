#ifndef RTOBJECTS_H
#define RTOBJECTS_H

#include <vector>
#include <string>
#include <ostream>
#include <istream>
#include <cmath>
#include <cassert>

#include "Geom.hpp"
#include "IVec3f.hpp"
#include "RTMaterial.h"
#include "RTGeometry.h"

inline constexpr float SELECTED_DELTA_X = 0.1;
inline constexpr float SELECTED_DELTA_Y = 0.1;
inline constexpr float SELECTED_DELTA_Z = 0.1;

class SceneManager;

class Primitives {
protected:
    static constexpr double EXPAND_COEF = 1.05;

    const SceneManager *parent_{nullptr};
    RTMaterial *material_{nullptr};

    gm::IPoint3 position_{};
    bool selectFlag_ = false;

    Primitives(RTMaterial *material, const SceneManager *parent=nullptr): parent_(parent), material_(material) {
        assert(material);
    }
    Primitives(const SceneManager *parent=nullptr): parent_(parent) {}

    // dump/scan order: typeString position.x position.y position.z selected
    virtual std::ostream &dump(std::ostream &stream) const {
        stream 
        << typeString()   << ' '
        << position().x() << ' '
        << position().y() << ' '
        << position().z() << ' '
        << selected();
        return stream;
    }

    virtual std::istream &scan(std::istream &stream) {
        float x, y, z;
        bool sel;
        stream >> x >> y >> z >> sel;
        gm::IPoint3 pos(x, y, z);
        setPosition(pos);
        setSelectFlag(sel);
        return stream;
    }

    friend inline std::ostream &operator<<(std::ostream &os, const Primitives &p);
    friend inline std::istream &operator>>(std::istream &is, Primitives &p);

public:
    virtual ~Primitives() = default;

    virtual bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const = 0;
    virtual bool hitExpanded(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const = 0;

    virtual std::string typeString() const { return "Primitive"; }

    virtual void setPosition(const gm::IPoint3 position) { position_ = position; }
    virtual gm::IPoint3 position() const { return position_; }

    void setMaterial(RTMaterial *material) {
        material_ = material;
    }
    const RTMaterial* material() const { return material_; }
    RTMaterial* material() { return material_; }

    void setSelectFlag(bool val) {selectFlag_ = val; }
    bool selected() const { return selectFlag_; }

friend SceneManager;
};

class SphereObject : public Primitives {
    double radius_ = 0.0;

public:
    SphereObject(const SceneManager *parent=nullptr): Primitives(parent) {}
    SphereObject(double radius, RTMaterial *material, const SceneManager *parent=nullptr): Primitives(material, parent), radius_(radius) {}

    bool hit(const Ray& ray, Interval rayTime, HitRecord& rec) const override {
        return hitDetail(ray, rayTime, rec, radius_, position_, material_);
    }

    bool hitExpanded(const Ray& ray, Interval rayTime, HitRecord& rec) const override {
        if (!selected()) return false;
        if (hitDetail(ray, rayTime, rec, radius_, position_, material_)) {
            return true;
        }

        bool result = hitDetail(ray, rayTime, rec, radius_ * EXPAND_COEF, position_, material_);
        if (result) rec.hitExpanded = true;
        return result;
    }

    float getRadius() const { return radius_; }
    void setRadius(const float val) { radius_ = val; }

    std::string typeString() const override { return "Sphere"; }

protected:
 
    std::ostream &dump(std::ostream &stream) const override {
        Primitives::dump(stream);
        stream << ' ' << radius_;
        return stream;
    }

    std::istream &scan(std::istream &stream) override {
        Primitives::scan(stream);
        double r = 0.0;
        stream >> r;
        radius_ = r;
        return stream;
    }

private:
    bool hitDetail
    (
        const Ray& ray, Interval rayTime, HitRecord& rec,
        double radius, gm::IPoint3 position, RTMaterial *material
    ) const
    {
        gm::IVec3f oc = ray.origin - position;
        double a = dot(ray.direction, ray.direction);
        double half_b = dot(oc, ray.direction);
        double c = dot(oc, oc) - radius * radius;

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

        gm::IVec3f outwardNormal = (rec.point - position) / radius;
        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material;
        rec.object = this;
        return true;
    }
};

class PlaneObject : public Primitives {
    gm::IVec3f normal_;

public:
    PlaneObject
    (
        const gm::IPoint3 point, gm::IVec3f normal,
        RTMaterial *material, const SceneManager *parent=nullptr
    ):
        Primitives(material, parent), normal_(normal.normalized()) { position_ = point; }

    bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const override {
        double time = dot(normal_, position_ - ray.origin) * (1.0 / dot(normal_, ray.direction));

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

    virtual bool hitExpanded(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const override {
        return hit(ray, rayTime, hitRecord);
    }

    void setNormal(const gm::IVec3f normal) { normal_ = normal; }
    gm::IVec3f getNormal() const { return normal_; }

    std::string typeString() const override { return "Plane"; }

protected:
    std::ostream &dump(std::ostream &stream) const override {
        Primitives::dump(stream);
        stream << ' '
               << normal_.x() << ' '
               << normal_.y() << ' '
               << normal_.z();
        return stream;
    }

    std::istream &scan(std::istream &stream) override {
        Primitives::scan(stream);
        float nx, ny, nz;
        stream >> nx >> ny >> nz;
        normal_ = gm::IVec3f(nx, ny, nz);
        return stream;
    }
};

class Light {
    gm::IVec3f ambientIntensity_;
    gm::IVec3f defuseIntensity_;
    gm::IVec3f specularIntensity_;
    gm::IPoint3 position_;
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

    virtual ~Light() = default;

    virtual RTColor getDirectLighting(const gm::IVec3f toView, const HitRecord &rec, bool hitted)
    {
        gm::IVec3f toLight = (position_ - rec.point).normalized();

        gm::IVec3f ambientIntensity  = ambientIntensity_ * rec.material->diffuse();
        gm::IVec3f defuseIntensity   = defuseLightIntensity(toLight, rec.normal) * rec.material->diffuse();
        gm::IVec3f specularIntensity = specularLightIntesity(toLight, toView.normalized(), rec.normal);
        double    shadowFactor      = (hitted ? 0.0 : 1.0);

        return ambientIntensity + (defuseIntensity + specularIntensity) * shadowFactor;
    }

    void setPosition(const gm::IPoint3 position) { position_ = position; }
    gm::IPoint3 position() const { return position_; }

    const SceneManager *parent() const { return parent_; }
    void setParent(const SceneManager *parent) { parent_ = parent; }
    virtual std::string typeString() const { return "Light"; }

    std::ostream &dump(std::ostream &stream) const {
        stream
            << position_.x() << ' ' << position_.y() << ' ' << position_.z() << ' '
            << ambientIntensity_.x() << ' ' << ambientIntensity_.y() << ' ' << ambientIntensity_.z() << ' '
            << defuseIntensity_.x()  << ' ' << defuseIntensity_.y()  << ' ' << defuseIntensity_.z()  << ' '
            << specularIntensity_.x() << ' ' << specularIntensity_.y() << ' ' << specularIntensity_.z() << ' '
            << viewLightPow_;
        return stream;
    }

    std::istream &scan(std::istream &stream) {
        float px, py, pz;
        stream >> px >> py >> pz;
        position_ = gm::IPoint3(px, py, pz);

        float ax, ay, az;
        stream >> ax >> ay >> az;
        ambientIntensity_ = gm::IVec3f(ax, ay, az);

        float dx, dy, dz;
        stream >> dx >> dy >> dz;
        defuseIntensity_ = gm::IVec3f(dx, dy, dz);

        float sx, sy, sz;
        stream >> sx >> sy >> sz;
        specularIntensity_ = gm::IVec3f(sx, sy, sz);

        stream >> viewLightPow_;
        return stream;
    }

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

inline std::ostream &operator<<(std::ostream &os, const Primitives &p) {
    return p.dump(os);
}
inline std::istream &operator>>(std::istream &is, Primitives &p) {
    return p.scan(is);
}

#endif // RTOBJECTS_H
