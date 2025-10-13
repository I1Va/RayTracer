#include <iostream>

#include "Camera.h"
#include "RayTracer.h"

static const int CLOSEST_HIT_MIN_T = 0.001;

Camera::Camera(): direction_(0, 0, 1) {}

Camera::Camera
(
    const GmPoint<double, 3> &center, const GmVec<double, 3> &direction, const std::pair<int, int> &screenResolution
): 
    center_(center), direction_(direction.normalized()), screenResolution_(screenResolution), pixels_(screenResolution.first * screenResolution.second)
{   
    pixelSamplesScale_ = 1.0 / samplesPerPixel; 
    GmVec<double,3> A(1,0,0);
    if (std::abs(direction_.x()) > 0.999) A = GmVec<double,3>(0,1,0);

    viewPort_.rightDir_ = cross(A, direction_).normalized();
    viewPort_.downDir_ = cross(direction_, viewPort_.rightDir_).normalized();
    viewPort_.upperLeft_ = center_ + direction_ * FOCAL_LENGTH - viewPort_.rightDir_ * 0.5 - viewPort_.downDir_ * 0.5; 
};


RTColor Camera::getRayColor(const Ray& ray, int depth, const SceneManager& sceneManager) const {
    if (depth == 0) return RTColor(0,0,0);

    HitRecord HitRecord = {};

    if (sceneManager.hitClosest(ray, Interval(CLOSEST_HIT_MIN_T, std::numeric_limits<double>::infinity()), HitRecord)) {
        GmVec<double, 3> direction = randomOnHemisphere(HitRecord.normal);
        return getRayColor(Ray(HitRecord.point, direction),  depth - 1, sceneManager) * 0.5;
    }

    auto a = 0.5*(ray.direction.y() + 1.0);
    return RTColor(1.0, 1.0, 1.0) * (1.0-a) + RTColor(0.5, 0.7, 1.0) * a;   
}

Ray Camera::genRay(int pixelX, int pixelY) {
    double deltaWidth = viewPort_.VIEWPORT_WIDTH / screenResolution_.first;
    double deltaHeight = viewPort_.VIEWPORT_HEIGHT / screenResolution_.second;

    GmPoint<double, 3> viewPortPoint =  viewPort_.upperLeft_                              +
                                        viewPort_.rightDir_ * (pixelX + randomDouble(0.0, 1.0)) * deltaWidth +
                                        viewPort_.downDir_  * (pixelY + randomDouble(0.0, 1.0)) * deltaHeight;
    
    GmVec<double, 3> rayDirection = viewPortPoint - center_;

    return Ray(center_, rayDirection);
}

void Camera::render(const SceneManager& sceneManager) {
    for (int pixelX = 0; pixelX < screenResolution_.first; pixelX++) {
        for (int pixelY = 0; pixelY < screenResolution_.second; pixelY++) {
            RTColor sampleSumColor = RTColor(0,0,0);
            for (int sample = 0; sample < samplesPerPixel; sample++) {
                Ray ray = genRay(pixelX, pixelY);
                RTColor rayColor = getRayColor(ray, maxRayDepth, sceneManager);
                sampleSumColor += rayColor;
            }
            setPixel(pixelX, pixelY, sampleSumColor * pixelSamplesScale_);
        }
    }
}

const std::pair<int, int> &Camera::screenResolution() const { return screenResolution_; }

void Camera::setPixel(const int pixelX, const int pixelY, const RTColor color) {
    pixels_[pixelX * screenResolution_.second + pixelY] = color;
}

RTColor Camera::getPixel(const int pixelX, const int pixelY) const {
    return pixels_[pixelX * screenResolution_.second + pixelY];
}

const std::vector<RTColor> Camera::pixels() const { return pixels_; }