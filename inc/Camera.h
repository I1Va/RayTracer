#ifndef CAMERA_H
#define CAMERA_H

#include <vector>

#include "RTGeometry.h"
#include "RTObjects.h"
class SceneManager;

inline constexpr int DEFAULT_CAMERA_SAMPLES_PER_PIXEL = 3;
inline constexpr int DEFAULT_CAMERA_SAMPLES_PER_SCATTER = 3;
inline constexpr int DEFAULT_CAMERA_MAX_RAY_DEPTH = 10;
inline constexpr bool DEFAULT_CAMERA_ENABLE_LIDERECT = true;
inline constexpr size_t DEFAULT_THREAD_PIXELBUNCH_SIZE = 64;

struct RTPixelColor {
    uint8_t r, g, b, a;
};

struct Viewport {
    static constexpr const double VIEWPORT_WIDTH = 1;
    static constexpr const double VIEWPORT_HEIGHT = 1;
    
    GmPoint<double, 3> upperLeft_;
    GmVec<double, 3> rightDir_;
    GmVec<double, 3> downDir_;
};

class Camera {
    static constexpr const double FOCAL_LENGTH = 1;

    GmPoint<double, 3> center_    = {};
    GmVec<double, 3>   direction_ = {};

    Viewport         viewPort_  = {};
    GmVec<double, 2> viewAngle_ = {};

    std::pair<int, int> screenResolution_ = {};
    std::vector<RTPixelColor> pixels_ = {};

    double pixelSamplesScale_ = 0;
    double sampleScatterScale_ = 0;

    int samplesPerPixel_        = DEFAULT_CAMERA_SAMPLES_PER_PIXEL;
    int samplesPerScatter_      = DEFAULT_CAMERA_SAMPLES_PER_SCATTER;
    int maxRayDepth_            = DEFAULT_CAMERA_MAX_RAY_DEPTH;

    int threadPixelbunchSize_   = DEFAULT_THREAD_PIXELBUNCH_SIZE;


    bool enableLDirect_     = true;

public:
    Camera();
 
    Camera
    (
        const GmPoint<double, 3> &center, const GmVec<double, 3> &direction, const std::pair<int, int> &screenResolution
    );


    void setDirection(const GmVec<double, 3> &direction);
    void rotate(const double widthRadians, const double heightRadians);


    GmVec<double, 3> computeDirectLighting(const HitRecord &rec, const SceneManager& sceneManager) const;

    RTColor getRayColor(const Ray& ray, const int depth, const SceneManager& sceneManager) const;

    bool getMultipleScatterLInderect(const Ray& ray, const HitRecord &hitRecord, 
                                     const int depth, const SceneManager& sceneManager,
                                     GmVec<double, 3> &LIndirect) const;

    Ray genRay(int pixelX, int pixelY);

    void render(const SceneManager& sceneManager);

    const std::pair<int, int> &screenResolution() const;
    GmVec<double, 2> viewAngle() const;

    void setPixel(const int pixelX, const int pixelY, const RTPixelColor color);

    RTPixelColor getPixel(const int pixelX, const int pixelY) const;

    const std::vector<RTPixelColor> pixels() const;

    void setSamplesPerPixel(const int newVal);
    void setSamplesPerScatter(const int newVal);
    void setMaxRayDepth(const int newVal);
    void setThreadPixelbunchSize(const int newVal);
    void disableLDirect();
    void enableLDirect();

private:
    void renderSamplesPerPixel(const int pixelId, const SceneManager& sceneManager);
};


#endif // CAMERA_H