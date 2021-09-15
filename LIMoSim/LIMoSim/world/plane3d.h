#ifndef LIMOSIM_PLANE3D_H
#define LIMOSIM_PLANE3D_H

#include "vector3d.h"
#include "bounds.h"

namespace LIMoSim
{

class Plane3d
{
public:
    Plane3d(const Vector3d &_v0, const Vector3d &_v1, const Vector3d &_v2);

    Vector3d computeIntersection(const Vector3d &_from, const Vector3d &_to);

public:
    Vector3d v0;
    Vector3d v1;
    Vector3d v2;

    Vector3d n;
    double d;

    Bounds bounds;
};

}
#endif // LIMOSIM_PLANE3D_H
