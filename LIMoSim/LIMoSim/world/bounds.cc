#include "bounds.h"
#include <limits>
#include <algorithm>

namespace LIMoSim
{

Bounds::Bounds()
{
    double m = std::numeric_limits<double>::max();
    m_min = Vector3d(m, m, m);
    m_max = Vector3d(-m, -m, -m);
}

Bounds::Bounds(const Vector3d &_min, const Vector3d &_max) :
    m_min(_min),
    m_max(_max)
{

}

/*************************************
 *            PUBLIC METHODS         *
 ************************************/

void Bounds::update(const Vector3d &_pos)
{
    m_min.x = std::min(m_min.x, _pos.x);
    m_min.y = std::min(m_min.y, _pos.y);
    m_min.z = std::min(m_min.z, _pos.z);

    m_max.x = std::max(m_max.x, _pos.x);
    m_max.y = std::max(m_max.y, _pos.y);
    m_max.z = std::max(m_max.z, _pos.z);
}

bool Bounds::isInside(const Vector3d &_pos)
{
    Vector3d c = checkInside(_pos);

    if (c.x && c.y && c.z)
        return true;
    return false;
}

Vector3d Bounds::checkInside(const Vector3d &_pos)
{
    bool x = (_pos.x>=m_min.x) && (_pos.x<=m_max.x);
    bool y = (_pos.y>=m_min.y) && (_pos.y<=m_max.y);
    bool z = (_pos.z>=m_min.z) && (_pos.z<=m_max.z);

    return Vector3d(x, y, z);
}

Vector3d Bounds::getMin()
{
    return m_min;
}

Vector3d Bounds::getMax()
{
    return m_max;
}

}
