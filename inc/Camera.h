#ifndef CAMERA_H
#define CAMERA_H

#include <vector>

#include "RTGeometry.h"
#include "RTObjects.h"
class SceneManager;

struct RTPixelColor {
    uint8_t r, g, b, a;
};

struct CameraRenderProperties {
    int samplesPerPixel;
    int samplesPerScatter;    
    int maxRayDepth;       
    int threadPixelbunchSize; 
    bool enableParallelRender;  
    bool enableLDirect;         
    bool enableRayTracerMode;   
};

struct Viewport {
    static constexpr const double VIEWPORT_WIDTH = 1;
    static constexpr const double VIEWPORT_HEIGHT = 1;
    
    gm::IPoint3 upperLeft_;
    gm::IVec3f  rightDir_;
    gm::IVec3f  downDir_;

    gm::IVec2f viewAngle_;
};

class Camera {
public:
    CameraRenderProperties renderProperties = 
    {
        .samplesPerPixel        = 3,
        .samplesPerScatter      = 3,
        .maxRayDepth            = 10,
        .threadPixelbunchSize   = 64,
        .enableParallelRender   = true,
        .enableLDirect          = true,
        .enableRayTracerMode    = true,
    };
private:
    static constexpr const double FOCAL_LENGTH = 1;

    gm::IPoint3 center_    = gm::IPoint3(0, 0, 0);
    gm::IVec3f  direction_ = gm::IVec3f(0, 0, 1);

    Viewport   viewPort_  = {};
    double pixelSamplesScale_   = 1.0 / renderProperties.samplesPerPixel;
    double sampleScatterScale_  = 1.0 / renderProperties.samplesPerScatter;

public:
  // Constructors
    Camera();

  // Camera control
    void rotate(const double widthRadians, const double heightRadians);
    void move(const gm::IVec3f motionVec);

  // Render
    void render
    (
        const SceneManager& sceneManager,
        const std::pair<int, int> screenResolution,
        std::vector<RTPixelColor> &outputBufer
    );

    void renderParallel
    (
        const SceneManager& sceneManager,
        const std::pair<int, int> screenResolution,
        std::vector<RTPixelColor> &outputBufer
    );

    void renderSerial
    (
        const SceneManager& sceneManager,
        const std::pair<int, int> screenResolution,
        std::vector<RTPixelColor> &outputBufer
    );

    RTPixelColor renderPixelColor
    (
        const SceneManager& sceneManager,
        const int pixelId,
        const std::pair<int, int> screenResolution
    );

  // Getters
    gm::IVec3f direction() const;
    const gm::IPoint3 center() const;

  // Setters
    void setCenter(const gm::IPoint3 center);
    void setDirection(const gm::IVec3f direction);

private:    
// camera fields updating
    void updateViewPort();
    

// render details
    Ray genRay(int pixelX, int pixelY, std::pair<int, int> screenResolution);
    
    RTColor getRayColor
    (
        const Ray& ray, 
        const int depth, 
        const SceneManager& sceneManager
    ) const;

// ray color details
    gm::IVec3f computeDirectLighting
    (
      const HitRecord &rec, 
      const SceneManager& sceneManager
    ) const;

    gm::IVec3f computeMultipleScatterLInderect
    (
      const Ray& ray, 
      const HitRecord &hitRecord, 
      const int depth, 
      const SceneManager& sceneManager
    ) const;
};


// Output
std::ostream &operator<<(std::ostream &stream, const Camera &camera);


#endif // CAMERA_H