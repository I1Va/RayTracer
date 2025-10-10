#ifndef CAMERA_H
#define CAMERA_H


#include <vector>

#include "Geom.h"
#include "Objects.h"

using RTColor = GmVec<double, 3>;


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
public:
    Camera(): center_{}, direction_(0, 0, 1) {}
 
    Camera(const GmPoint<double, 3> &center, const GmVec<double, 3> &direction, const std::pair<int, int> &screenResolution): 
        center_(center), direction_(direction.normalized()), screenResolution_(screenResolution), pixels_(screenResolution.first * screenResolution.second)
    {
        GmVec<double,3> A(1,0,0);
        if (std::abs(direction_.x()) > 0.999) A = GmVec<double,3>(0,1,0);

        viewPort_.rightDir_ = cross(A, direction_).normalized();
        viewPort_.downDir_ = cross(direction_, viewPort_.rightDir_).normalized();
        viewPort_.upperLeft_ = center_ + direction_ * FOCAL_LENGTH - viewPort_.rightDir_ * 0.5 - viewPort_.downDir_ * 0.5; 
    };

    Ray genRay(int pixelX, int pixelY) {

        double deltaWidth = viewPort_.VIEWPORT_WIDTH / screenResolution_.first;
        double deltaHeight = viewPort_.VIEWPORT_HEIGHT / screenResolution_.second;

        GmPoint<double, 3> viewPortPoint = viewPort_.upperLeft_                              +
                                           viewPort_.rightDir_ * (pixelX + 0.5) * deltaWidth +
                                           viewPort_.downDir_  * (pixelY + 0.5) * deltaHeight;
        
        GmVec<double, 3> rayDirection = viewPortPoint - center_;

        return Ray(center_, rayDirection);
    }

    const std::pair<int, int> &screenResolution() const { return screenResolution_; }
    void setPixel(const int pixelX, const int pixelY, const RTColor color) {
        pixels_[pixelX * screenResolution_.second + pixelY] = color;
    }
    RTColor getPixel(const int pixelX, const int pixelY) const {
        return pixels_[pixelX * screenResolution_.second + pixelY];
    }

    const std::vector<RTColor> pixels() const { return pixels_; }
};


#endif // CAMERA_H