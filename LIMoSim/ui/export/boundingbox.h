#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "LIMoSim/world/vector3d.h"

namespace LIMoSim
{

class BoundingBox
{
public:
    BoundingBox();

public:
    Vector3d leftBottom;
    Vector3d leftTop;
    Vector3d rightBottom;
    Vector3d rightTop;
};

}

#endif // BOUNDINGBOX_H
