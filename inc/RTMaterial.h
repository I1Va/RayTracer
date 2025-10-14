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
        GmVec<double,3>& attenuation,
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
                 GmVec<double,3>& attenuation,
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
                GmVec<double,3>& attenuation,
                Ray& scattered) const override
    {
        GmVec<double,3> reflected = reflect(inRay.direction, hitRecord.normal);
        reflected = reflected.normalized() + randomUnitVector() * fuzz_;
        scattered = Ray(hitRecord.point, reflected);
        attenuation = albedo_;
        return true;
    }
};


#endif // #define RTMATERIAL_H