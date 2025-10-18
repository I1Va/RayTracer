#ifndef RTMATERIAL_H
#define RTMATERIAL_H


#include "RTGeometry.h"
#include "Utilities.h"
using RTColor = GmVec<double, 3>;


struct RTMaterial {
    virtual ~RTMaterial() = default;

    virtual bool scatter(
        const Ray& inRay,
        const HitRecord &hitRecord,
        GmVec<double,3> &attenuation,
        Ray& scattered
    ) const = 0;

    virtual GmVec<double,3> emitted() const { return {0.0, 0.0, 0.0}; }
    virtual GmVec<double,3> albedo() const { return {0.0, 0.0, 0.0}; }
};

class RTLambertian : public RTMaterial {
    GmVec<double,3> albedo_; 

public:
    explicit RTLambertian(const GmVec<double,3>& albedo)
        : albedo_(albedo) {}

    bool scatter(const Ray& inRay,
                 const HitRecord &hitRecord,
                 GmVec<double,3> &attenuation,
                 Ray& scattered) const override
    {
        GmVec<double,3> scatterDir = hitRecord.normal + randomUnitVector();

        if (scatterDir.nearZero())
            scatterDir = hitRecord.normal;

        scattered = Ray(hitRecord.point, scatterDir.normalized());
        attenuation = albedo_;
        return true;
    }

    GmVec<double,3> albedo() const override { return albedo_; }
};

class RTMetal : public RTMaterial {
    GmVec<double,3> specularColor_; 
    double fuzz_;

public:
    RTMetal(const RTColor& specularColor, double fuzz) : specularColor_(specularColor), fuzz_(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const Ray& inRay,
                const HitRecord &hitRecord,
                GmVec<double,3> &attenuation,
                Ray& scattered) const override
    {
        GmVec<double,3> reflected = reflect(inRay.direction, hitRecord.normal);
        reflected = reflected.normalized() + randomUnitVector() * fuzz_;
        scattered = Ray(hitRecord.point, reflected);
        attenuation = specularColor_;
        return true;
    }

    GmVec<double,3> albedo() const override { return {0.0, 0.0, 0.0}; }
};

class RTDielectric : public RTMaterial {
    GmVec<double, 3> albedo_;
    double refractionIndex_;
    
public:
    RTDielectric(const GmVec<double, 3> albedo, const double refractionIndex) : albedo_(albedo), refractionIndex_(refractionIndex) {}

    bool scatter(const Ray& inRay,
                const HitRecord &hitRecord,
                GmVec<double,3> &attenuation,
                Ray& scattered) const override
    {
        attenuation = albedo_;

        double eta = hitRecord.frontFace ? (1.0 / refractionIndex_) : refractionIndex_;

        GmVec<double,3> unitDir = inRay.direction.normalized();
        double cosTheta = std::fmin(dot(unitDir * (-1), hitRecord.normal), 1.0);
        double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));

        bool cannotRefract = eta * sinTheta > 1.0;

        GmVec<double,3> direction;
        if (cannotRefract || reflectance(cosTheta, eta) > randomDouble()) {
            direction = reflect(unitDir, hitRecord.normal);
        } else {
            GmVec<double,3> refr;
            direction = refract(unitDir, hitRecord.normal, eta).normalized();
        }

        constexpr double ORIGIN_EPS = 1e-4;
        GmPoint<double,3> newOrigin = hitRecord.point + hitRecord.normal * (hitRecord.frontFace ? ORIGIN_EPS : -ORIGIN_EPS);

        scattered = Ray(newOrigin, direction.normalized());
        return true;
    }

    GmVec<double,3> albedo() const override { return albedo_; }

private:
    static double reflectance(double cosine, double refractionIndex) {   
        auto r0 = (1 - refractionIndex) / (1 + refractionIndex);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
};

class RTEmissive : public RTMaterial {
    GmVec<double,3> emission_;
public:
    explicit RTEmissive(const GmVec<double,3>& emission)
        : emission_(emission) {}

    bool scatter(const Ray&, const HitRecord&, GmVec<double,3>&, Ray&) const override {
        return false;
    }

    GmVec<double,3> emitted() const override {
        return emission_;
    }
};


#endif // #define RTMATERIAL_H