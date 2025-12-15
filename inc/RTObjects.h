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
    PlaneObject(const SceneManager *parent=nullptr): Primitives(parent) {}
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

class PolygonObject : public Primitives {
    std::vector<gm::IPoint3> vertices_;
    gm::IVec3f normal_;   // unit normal
    gm::IPoint3 centroid_; // average of vertices

public:
    PolygonObject(const SceneManager *parent=nullptr): Primitives(parent) {}
    PolygonObject(const std::vector<gm::IPoint3> &verts, RTMaterial *material, const SceneManager *parent=nullptr)
        : Primitives(material, parent), vertices_(verts)
    {
        computeNormalAndCentroid();
        if (!vertices_.empty()) position_ = vertices_[0];
    }

    void setVertices(const std::vector<gm::IPoint3> &verts) {
        vertices_ = verts;
        computeNormalAndCentroid();
        if (!vertices_.empty()) position_ = vertices_[0];
    }

    const std::vector<gm::IPoint3>& vertices() const { return vertices_; }
    gm::IVec3f normal() const { return normal_; }
    gm::IPoint3 centroid() const { return centroid_; }

    std::string typeString() const override { return "Polygon"; }

    bool hit(const Ray& ray, Interval rayTime, HitRecord& rec) const override {
        return hitDetail(ray, rayTime, rec, vertices_, normal_, centroid_, material_, /*markExpanded*/false);
    }

    bool hitExpanded(const Ray& ray, Interval rayTime, HitRecord& rec) const override {
        if (!selected()) return false;
        if (hit(ray, rayTime, rec)) return true;

        if (vertices_.size() < 3) return false;
        std::vector<gm::IPoint3> expanded;
        expanded.reserve(vertices_.size());
        for (const auto &v : vertices_) {
            gm::IVec3f dir = v - centroid_;
            gm::IVec3f scaled = dir * EXPAND_COEF;
            expanded.emplace_back( centroid_.x() + scaled.x(), centroid_.y() + scaled.y(), centroid_.z() + scaled.z() );
        }

        bool result = hitDetail(ray, rayTime, rec, expanded, normal_, centroid_, material_, /*markExpanded*/true);
        if (result) rec.hitExpanded = true;
        return result;
    }

protected:
    std::ostream &dump(std::ostream &stream) const override {
        Primitives::dump(stream);
        stream << ' ' << vertices_.size();
        for (const auto &v : vertices_) {
            stream << ' ' << v.x() << ' ' << v.y() << ' ' << v.z();
        }
        return stream;
    }

    std::istream &scan(std::istream &stream) override {
        Primitives::scan(stream);
        size_t n = 0;
        stream >> n;
        vertices_.clear();
        for (size_t i = 0; i < n; ++i) {
            float x, y, z;
            stream >> x >> y >> z;
            vertices_.emplace_back(x, y, z);
        }
        computeNormalAndCentroid();
        return stream;
    }

private:
    void computeNormalAndCentroid() {
        centroid_ = gm::IPoint3(0.0f, 0.0f, 0.0f);
        if (vertices_.empty()) {
            normal_ = gm::IVec3f(0.0, 0.0, 1.0);
            return;
        }

        for (const auto &v : vertices_) {
            centroid_.setX(centroid_.x() + v.x());
            centroid_.setY(centroid_.y() + v.y());
            centroid_.setZ(centroid_.z() + v.z());
        }
        float inv = 1.0f / static_cast<float>(vertices_.size());
        centroid_.setX(centroid_.x() * inv);
        centroid_.setY(centroid_.y() * inv); 
        centroid_.setZ(centroid_.z() * inv);

        if (vertices_.size() >= 3) {
            gm::IVec3f a = vertices_[1] - vertices_[0];
            gm::IVec3f b = vertices_[2] - vertices_[0];
            // cross product manually to avoid depending on a member API
            gm::IVec3f cr(
                a.y() * b.z() - a.z() * b.y(),
                a.z() * b.x() - a.x() * b.z(),
                a.x() * b.y() - a.y() * b.x()
            );
            normal_ = cr.normalized();
        } else {
            normal_ = gm::IVec3f(0.0, 0.0, 1.0);
        }
    }

