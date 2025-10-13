#include <iostream>

#include "Scene.h"
#include "RayTracer.h"
#include "Camera.h"


SceneManager::~SceneManager() {
    for (SceneObject *object : sceneObjects_)
        delete object;
}

void SceneManager::addObject(const GmPoint<double, 3> &position, SceneObject *object) {
    assert(object);

    if (object->parent_ != this) {
        std::cerr << "addObject failed : object parent != this\n";
        assert(0);
    }
    
    object->parent_ = this;
    object->position_ = position;
    sceneObjects_.push_back(object);
}

bool SceneManager::hitClosest(const Ray& ray, Interval rayTime, HitRecord& hitRecord) const {
    HitRecord tempRecord = {};
    bool hitAnything = false;
    double closestTime = rayTime.max;

    for (SceneObject *object: sceneObjects_) {
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