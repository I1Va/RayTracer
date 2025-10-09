#ifndef CAMERA_H
#define CAMERA_H

#include "Geom.h"

struct Viewport {
    static constexpr const double VIEWPORT_WIDTH = 1;
    double viewPortHeigthWidthRatio;
    GmPoint<double, 3> upperLeft_;
    GmVec<double, 3> rightDir_;
    GmVec<double, 3> downDir_;
};

class Camera {
    static constexpr const double FOCAL_LENGTH = 1;



    GmPoint<double, 3> center_;
    GmVec<double, 3> direction_;
    Viewport viewPort_;

public:
    Camera(const GmPoint<double, 3> &center, const GmVec<double, 3> &direction, const double viewPortHeigthWidthRatio): 
        center_(center), direction_(direction.normalized()) {
        GmVec<double,3> A(1,0,0);
        if (std::abs(direction_.x()) > 0.999) A = GmVec<double,3>(0,1,0);

        viewPort_.viewPortHeigthWidthRatio = viewPortHeigthWidthRatio;
        viewPort_.rightDir_ = cross(A, direction_).normalized();
        viewPort_.downDir_ = cross(direction_, viewPort_.rightDir_).normalized();
        viewPort_.upperLeft_ = center_ + direction_ * FOCAL_LENGTH - viewPort_.rightDir_ * 0.5 - viewPort_.downDir_ * 0.5; 
    };
};




#endif // CAMERA_H