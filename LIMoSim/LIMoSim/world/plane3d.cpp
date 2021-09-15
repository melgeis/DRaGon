#include "plane3d.h"
#include <math.h>
#include <iostream>

namespace LIMoSim
{

Plane3d::Plane3d(const Vector3d &_v0, const Vector3d &_v1, const Vector3d &_v2) :
    v0(_v0),
    v1(_v1),
    v2(_v2)
{
    Vector3d u = (v1-v0).normed();
    Vector3d w = (v2-v0).normed();

    n = u.cross(w).normed();
    d = (v0 * (n * -1));

    bounds.update(v0);
    bounds.update(v1);
    bounds.update(v2);
}

/*************************************
 *            PUBLIC METHODS         *
 ************************************/

Vector3d Plane3d::computeIntersection(const Vector3d &_from, const Vector3d &_to)
{
    Vector3d intersection;
    Vector3d dir = (_to-_from).normed();
    double denom = n * dir;
    if(fabs(denom)>0.000000001)
    {
        double t = -(n * _from + d) / denom;
        intersection = _from + dir * t;
    }
    else
        intersection.valid = false;

    //
    double orginDistance = (intersection-_from).norm();
    double targetDistance = (intersection-_to).norm();
    double rayDistance = (_to-_from).norm();
    if(targetDistance>rayDistance || orginDistance>rayDistance)
        intersection.valid = false;

    return intersection;
}

/*************************************
 *              OPERATORS            *
 ************************************/



/*************************************
 *           PRIVATE METHODS         *
 ************************************/



}
