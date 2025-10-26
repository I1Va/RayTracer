#ifndef RTMATERIAL_H
#define RTMATERIAL_H


#include "RTGeometry.h"
#include "Utilities.h"
using RTColor = gm::IVec3;


struct RTMaterial {
    virtual ~RTMaterial() = default;

    virtual bool scatter(
        const Ray& inRay,
        const HitRecord &hitRecord,
        gm::IVec3 &attenuation,
        Ray& scattered
    ) const = 0;

    virtual gm::IVec3 emitted() const { return {0.0, 0.0, 0.0}; }
    virtual gm::IVec3 albedo() const { return {0.0, 0.0, 0.0}; }
};

class RTLambertian : public RTMaterial {
    gm::IVec3 albedo_; 

public:
    explicit RTLambertian(const gm::IVec3& albedo)
        : albedo_(albedo) {}

    bool scatter(const Ray& inRay,
                 const HitRecord &hitRecord,
                 gm::IVec3 &attenuation,
                 Ray& scattered) const override
    {
        gm::IVec3 scatterDir = hitRecord.normal + randomUnitVector();

        if (scatterDir.nearZero())
            scatterDir = hitRecord.normal;

        scattered = Ray(hitRecord.point, scatterDir.normalized());
        attenuation = albedo_;
        return true;
    }

    gm::IVec3 albedo() const override { return albedo_; }
};

class RTMetal : public RTMaterial {
    gm::IVec3 specularColor_; 
    double fuzz_;

public:
    RTMetal(const RTColor& specularColor, double fuzz) : specularColor_(specularColor), fuzz_(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const Ray& inRay,
                const HitRecord &hitRecord,
                gm::IVec3 &attenuation,
                Ray& scattered) const override
    {
        gm::IVec3 reflected = reflect(inRay.direction, hitRecord.normal);
        reflected = reflected.normalized() + randomUnitVector() * fuzz_;
        scattered = Ray(hitRecord.point, reflected);
        attenuation = specularColor_;
        return true;
    }

    gm::IVec3 albedo() const override { return {0.0, 0.0, 0.0}; }
};

class RTDielectric : public RTMaterial {
    gm::IVec3 albedo_;
    double refractionIndex_;
    
public:
    RTDielectric(const gm::IVec3 albedo, const double refractionIndex) : albedo_(albedo), refractionIndex_(refractionIndex) {}

    bool scatter(const Ray& inRay,
                const HitRecord &hitRecord,
                gm::IVec3 &attenuation,
                Ray& scattered) const override
    {
        attenuation = albedo_;

        double eta = hitRecord.frontFace ? (1.0 / refractionIndex_) : refractionIndex_;

        gm::IVec3 unitDir = inRay.direction.normalized();
        double cosTheta = std::fmin(dot(unitDir * (-1), hitRecord.normal), 1.0);
        double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));

        bool cannotRefract = eta * sinTheta > 1.0;

        gm::IVec3 direction;
        if (cannotRefract || reflectance(cosTheta, eta) > gm::randomDouble()) {
            direction = reflect(unitDir, hitRecord.normal);
        } else {
            gm::IVec3 refr;
            direction = refract(unitDir, hitRecord.normal, eta).normalized();
        }

        constexpr double ORIGIN_EPS = 1e-4;
        gm::IPoint3 newOrigin = hitRecord.point + hitRecord.normal * (hitRecord.frontFace ? ORIGIN_EPS : -ORIGIN_EPS);

        scattered = Ray(newOrigin, direction.normalized());
        return true;
    }

    gm::IVec3 albedo() const override { return albedo_; }

private:
    static double reflectance(double cosine, double refractionIndex) {   
        auto r0 = (1 - refractionIndex) / (1 + refractionIndex);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
};

class RTEmissive : public RTMaterial {
    gm::IVec3 emission_;
public:
    explicit RTEmissive(const gm::IVec3& emission)
        : emission_(emission) {}

    bool scatter(const Ray&, const HitRecord&, gm::IVec3&, Ray&) const override {
        return false;
    }

    gm::IVec3 emitted() const override {
        return emission_;
    }
};


#endif // #define RTMATERIAL_H