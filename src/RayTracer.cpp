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

void SceneManager::addObject(gm::IPoint3 position, Primitives *object) {
    assert(object);

    if (object->parent_ != this && object->parent_ != nullptr) 
        assert(0 && "addObject failed : object parent != this or nullptr");
    
    object->parent_ = this;
    object->position_ = position;
    Primitivess_.push_back(object);
}

void SceneManager::addLight(gm::IPoint3 position, Light *object) {
    assert(object);

    if (object->parent() != this && object->parent() != nullptr) 
        assert(0 && "addObject failed : object parent != this or nullptr");
    
    object->setParent(this);
    object->setPosition(position);
    inderectLightSources_.push_back(object);
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