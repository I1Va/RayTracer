#ifndef CAMERA_H
#define CAMERA_H

#include <vector>

#include "RTGeometry.h"
#include "Scene.h"
class SceneManager;


using RTColor = GmVec<double, 3>;

const int CAMERA_SAMPLES_PER_PIXEL = 3;


struct Viewport {
    static constexpr const double VIEWPORT_WIDTH = 1;
    static constexpr const double VIEWPORT_HEIGHT = 1;
    GmPoint<double, 3> upperLeft_;
    GmVec<double, 3> rightDir_;
    GmVec<double, 3> downDir_;
};

class Camera {
    static constexpr const double FOCAL_LENGTH = 1;

    GmPoint<double, 3> center_ = {};
    GmVec<double, 3> direction_ = {};
    Viewport viewPort_ = {};

    std::pair<int, int> screenResolution_ = {};
    std::vector<RTColor> pixels_ = {};

    double pixelSamplesScale_ = 0;
    int samplesPerPixel_ = CAMERA_SAMPLES_PER_PIXEL;

public:
    Camera();
 
    Camera
    (
        const GmPoint<double, 3> &center, const GmVec<double, 3> &direction, const std::pair<int, int> &screenResolution, 
        const int samplesPerPixel=CAMERA_SAMPLES_PER_PIXEL
    );


    RTColor getRayColor(const Ray& ray, const SceneManager& sceneManager) const;

    Ray genRay(int pixelX, int pixelY);

    void render(const SceneManager& sceneManager);

    const std::pair<int, int> &screenResolution() const;

    void setPixel(const int pixelX, const int pixelY, const RTColor color);

    RTColor getPixel(const int pixelX, const int pixelY) const;

    const std::vector<RTColor> pixels() const;
};


#endif // CAMERA_H