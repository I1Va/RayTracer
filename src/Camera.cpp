#include <iostream>
#include <omp.h>

#include "Camera.h"
#include "RayTracer.h"
#include "Output.h"

// Utilities
static constexpr double CLOSEST_HIT_MIN_T = 0.001;

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


// Constructors
Camera::Camera(): direction_(0, 0, 1) {}

Camera::Camera
(
    const gm::IPoint3 &center, const gm::IVec3f &direction,
    const std::pair<int, int> &screenResolution
): 
    center_(center), direction_(direction.normalized()), 
    screenResolution_(screenResolution), pixels_(screenResolution.first * screenResolution.second)
{   
    pixelSamplesScale_ = 1.0 / samplesPerPixel_; 
    samplesPerScatter_ = 1.0 / samplesPerScatter_;

    viewAngle_.setX(2 * std::atan(viewPort_.VIEWPORT_HEIGHT / FOCAL_LENGTH));
    viewAngle_.setY(2 * std::atan(viewPort_.VIEWPORT_WIDTH / FOCAL_LENGTH));
    updateViewPort();
};


// Camera control
void Camera::move(const gm::IVec3f motionVec) {
    center_ = center_ + motionVec;
    updateViewPort();
}

void Camera::rotate(const double widthRadians, const double heightRadians) {
    direction_.rotate(viewPort_.downDir_, -widthRadians);
    direction_.rotate(viewPort_.rightDir_, heightRadians);
    updateViewPort();
}


// Render


void Camera::renderParallel(const SceneManager& sceneManager) {
    int pixelCount = screenResolution_.first * screenResolution_.second;

    #pragma omp parallel
    {
        std::mt19937 rng(1234 + omp_get_thread_num());
        #pragma omp for schedule(dynamic,threadPixelbunchSize_)
        for (int pixelId = 0; pixelId < pixelCount; ++pixelId) {
            renderSamplesPerPixel(pixelId, sceneManager);
        }
    }
}

void Camera::render(const SceneManager& sceneManager) {
    if (enableparallelRender_) {
        renderParallel(sceneManager);
        return;
    }

    renderSerial(sceneManager);
}

void Camera::renderSerial(const SceneManager& sceneManager) {
    int pixelCount = screenResolution_.first * screenResolution_.second;
    for (int pixelId = 0; pixelId < pixelCount; ++pixelId) {
        renderSamplesPerPixel(pixelId, sceneManager);
    }
}

void Camera::renderSamplesPerPixel(const int pixelId, const SceneManager& sceneManager) {
    int pixelX = pixelId % screenResolution_.first;
    int pixelY = pixelId / screenResolution_.first;

    RTColor sampleSumColor = RTColor(0,0,0);
    for (int sample = 0; sample < samplesPerPixel_; sample++) {
        Ray ray = genRay(pixelX, pixelY);
        RTColor rayColor = getRayColor(ray, maxRayDepth_, sceneManager);

        sampleSumColor += rayColor;
    }   
    setPixel(pixelX, pixelY, convertRTColor(sampleSumColor * pixelSamplesScale_));
}

Ray Camera::genRay(int pixelX, int pixelY) {
    double deltaWidth = viewPort_.VIEWPORT_WIDTH / screenResolution_.first;
    double deltaHeight = viewPort_.VIEWPORT_HEIGHT / screenResolution_.second;

    gm::IPoint3 viewPortPoint =  viewPort_.upperLeft_                                                 +
                                        viewPort_.rightDir_ * (pixelX + gm::randomDouble(0.0, 1.0)) * deltaWidth +
                                        viewPort_.downDir_  * (pixelY + gm::randomDouble(0.0, 1.0)) * deltaHeight;
    
    gm::IVec3f rayDirection = viewPortPoint - center_;

    return Ray(center_, rayDirection.normalized());
}


RTColor Camera::getRayColor(const Ray& ray, const int depth, const SceneManager& sceneManager) const {
    if (depth == 0) return RTColor(0,0,0);

    HitRecord rec = {};
    if (sceneManager.hitClosest(ray, Interval(CLOSEST_HIT_MIN_T, std::numeric_limits<double>::infinity()), rec)) {
        gm::IVec3f emitted = rec.material->emitted();
        
        gm::IVec3f selected_delta = {SELECTED_DELTA_X, SELECTED_DELTA_Y, SELECTED_DELTA_Z}; 
        if (rec.object->selected()) emitted += selected_delta;

        gm::IVec3f LIndirect = computeMultipleScatterLInderect(ray, rec, depth, sceneManager);
        gm::IVec3f LDirect   = (enableLDirect_ ? computeDirectLighting(rec, sceneManager) : gm::IVec3f{0, 0, 0});
        
        return emitted + LIndirect + LDirect;
    }

    auto a = 0.5*(ray.direction.y() + 1.0);
    return RTColor(1.0, 1.0, 1.0) * (1.0-a) + RTColor(0.5, 0.7, 1.0) * a;   
}

