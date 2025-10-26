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
    
    gm::IPoint3 upperLeft_;
    gm::IVec3 rightDir_;
    gm::IVec3 downDir_;
};

class Camera {
    static constexpr const double FOCAL_LENGTH = 1;

    gm::IPoint3 center_    = {};
    gm::IVec3   direction_ = {};

    Viewport         viewPort_  = {};
    gm::IVec2 viewAngle_ = {};

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
  // Constructors
    Camera();
    Camera(const gm::IPoint3 &center, const gm::IVec3 &direction,
           const std::pair<int, int> &screenResolution);

  // Camera control
    void rotate(const double widthRadians, const double heightRadians);
    void move(const gm::IVec3 motionVec);

  // Render
    void render(const SceneManager& sceneManager);

  // Getters
    const std::pair<int, int> &screenResolution() const;
    gm::IVec2 viewAngle() const;
    RTPixelColor getPixel(const int pixelX, const int pixelY) const;
    const Viewport &viewPort() const;
    const std::vector<RTPixelColor> pixels() const;
    gm::IVec3 direction() const;
    const gm::IPoint3 center() const;

  // Setters
    void setSamplesPerPixel(const int newVal);
    void setSamplesPerScatter(const int newVal);
    void setMaxRayDepth(const int newVal);
    void setThreadPixelbunchSize(const int newVal);
    void disableLDirect();
    void enableLDirect();

private:    
    // camera fields updating
    void updateViewPort();
    

    // render details
    void setPixel(const int pixelX, const int pixelY, const RTPixelColor color);
    void renderSamplesPerPixel(const int pixelId, const SceneManager& sceneManager);
    Ray genRay(int pixelX, int pixelY);
    RTColor getRayColor(const Ray& ray, const int depth, const SceneManager& sceneManager) const;


    // ray color details
    gm::IVec3 computeDirectLighting(const HitRecord &rec, const SceneManager& sceneManager) const;

  

    bool getMultipleScatterLInderect(const Ray& ray, const HitRecord &hitRecord, 
                                     const int depth, const SceneManager& sceneManager,
                                     gm::IVec3 &LIndirect) const;
};


// Output
std::ostream &operator<<(std::ostream &stream, const Camera &camera);


#endif // CAMERA_H