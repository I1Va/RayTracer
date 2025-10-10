#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "Geom.h"
#include "Camera.h"
#include "Objects.h"

class SceneManager {
    std::vector<SceneObject *> sceneObjects_;

public:
    explicit SceneManager() {};
    ~SceneManager() {
        for (SceneObject *object : sceneObjects_) {
            delete object;
        }
    }

    bool addObject(const GmPoint<double, 3> &position, SceneObject *object) {
        assert(object);

        if (object->parent_ != this) {
            std::cerr << "addObject failed : object parent != this\n";
            return false;
        }
        
        object->parent_ = this;
        object->position_ = position;
        sceneObjects_.push_back(object);

        return true;
    }
    
    void render(Camera &camera) {
        update();

        computeCameraPixelBufer(camera);
    }

private:
    void update() {}

    void computeCameraPixelBufer(Camera &camera) {
        for (int pixelX = 0; pixelX < camera.screenResolution().first; pixelX++) {
            for (int pixelY = 0; pixelY < camera.screenResolution().second; pixelY++) {
                Ray ray = camera.genRay(pixelX, pixelY);
                camera.setPixel(pixelX, pixelY, rayColor(ray));
            }
        }
    }

    RTColor rayColor(const Ray& ray) {
        for (SceneObject *object : sceneObjects_) {
            if (object->rayHit(ray)) {
                return {1.0, 0.0, 0.0};
            }
        }

        double a = 0.5 * (ray.direction.normalized().x() + 1.0);
    
        return RTColor(1.0, 1.0, 1.0) * (1.0 - a) + RTColor(0.5, 0.7, 1.0) * a;
    }
};


#endif // RAY_TRACER_H