void Camera::setPixel(const int pixelX, const int pixelY, const RTPixelColor color) {
    pixels_[pixelX * screenResolution_.second + pixelY] = color;
}


// Light???
gm::IVec3f Camera::computeDirectLighting(const HitRecord &rec, const SceneManager& sceneManager) const {
    gm::IVec3f summaryLighting = {0, 0, 0};

    gm::IVec3f toView = center_ - rec.point;
    for (Light *lightSrc : sceneManager.inderectLightSources()) {
        Ray toLightRay = Ray(rec.point, lightSrc->center() - rec.point);
    
        HitRecord tmp;
        bool hitted = sceneManager.hitClosest(toLightRay, Interval(CLOSEST_HIT_MIN_T, std::numeric_limits<double>::infinity()), tmp);
    
        summaryLighting += lightSrc->getDirectLighting(toView, rec, hitted);    
    }

    return summaryLighting;
}

gm::IVec3f Camera::computeMultipleScatterLInderect(const Ray& ray, const HitRecord &hitRecord, 
                                                  const int depth, const SceneManager& sceneManager) const
{
    gm::IVec3f LIndirect = {0, 0, 0};
    for (int i = 0; i < samplesPerScatter_; i++) {
        Ray scattered = {};
        RTColor attenuation = {};
        
        if (hitRecord.material->scatter(ray, hitRecord, attenuation, scattered)) {
            LIndirect += attenuation * getRayColor(scattered, depth-1, sceneManager);
        }
    }
    
    return LIndirect * sampleScatterScale_;
}


// Output
std::ostream &operator<<(std::ostream &stream, const Camera &camera) {
    stream << "Camera{" << camera.center() << ", " << camera.direction() << "}";
    return stream;
}


// Camera fields updating
void Camera::updateViewPort() {
    gm::IVec3f worldUp(0.0, 0.0, 1.0);
    if (std::abs(gm::dot(direction_, worldUp)) > 0.999)
        worldUp = gm::IVec3f(0.0, 1.0, 0.0);

    viewPort_.rightDir_ = cross(direction_, worldUp).normalized();
    viewPort_.downDir_ = cross(direction_, viewPort_.rightDir_).normalized();

    gm::IVec3f rightFull = viewPort_.rightDir_ * viewPort_.VIEWPORT_WIDTH;
    gm::IVec3f downFull    = viewPort_.downDir_ * viewPort_.VIEWPORT_HEIGHT;

    viewPort_.upperLeft_ = center_ + direction_ * FOCAL_LENGTH - rightFull* 0.5 - downFull * 0.5; 
}


// Getters
gm::IVec3f Camera::direction() const { return direction_; }
RTPixelColor Camera::getPixel(const int pixelX, const int pixelY) const {
    return pixels_[pixelX * screenResolution_.second + pixelY];
}
const std::vector<RTPixelColor> Camera::pixels() const { return pixels_; }
const std::pair<int, int> &Camera::screenResolution() const { return screenResolution_; }
gm::IVec2f Camera::viewAngle() const { return viewAngle_; }
const Viewport &Camera::viewPort() const { return viewPort_; }
const gm::IPoint3 Camera::center() const { return center_; }

// Setters
void Camera::setSamplesPerPixel(int newVal) {
    samplesPerPixel_ = newVal;
    pixelSamplesScale_ = 1.0 / samplesPerPixel_; 
}
void Camera::setSamplesPerScatter(int newVal) {
    samplesPerScatter_ = newVal;
    sampleScatterScale_ = 1.0 / samplesPerScatter_; 
}
void Camera::setThreadPixelbunchSize(const int newVal) {
    threadPixelbunchSize_ = newVal;
}
void Camera::disableLDirect() { enableLDirect_ = false; }
void Camera::enableLDirect() { enableLDirect_ = true; }
void Camera::setMaxRayDepth(int newVal) { maxRayDepth_ = newVal; }
void Camera::enableParallelRender() { enableparallelRender_ = true; }
void Camera::disableParallelRender() { enableparallelRender_ = false; }
void Camera::enableRayTracerMode() { enableRayTracerMode_ = true; }
void Camera::disableRayTracerMode() { enableRayTracerMode_ = false; }