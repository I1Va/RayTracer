#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "RTObjects.h"
class Camera;


class SceneManager {
    std::vector<SceneObject *> sceneObjects_;
public:
    SceneManager() = default;

    ~SceneManager();

    void addObject(const GmPoint<double, 3> &position, SceneObject *object);

    bool hitClosest(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const;

    void render(Camera &camera);

    private:
        void update();
};


#endif // RAY_TRACER_H