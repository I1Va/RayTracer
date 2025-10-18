#include <iostream>

#include "Camera.h"
#include "RayTracer.h"

#include "Output.h"


static const double CLOSEST_HIT_MIN_T = 0.001;


inline double linearToGamma(double linear_component)
{
    if (linear_component > 0)
        return std::sqrt(linear_component);

    return 0;
}

RTPixelColor convertRTColor(const RTColor &color) {  
    double r = color.x();
    double g = color.y();
    double b = color.z();

    r = linearToGamma(r);
    g = linearToGamma(g);
    b = linearToGamma(b);

    static const Interval intensity(0.000, 0.999);
    uint8_t rbyte = uint8_t(256 * intensity.clamp(r));
    uint8_t gbyte = uint8_t(256 * intensity.clamp(g));
    uint8_t bbyte = uint8_t(256 * intensity.clamp(b));

    return { rbyte, gbyte, bbyte, 255 };
}

Camera::Camera(): direction_(0, 0, 1) {}

Camera::Camera
(
    const GmPoint<double, 3> &center, const GmVec<double, 3> &direction, const std::pair<int, int> &screenResolution
): 
    center_(center), direction_(direction.normalized()), screenResolution_(screenResolution), pixels_(screenResolution.first * screenResolution.second)
{   
    pixelSamplesScale_ = 1.0 / samplesPerPixel_; 
    GmVec<double,3> worldUp(0.0, 0.0, 1.0);
    if (std::abs(dot(direction_, worldUp)) > 0.999)
        worldUp = GmVec<double,3>(0.0, 1.0, 0.0);

    viewPort_.rightDir_ = cross(direction_, worldUp).normalized();
    viewPort_.downDir_ = cross(direction_, viewPort_.rightDir_).normalized();

    GmVec<double,3> rightFull = viewPort_.rightDir_ * viewPort_.VIEWPORT_WIDTH;
    GmVec<double,3> downFull    = viewPort_.downDir_ * viewPort_.VIEWPORT_HEIGHT;

    viewPort_.upperLeft_ = center_ + direction_ * FOCAL_LENGTH - rightFull* 0.5 - downFull * 0.5; 
};




// RTColor Camera::computeDirectLighting(const HitRecord &rec, const SceneManager& sceneManager) {
//     RTColor summaryLighting = {0, 0, 0};

//     for (Light *lightSrc : sceneManager.inderectLightSources()) {
        



//     }
// }

RTColor Camera::getRayColor(const Ray& ray, int depth, const SceneManager& sceneManager) const {
    if (depth == 0) return RTColor(0,0,0);

    HitRecord rec = {};

    if (sceneManager.hitClosest(ray, Interval(CLOSEST_HIT_MIN_T, std::numeric_limits<double>::infinity()), rec)) {
        Ray scattered = {};
        RTColor emitted = rec.material->emitted();
        // RTColor Ldirect = computeDirectLighting(rec, sceneManager);
        RTColor attenuation = {};

        if (rec.material->scatter(ray, rec, attenuation, scattered))
            return emitted + attenuation * getRayColor(scattered, depth-1, sceneManager);
        else
            return emitted;
    }

    auto a = 0.5*(ray.direction.y() + 1.0);
    return RTColor(1.0, 1.0, 1.0) * (1.0-a) + RTColor(0.5, 0.7, 1.0) * a;   
}

Ray Camera::genRay(int pixelX, int pixelY) {
    double deltaWidth = viewPort_.VIEWPORT_WIDTH / screenResolution_.first;
    double deltaHeight = viewPort_.VIEWPORT_HEIGHT / screenResolution_.second;

    GmPoint<double, 3> viewPortPoint =  viewPort_.upperLeft_                                                 +
                                        viewPort_.rightDir_ * (pixelX + randomDouble(0.0, 1.0)) * deltaWidth +
                                        viewPort_.downDir_  * (pixelY + randomDouble(0.0, 1.0)) * deltaHeight;
    
    GmVec<double, 3> rayDirection = viewPortPoint - center_;

    return Ray(center_, rayDirection.normalized());
}

void Camera::render(const SceneManager& sceneManager) {
    for (int pixelX = 0; pixelX < screenResolution_.first; pixelX++) {
        for (int pixelY = 0; pixelY < screenResolution_.second; pixelY++) {
            RTColor sampleSumColor = RTColor(0,0,0);
            for (int sample = 0; sample < samplesPerPixel_; sample++) {
                Ray ray = genRay(pixelX, pixelY);
                RTColor rayColor = getRayColor(ray, maxRayDepth_, sceneManager);
                
                
                sampleSumColor += rayColor;
            }   
            setPixel(pixelX, pixelY, convertRTColor(sampleSumColor * pixelSamplesScale_));
        }
    }
}

const std::pair<int, int> &Camera::screenResolution() const { return screenResolution_; }

void Camera::setPixel(const int pixelX, const int pixelY, const RTPixelColor color) {
    pixels_[pixelX * screenResolution_.second + pixelY] = color;
}

RTPixelColor Camera::getPixel(const int pixelX, const int pixelY) const {
    return pixels_[pixelX * screenResolution_.second + pixelY];
}

const std::vector<RTPixelColor> Camera::pixels() const { return pixels_; }

void Camera::setSamplesPerPixel(int newVal) {
    samplesPerPixel_ = newVal;
    pixelSamplesScale_ = 1.0 / samplesPerPixel_; 
}

void Camera::setMaxRayDepth(int newVal) { maxRayDepth_ = newVal; }