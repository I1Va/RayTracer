#ifndef RTMATERIAL_H
#define RTMATERIAL_H
#include <memory>
#include <ostream>
#include <istream>
#include <sstream>

#include "RTGeometry.h"
using RTColor = gm::IVec3f;


struct RTMaterial {
    virtual ~RTMaterial() = default;

    gm::IVec3f diffuse_ = gm::IVec3f(0, 0, 0);
    gm::IVec3f specular_ = gm::IVec3f(0, 0, 0);
    gm::IVec3f emission_ = gm::IVec3f(0, 0, 0);

    virtual bool scatter(
        const Ray& inRay,
        const HitRecord &hitRecord,
        gm::IVec3f &attenuation,
        Ray& scattered
    ) const = 0;

    virtual bool hasSpecular() const { return false; }
    virtual bool hasDiffuse() const { return false; }
    virtual bool hasEmmision() const { return false; }

    virtual std::string typeString() const { return "RTMaterial"; }

    gm::IVec3f &diffuse()  { return diffuse_; }
    gm::IVec3f &specular() { return specular_; }
    gm::IVec3f &emitted()  { return emission_; }
    const gm::IVec3f &diffuse()  const { return diffuse_; }
    const gm::IVec3f &specular() const { return specular_; }
    const gm::IVec3f &emitted()  const { return emission_; }

protected:
    virtual std::ostream &dump(std::ostream &os) const {
        os << typeString() << ' '
           << diffuse_.x()  << ' ' << diffuse_.y()  << ' ' << diffuse_.z()  << ' '
           << specular_.x() << ' ' << specular_.y() << ' ' << specular_.z() << ' '
           << emission_.x() << ' ' << emission_.y() << ' ' << emission_.z();
        return os;
    }

    virtual std::istream &scan(std::istream &is) {
        float dx, dy, dz;
        float sx, sy, sz;
        float ex, ey, ez;
        is >> dx >> dy >> dz >> sx >> sy >> sz >> ex >> ey >> ez;
        diffuse_  = gm::IVec3f(dx, dy, dz);
        specular_ = gm::IVec3f(sx, sy, sz);
        emission_ = gm::IVec3f(ex, ey, ez);
        return is;
    }

    friend inline std::ostream &operator<<(std::ostream &os, const RTMaterial &m);
    friend inline std::istream &operator>>(std::istream &is, RTMaterial &m);
};

class RTLambertian : public RTMaterial {
public:
    explicit RTLambertian(const gm::IVec3f& diffuse) { diffuse_ = diffuse; }

    bool scatter(const Ray&,
                 const HitRecord &hitRecord,
                 gm::IVec3f &attenuation,
                 Ray& scattered) const override
    {
        gm::IVec3f scatterDir = hitRecord.normal + gm::IVec3f::randomUnit();

        if (scatterDir.nearZero())
            scatterDir = hitRecord.normal;

        scattered = Ray(hitRecord.point, scatterDir);
        attenuation = diffuse_;
        return true;
    }

    std::string typeString() const override { return "Lambertian"; }

    bool hasDiffuse() const override { return true; }

protected:
    std::ostream &dump(std::ostream &os) const override {
        RTMaterial::dump(os);
        return os;
    }

    std::istream &scan(std::istream &is) override {
        RTMaterial::scan(is);
        return is;
    }
};

class RTMetal : public RTMaterial {

    double fuzz_ = 0.0;

public:
    RTMetal(const RTColor& specularColor, double fuzz) : fuzz_(fuzz < 1 ? fuzz : 1) { specular_ = specularColor; }

    bool scatter(const Ray& inRay,
                const HitRecord &hitRecord,
                gm::IVec3f &attenuation,
                Ray& scattered) const override
    {
        gm::IVec3f reflected = reflect(inRay.direction, hitRecord.normal);
        reflected = reflected.normalized() + gm::IVec3f::randomUnit() * fuzz_;
        scattered = Ray(hitRecord.point, reflected);
        attenuation = specular_;
        return true;
    }

    std::string typeString() const override { return "Metal"; }

    bool hasSpecular() const override { return true; }

protected:
    std::ostream &dump(std::ostream &os) const override {
        RTMaterial::dump(os);
        os << ' ' << fuzz_;
        return os;
    }

    std::istream &scan(std::istream &is) override {
        RTMaterial::scan(is);
        is >> fuzz_;
        return is;
    }
};

class RTDielectric : public RTMaterial {
    gm::IVec3f specular_local_;
    double refractionIndex_ = 1.0;

public:
    RTDielectric(const gm::IVec3f specular, const double refractionIndex) : specular_local_(specular), refractionIndex_(refractionIndex) {}

