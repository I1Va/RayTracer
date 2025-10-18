#include <iostream>

#include "RTObjects.h"
#include "RayTracer.h"
#include "Camera.h"


SceneManager::~SceneManager() {
    for (Primitives *object : Primitivess_)
        delete object;
}

const std::vector<Light *> &SceneManager::inderectLightSources() const {
    return inderectLightSources_;
}

void SceneManager::addObject(const GmPoint<double, 3> &position, Primitives *object) {
    assert(object);

    if (object->parent_ != this) {
        std::cerr << "addObject failed : object parent != this\n";
        assert(0);
    }
    
    object->parent_ = this;
    object->position_ = position;
    Primitivess_.push_back(object);
}

bool SceneManager::hitClosest(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const {
    HitRecord tempRecord = {};
    bool hitAnything = false;
    double closestTime = rayTime.max;

    for (Primitives *object: Primitivess_) {
        if (object->hit(ray, Interval(rayTime.min, closestTime), tempRecord)) {
            hitAnything = true;
            closestTime = tempRecord.time;
            hitRecord = tempRecord;
        }
    }

    return hitAnything;
}

void SceneManager::render(Camera &camera) {
    update();
    camera.render(*this);
}

void SceneManager::update() {}