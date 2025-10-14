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
};

struct RTLambertian : public RTMaterial {
    GmVec<double,3> albedo_; 

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
};

class RTMetal : public RTMaterial {
    GmVec<double,3> albedo_; 
    double fuzz_;

public:
    RTMetal(const RTColor& albedo, double fuzz) : albedo_(albedo), fuzz_(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const Ray& inRay,
                const HitRecord &hitRecord,
                GmVec<double,3> &attenuation,
                Ray& scattered) const override
    {
        GmVec<double,3> reflected = reflect(inRay.direction, hitRecord.normal);
        reflected = reflected.normalized() + randomUnitVector() * fuzz_;
        scattered = Ray(hitRecord.point, reflected);
        attenuation = albedo_;
        return true;
    }
};

class RTDielectric : public RTMaterial {
    double refractionIndex_;


public:
    RTDielectric(double refractionIndex) : refractionIndex_(refractionIndex) {}

    bool scatter(const Ray& inRay,
                const HitRecord &hitRecord,
                GmVec<double,3> &attenuation,
                Ray& scattered) const override
    {
        attenuation = {1.0, 1.0, 1.0};
        double refractionRatio = hitRecord.frontFace ? (1.0/refractionIndex_) : refractionIndex_;

        
        GmVec<double,3> unitDirection = inRay.direction.normalized();


        double cosTheta = std::fmin(dot(unitDirection * (-1), hitRecord.normal), 1.0);
        double sinTheta = std::sqrt(1.0 - cosTheta*cosTheta);

        bool cannotRefract = refractionRatio * sinTheta > 1.0;
    
        GmVec<double,3> direction = {};
        if (cannotRefract || reflectance(cosTheta, refractionRatio) > randomDouble())
            direction = reflect(unitDirection, hitRecord.normal);
        else
            direction = refract(unitDirection, hitRecord.normal, refractionRatio);

        scattered = Ray(hitRecord.point, direction.normalized());
        return true;
    }

private:
    // Schlick's approximation for reflectance.
    static double reflectance(double cosine, double refractionIndex) {   
        auto r0 = (1 - refractionIndex) / (1 + refractionIndex);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
};


#endif // #define RTMATERIAL_H