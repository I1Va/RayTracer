#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include "Geom.h"
#include "Camera.h"


class SceneObject {
    GmVec<double, 3> pos_;
};


class SceneManager {
    std::vector<SceneObject *> sceneObjects_;

public:
    explicit SceneManager() {};

    
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
        return {(uint8_t) 255, (uint8_t) (ray.direction.length2() * 100), (uint8_t) (ray.direction.length2() * 100), (uint8_t) (ray.direction.length2() * 100)};
    }
};


#endif // RAY_TRACER_H