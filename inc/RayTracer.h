#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "RTObjects.h"
class Camera;


class SceneManager {
    std::vector<Primitives *> primitives_;
    std::vector<Light *> directLightSources_;
public:
    SceneManager() = default;

    ~SceneManager();

    void addObject(const gm::IPoint3 position, Primitives *object);
    void addLight(const gm::IPoint3 position, Light *light);
    void addObject(Primitives *object);
    void addLight(Light *light);


    bool hitClosest(const Ray& ray, Interval rayTime, HitRecord& hitRecord, bool hitExpandedState) const;

    const std::vector<Light *> &inderectLightSources() const;


    std::vector<Primitives *> &primitives() { return primitives_; }
    std::vector<Light *> &lights() { return directLightSources_; }
    const std::vector<Primitives *> &primitives() const { return primitives_; }
    const std::vector<Light *> &lights() const { return directLightSources_; }
};


#endif // RAY_TRACER_H