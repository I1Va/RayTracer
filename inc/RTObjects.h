#ifndef RTOBJECTS_H
#define RTOBJECTS_H

#include <vector>

#include "Geom.h"
#include "RTMaterial.h"
#include "RTGeometry.h"

class SceneManager;

struct Primitives {
    const RTMaterial *material_;
    const SceneManager *parent_;
    GmPoint<double, 3> position_; 

    Primitives(const RTMaterial *material, const SceneManager *parent=nullptr): material_(material), parent_(parent) {
        assert(material);
    }

public:
    virtual ~Primitives() = default;

    virtual bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const = 0;
};

class SphereObject : public Primitives {
    double radius_;

public:
    SphereObject(double radius, const RTMaterial *material, const SceneManager *parent=nullptr): Primitives(material, parent), radius_(radius) {}

    bool hit(const Ray& ray, Interval rayTime, HitRecord& rec) const override {
        GmVec<double,3> oc = ray.origin - position_; 
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

        GmVec<double,3> outwardNormal = (rec.point - position_) / radius_; 
        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material_;
        return true;
    }
};

class PlaneObject : public Primitives {
    GmPoint<double, 3> point_;
    GmVec<double, 3> normal_;

public:
    PlaneObject
    (
        const GmPoint<double, 3> point, GmVec<double, 3> normal,
        const RTMaterial *material, const SceneManager *parent=nullptr
    ):
        Primitives(material, parent), point_(point), normal_(normal.normalized()) {}

    bool hit(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const override {
        double time = dot(normal_, point_ - ray.origin) * (1.0 / dot(normal_, ray.direction));

        if (!rayTime.surrounds(time)) return false;

        hitRecord.material = material_;
        hitRecord.time = time;
        hitRecord.point = ray.origin + ray.direction * time;
        
        GmVec<double, 3> outwardNormal = normal_;
        if (dot(normal_, ray.direction) < 0) outwardNormal = normal_ * (-1);  

        hitRecord.setFaceNormal(ray, outwardNormal);

        return true;
    }
};






// // Lambert
// RTColor  get_defuse_light_intensity(RTColor sphere_to_light_vec, RTColor sphere_normal_vec, RTColor defuse_intensity) {
//     double light_ref_angle_cos = (!(sphere_to_light_vec)).scalar_product(!sphere_normal_vec);

//     if (light_ref_angle_cos > 0)
//         return defuse_intensity * light_ref_angle_cos;
//     return RTColor(0, 0, 0);
// }

// // Phong
// RTColor get_specular_intensity(
//     RTColor sphere_to_light_vec, RTColor sphere_point, RTColor sphere_center, RTColor camera_point, 
//     RTColor intensity, double view_light_pow) 
// {
//     RTColor sphere_to_refl_light_vec = sphere_to_light_vec - get_ortogonal(sphere_to_light_vec, sphere_point - sphere_center) * 2;
    
//     RTColor sphere_to_view_vec = camera_point - sphere_point;
//     double view_angle_cos = (!sphere_to_refl_light_vec).scalar_product(!sphere_to_view_vec);

//     if (view_angle_cos > 0) {
//         RTColor res_intensity = intensity * std::pow(view_angle_cos, view_light_pow); 
//         return res_intensity;
//     }
        
//     return RTColor(0, 0, 0);        
// }

// double get_shadow_factor(const RTColor &sphere_point, const int sphere_idx, const scene_manager &scene) {
//     gm_line<double, 3> sphere_to_light_ray(sphere_point, scene.get_light_src_center() - sphere_point);

//     int shadow_sphere_idx = -1;
//     RTColor shadow_sphere_point = RTColor::POISON();
//     bool intersection_state = scene.get_closest_sphere_intersection(sphere_to_light_ray, &shadow_sphere_point, &shadow_sphere_idx, sphere_idx);

//     if (shadow_sphere_idx == -1) {
//         return 1.0;
//     }

//     gm_sphere<double, 3> shadow_sphere = scene.get_sphere(shadow_sphere_idx).shape;
//     double shadow_distance = shadow_sphere.get_distance2_to_line(sphere_to_light_ray) / (shadow_sphere.get_radius() * shadow_sphere.get_radius());

//     return 1.0 / (1 + std::exp(-scene.get_shadow_coef() * (shadow_distance - 0.5)));
// }




class Light {
    GmVec<double, 3> ambientIntensity_;
    GmVec<double, 3> defuseIntensity_;
    GmVec<double, 3> specularIntensity_;
    GmVec<double, 3> center_;
    double viewLightPow_;

public:
    Light
    (
        const GmVec<double, 3> ambientIntensity,
        const GmVec<double, 3> defuseIntensity,
        const GmVec<double, 3> specularIntensity,
        const GmVec<double, 3> center,
        double viewLightPow
    ) : 
    ambientIntensity_(ambientIntensity), 
    defuseIntensity_(defuseIntensity), 
    specularIntensity_(specularIntensity),
    center_(center),
    viewLightPow_(viewLightPow) {}

    // virtual RTColor getDirectLighting(const GmPoint<double, 3> point, const GmVec<double, 3> normal, const RTMaterial *surfaceMaterial) {

    //     GmVec<double, 3> ambientIntensity = ambientIntensity_ * surfaceMaterial

    //     GmVec<double, 3> sphere_normal_vec = sphere_point - visual_sphere.shape.get_center();
    //     GmVec<double, 3> sphere_to_light_vec = scene.get_light_src_center() - sphere_point;
    //     GmVec<double, 3> defuse_intensity = cord_mul(
    //         get_defuse_light_intensity(sphere_to_light_vec, sphere_normal_vec, scene.get_defuse_intensity()),
    //         visual_sphere.color
    //     );

    //     GmVec<double, 3> specular_intensity = get_specular_intensity(sphere_to_light_vec, sphere_point, visual_sphere.shape.get_center(), scene.get_camera_pos(), 
    //                                                     scene.get_specular_intensity(), scene.get_view_light_pow());
    
    //     double shadow_factor = get_shadow_factor(sphere_point, intersection_sphere_idx, scene);  
            
    //     GmVec<double, 3> summary_intensity = 
    //         ambient_intensity + 
    //         (defuse_intensity + specular_intensity ) * shadow_factor;

    //     summary_intensity = summary_intensity.clamp(0.0, 1.0);


    //     draw_pixel(window_pixel_bufer, pixel, summary_intensity);
    // }





};


#endif // RTOBJECTS_H