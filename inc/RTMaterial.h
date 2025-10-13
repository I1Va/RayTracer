#ifndef RTMATERIAL_H
#define RTMATERIAL_H


#include "RTGeometry.h"
#include "Utilities.h"

struct RTMaterial {
    virtual ~RTMaterial() = default;

    virtual bool scatter(
        const Ray& inRay,
        const GmPoint<double,3>& hitPoint,
        const GmVec<double,3>& normal,
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
                 const GmPoint<double,3>& hitPoint,
                 const GmVec<double,3>& normal,
                 GmVec<double,3>& attenuation,
                 Ray& scattered) const override
    {
        GmVec<double,3> scatterDir = normal + randomUnitVector();

        if (scatterDir.nearZero())
            scatterDir = normal;

        scattered = Ray(hitPoint, scatterDir.normalized());
        attenuation = albedo_;
        return true;
    }
};


#endif // #define RTMATERIAL_H