#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "RTObjects.h"
class Camera;


class SceneManager {
    std::vector<Primitives *> Primitivess_;

    std::vector<Light *> inderectLightSources_;
public:
    SceneManager() = default;

    ~SceneManager();

    void addObject(const GmPoint<double, 3> position, Primitives *object);
    void addLight(const GmPoint<double, 3> position, Light *light);

    bool hitClosest(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const;

    const std::vector<Light *> &inderectLightSources() const;

    void render(Camera &camera);

    private:
        void update();
};


#endif // RAY_TRACER_H