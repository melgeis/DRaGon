#include "wgs84.h"
#include <math.h>


namespace LIMoSim
{

WGS84::WGS84()
{

}

/*************************************
 *            PUBLIC METHODS         *
 ************************************/

double WGS84::computeDistance(const Vector3d &_from, const Vector3d &_to)
{
    double lat0 = Vector3d::toRad(_from.y);
    double lat1 = Vector3d::toRad(_to.y);

    double dLon = Vector3d::toRad(_to.x - _from.x);
    double dLat = Vector3d::toRad(_to.y - _from.y);
    double a = pow(sin(dLat/2.0), 2) + cos(lat0) * cos(lat1) * pow(sin(dLon/2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double distance_m = 6367 * c * 1000;

    return distance_m;
}

Vector3d WGS84::computeOffset(const Vector3d &_position, const Vector3d &_origin)
{
    if(_position.toString()==_origin.toString())
        return Vector3d();

    double dX = computeDistance(_origin, Vector3d(_position.x, _origin.y));
    double dY = computeDistance(_origin, Vector3d(_origin.x, _position.y));
    double dZ = 0;

    // adjust the origin alignment if needed
    if(_position.x<_origin.x)
        dX *= -1;
    if(_position.y<_origin.y)
        dY *= -1;
    if(_position.z<_origin.z)
        dZ *= -1;

    return Vector3d(dX, dY, dZ);
}

}