    bool scatter(const Ray& inRay,
                const HitRecord &hitRecord,
                gm::IVec3f &attenuation,
                Ray& scattered) const override
    {
        attenuation = specular_local_;

        double eta = hitRecord.frontFace ? (1.0 / refractionIndex_) : refractionIndex_;

        gm::IVec3f unitDir = inRay.direction.normalized();
        double cosTheta = std::fmin(dot(unitDir * (-1), hitRecord.normal), 1.0);
        double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));

        bool cannotRefract = eta * sinTheta > 1.0;

        gm::IVec3f direction;
        if (cannotRefract || reflectance(cosTheta, eta) > gm::randomDouble()) {
            direction = reflect(unitDir, hitRecord.normal);
        } else {
            gm::IVec3f refr;
            direction = refract(unitDir, hitRecord.normal, eta).normalized();
        }

        constexpr double ORIGIN_EPS = 1e-4;
        gm::IPoint3 newOrigin = hitRecord.point + hitRecord.normal * (hitRecord.frontFace ? ORIGIN_EPS : -ORIGIN_EPS);

        scattered = Ray(newOrigin, direction.normalized());
        return true;
    }

    std::string typeString() const override { return "Dielectric"; }

    bool hasSpecular() const override { return true; }

protected:
    std::ostream &dump(std::ostream &os) const override {
        RTMaterial::dump(os);
        os << ' ' << specular_local_.x() << ' ' << specular_local_.y() << ' ' << specular_local_.z()
           << ' ' << refractionIndex_;
        return os;
    }

    std::istream &scan(std::istream &is) override {
        RTMaterial::scan(is);
        float sx, sy, sz;
        is >> sx >> sy >> sz >> refractionIndex_;
        specular_local_ = gm::IVec3f(sx, sy, sz);
        return is;
    }

private:
    static double reflectance(double cosine, double refractionIndex) {
        auto r0 = (1 - refractionIndex) / (1 + refractionIndex);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
};

class RTEmissive : public RTMaterial {

public:
    explicit RTEmissive(const gm::IVec3f& emission) { emission_ = emission; }

    bool scatter(const Ray&, const HitRecord&, gm::IVec3f&, Ray&) const override {
        return false;
    }

    std::string typeString() const override { return "Emissive"; }

protected:
    std::ostream &dump(std::ostream &os) const override {
        RTMaterial::dump(os);
        return os;
    }

    std::istream &scan(std::istream &is) override {
        RTMaterial::scan(is);
        return is;
    }
};

class RTMaterialManager {
    std::vector<std::unique_ptr<RTMaterial>> children;

public:
    RTMaterialManager() = default;
    ~RTMaterialManager() = default;

    RTMaterial *MakeLambertian(const gm::IVec3f& diffuse) {
        children.push_back(std::make_unique<RTLambertian>(diffuse));
        return children.back().get();
    }

    RTMaterial *MakeMetal(const RTColor &specularColor, double fuzz) {
        children.push_back(std::make_unique<RTMetal>(specularColor, fuzz));
        return children.back().get();
    }

    RTMaterial *MakeDielectric(gm::IVec3f specular, double refractionIndex) {
        children.push_back(std::make_unique<RTDielectric>(specular, refractionIndex));
        return children.back().get();
    }

    RTMaterial *MakeEmissive(const gm::IVec3f &emission) {
        children.push_back(std::make_unique<RTEmissive>(emission));
        return children.back().get();
    }

    RTMaterial *deserializeMaterial(std::istream &iss) {
        std::string type;
        iss >> type;

        std::unique_ptr<RTMaterial> mat;
        std::cout << "type : " << type << "\n";

        if (type == "Lambertian") {
            mat = std::make_unique<RTLambertian>(gm::IVec3f{});
        } else if (type == "Metal") {
            mat = std::make_unique<RTMetal>(gm::IVec3f{}, 0.0);
        } else if (type == "Dielectric") {
            mat = std::make_unique<RTDielectric>(gm::IVec3f{}, 1.0);
        } else if (type == "Emissive") {
            mat = std::make_unique<RTEmissive>(gm::IVec3f{});
        } else {
            return nullptr;
        }

        iss >> *mat;

        children.push_back(std::move(mat));
        return children.back().get();
    }
};

inline std::ostream &operator<<(std::ostream &os, const RTMaterial &m) {
    return m.dump(os);
}
inline std::istream &operator>>(std::istream &is, RTMaterial &m) {
    return m.scan(is);
}

#endif // RTMATERIAL_H
