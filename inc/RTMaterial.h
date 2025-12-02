#ifndef RTMATERIAL_H
#define RTMATERIAL_H


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
};

class RTLambertian : public RTMaterial {
public:
    explicit RTLambertian(const gm::IVec3f& diffuse) {diffuse_ = diffuse; }

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
};

class RTMetal : public RTMaterial {
   
    double fuzz_;

public:
    RTMetal(const RTColor& specularColor, double fuzz) : fuzz_(fuzz < 1 ? fuzz : 1) {specular_ = specularColor; }

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
};

class RTDielectric : public RTMaterial {
    gm::IVec3f specular_;
    double refractionIndex_;
    
public:
    RTDielectric(const gm::IVec3f specular, const double refractionIndex) : specular_(specular), refractionIndex_(refractionIndex) {}

    bool scatter(const Ray& inRay,
                const HitRecord &hitRecord,
                gm::IVec3f &attenuation,
                Ray& scattered) const override
    {
        attenuation = specular_;

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
};


#endif // #define RTMATERIAL_H