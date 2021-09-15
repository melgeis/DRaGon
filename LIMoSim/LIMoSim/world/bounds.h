#ifndef LIMOSIM_BOUNDS_H
#define LIMOSIM_BOUNDS_H

#include "vector3d.h"

namespace LIMoSim
{

class Bounds
{
public:
    Bounds();
    Bounds(const Vector3d &_min, const Vector3d &_max);

    void update(const Vector3d &_pos);
    bool isInside(const Vector3d &_pos);
    Vector3d checkInside(const Vector3d &_pos);

    Vector3d getMin();
    Vector3d getMax();

private:
    Vector3d m_min;
    Vector3d m_max;
};

}

#endif // LIMOSIM_BOUNDS_H
