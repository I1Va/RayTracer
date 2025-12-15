#include <iostream>

#include "RTObjects.h"
#include "RayTracer.h"
#include "Camera.h"


SceneManager::~SceneManager() {
    for (Primitives *object : primitives_)
        delete object;
    for (Light *object : directLightSources_)
        delete object;
}

const std::vector<Light *> &SceneManager::inderectLightSources() const {
    return directLightSources_;
}

void SceneManager::addObject(gm::IPoint3 position, Primitives *object) {
    assert(object);

    if (object->parent_ != this && object->parent_ != nullptr) 
        assert(0 && "addObject failed : object parent != this or nullptr");
    
    object->parent_ = this;
    object->position_ = position;
    primitives_.push_back(object);
}


void SceneManager::addLight(gm::IPoint3 position, Light *light) {
    assert(light);

    if (light->parent() != this && light->parent() != nullptr) 
        assert(0 && "addLight failed : light parent != this or nullptr");
    
    light->setParent(this);
    light->setPosition(position);
    directLightSources_.push_back(light);
}

void SceneManager::addObject(Primitives *object) {
    addObject(object->position(), object);
}
void SceneManager::addLight(Light *light) {
    addLight(light->position(), light);
}

void SceneManager::clear() {
    for (Primitives *object : primitives_)
        if (object) delete object;
    for (Light *object : directLightSources_)
        if (object) delete object;

    primitives_.clear();
    directLightSources_.clear();
}

bool SceneManager::hitClosest(const Ray& ray, Interval rayTime, HitRecord& hitRecord, bool hitExpandedState) const {
    HitRecord tempRec = {};
    double closestHitTime = rayTime.max;

    HitRecord expandedRec = {};
    double closestExpandedHitTime = rayTime.max;

    bool hitAnything = false;
    
    for (Primitives *object: primitives_) {
        if (object->hit(ray, Interval(rayTime.min, closestHitTime), tempRec)) {
            hitAnything = true;
            closestHitTime = tempRec.time;
        }
    }

    if (hitExpandedState) {
        for (Primitives *object: primitives_) {
            if (object->hitExpanded(ray, Interval(rayTime.min, closestExpandedHitTime), expandedRec)) {
                hitAnything = true;
                closestExpandedHitTime = expandedRec.time;
            }
        }
    }

    if (closestExpandedHitTime < closestHitTime) {
        hitRecord = expandedRec;
    } else {
        hitRecord = tempRec;
    }

    return hitAnything;
}