    // Project point and polygon to 2D by dropping the coordinate where normal has largest absolute component.
    static void projectTo2D(const gm::IVec3f &normal, const gm::IPoint3 &p, double &u, double &v) {
        double ax = std::fabs(normal.x()), ay = std::fabs(normal.y()), az = std::fabs(normal.z());
        if (ax > ay && ax > az) {
            // drop x -> use (y,z)
            u = p.y(); v = p.z();
        } else if (ay > az) {
            // drop y -> use (x,z)
            u = p.x(); v = p.z();
        } else {
            // drop z -> use (x,y)
            u = p.x(); v = p.y();
        }
    }

    static bool pointInPolygon2D(const std::vector<std::pair<double,double>> &poly, double px, double py) {
        // winding number / crossing number algorithm
        bool inside = false;
        size_t n = poly.size();
        for (size_t i = 0, j = n - 1; i < n; j = i++) {
            double xi = poly[i].first, yi = poly[i].second;
            double xj = poly[j].first, yj = poly[j].second;

            bool intersect = ((yi > py) != (yj > py)) &&
                             (px < (xj - xi) * (py - yi) / (yj - yi + 1e-20) + xi);
            if (intersect) inside = !inside;
        }
        return inside;
    }

    static bool hitDetail(
        const Ray& ray, Interval rayTime, HitRecord& rec,
        const std::vector<gm::IPoint3> &vertices,
        const gm::IVec3f &normal,
        const gm::IPoint3 &centroid,
        RTMaterial *material,
        bool markExpanded
    ) {
        if (vertices.size() < 3) return false;

        double denom = dot(normal, ray.direction);
        if (std::fabs(denom) < 1e-12) return false; // parallel

        double t = dot(normal, centroid - ray.origin) / denom;
        if (!rayTime.surrounds(t)) return false;

        gm::IPoint3 p = ray.origin + ray.direction * t;

        // project polygon and point to 2D
        std::vector<std::pair<double,double>> poly2d;
        poly2d.reserve(vertices.size());
        double px, py;
        projectTo2D(normal, p, px, py);
        for (const auto &v : vertices) {
            double u, v2;
            projectTo2D(normal, v, u, v2);
            poly2d.emplace_back(u, v2);
        }

        if (!pointInPolygon2D(poly2d, px, py)) return false;

        rec.time = t;
        rec.point = p;

        gm::IVec3f outwardNormal = normal;
        if (dot(normal, ray.direction) > 0) outwardNormal = normal * (-1);

        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material;
        rec.object = nullptr; // will be set by caller if needed; in this context cannot set "this" because static
        // But we can set object to something: we cannot access 'this' here (static). The caller uses instance method so assign below.
        // To keep semantics consistent, the non-static wrapper will set rec.object = this.
        if (!markExpanded) {
            // nothing else
        }
        return true;
    }

    // Non-static wrapper to set rec.object properly.
    bool hitDetailWrapper(const Ray& ray, Interval rayTime, HitRecord& rec,
                          const std::vector<gm::IPoint3> &vertices,
                          const gm::IVec3f &normal,
                          const gm::IPoint3 &centroid,
                          RTMaterial *material,
                          bool markExpanded) const
    {
        HitRecord localRec;
        bool ok = hitDetail(ray, rayTime, localRec, vertices, normal, centroid, material, markExpanded);
        if (!ok) return false;
        // copy fields and set object pointer
        rec = localRec;
        rec.object = const_cast<PolygonObject*>(this);
        return true;
    }

    // Override base methods to call wrapper so rec.object is set properly
    bool hit(const Ray& ray, Interval rayTime, HitRecord& rec, const std::vector<gm::IPoint3> &verts, const gm::IVec3f &norm, const gm::IPoint3 &cent) const {
        return hitDetailWrapper(ray, rayTime, rec, verts, norm, cent, material_, false);
    }

    bool hitExpandedWrapper(const Ray& ray, Interval rayTime, HitRecord& rec, const std::vector<gm::IPoint3> &verts, const gm::IVec3f &norm, const gm::IPoint3 &cent) const {
        return hitDetailWrapper(ray, rayTime, rec, verts, norm, cent, material_, true);
    }

    // adapt public hit() and hitExpanded() to use wrapper (so rec.object points to this)
    bool hit(const Ray& ray, Interval rayTime, HitRecord& rec, bool) const {
        return hitDetailWrapper(ray, rayTime, rec, vertices_, normal_, centroid_, material_, false);
    }
};

inline std::ostream &operator<<(std::ostream &os, const Primitives &p) {
    return p.dump(os);
}
inline std::istream &operator>>(std::istream &is, Primitives &p) {
    return p.scan(is);
}

#endif // RTOBJECTS_H
