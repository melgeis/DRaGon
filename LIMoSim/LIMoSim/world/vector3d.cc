#include "vector3d.h"
#include "worldutils.h"
#include <math.h>
#include <iostream>
#include <algorithm>

namespace LIMoSim
{

Vector3d::Vector3d(double _x, double _y, double _z) :
    x(_x),
    y(_y),
    z(_z),
    valid(true)
{

}

Vector3d Vector3d::fromSphere(double _theta, double _phi, double _r)
{
    double theta = toRad(_theta);
    double phi = toRad(_phi);

    return Vector3d(_r*sin(theta)*cos(phi), _r*sin(theta)*sin(phi), _r*cos(theta));
}

/*************************************
 *            PUBLIC METHODS         *
 ************************************/

double Vector3d::norm() const
{
    return sqrt(x*x+y*y+z*z);
}

double Vector3d::norm2d()
{
    return sqrt(x*x+y*y);
}

Vector3d Vector3d::normed()
{
    return *this/norm();
}

Vector3d Vector3d::rotateRight()
{
    return Vector3d(y, -x, z);
}


Vector3d Vector3d::rotateTheta(double _offset)
{
    return Vector3d(x*cos(_offset) -y*sin(_offset), x*sin(_offset) + y*cos(_offset), z);
}

Vector3d Vector3d::rotateLeft()
{
    return Vector3d(-y, x, z);
}

Vector3d Vector3d::cross(const Vector3d &_v)
{
    return Vector3d(y*_v.z - z*_v.y, z*_v.x - x*_v.z, x*_v.y - y*_v.x);
}

Vector3d Vector3d::abs()
{
    return Vector3d(fabs(x), fabs(y), fabs(z));
}

double Vector3d::cross2d(const Vector3d &_v)
{
    return (x*_v.y) - (y*_v.x);
}

double Vector3d::computePhi()
{
    return toGrad(atan2(y, x));
}

double Vector3d::computeAngleDifference(double _from, double _to)
{
    double d = _to - _from;
    if(fabs(d)>180)
    {
        d = std::min(_from, _to) + 360 - std::max(_from, _to);
        if(_from<_to)
            d *= -1;
    }

    return d;
}

double Vector3d::computeMinimumAngleDifference(double _from, double _to)
{
    double angleDifference = computeAngleDifference(_from, _to);
    if(angleDifference>90)
        angleDifference = 180 - angleDifference;
    else if(angleDifference<-90)
        angleDifference = angleDifference + 180;

    return angleDifference;
}

double Vector3d::toRad(double _grad)
{
    return _grad/180.0*3.141592654;
}

double Vector3d::toGrad(double _rad)
{
    return _rad*180.0/3.141592654;
}

std::string Vector3d::toString() const
{
    std::stringstream stream;
    stream << x << "," << y << "," << z;

    return stream.str();
}

void Vector3d::info()
{
    std::cout << toString() << std::endl;
}


double Vector3d::pitch()
{
    return this->x;
}

double Vector3d::roll()
{
    return this->y;
}

double Vector3d::yaw()
{
    return this->z;
}

void Vector3d::setPitch(double _pitch)
{
    this->x = _pitch;
}

void Vector3d::setRoll(double _roll)
{
    this->y = _roll;
}

void Vector3d::setYaw(double _yaw)
{
    this->z = _yaw;
}


Vector3d Vector3d::orientationFromDirection(Vector3d _direction)
{
    double cosinus = _direction.x / sqrt(_direction.x*_direction.x + _direction.y*_direction.y);
    Vector3d orientation (
                Vector3d::toGrad(acos(_direction.z / _direction.norm())),
                0, // no roll for now
                worldUtils::normalizeAngle(Vector3d::toGrad(atan(_direction.y / _direction.x)) + ((cosinus < 0) ? 180 : 0))
                );
    return orientation;
}


/*************************************
 *              OPERATORS            *
 ************************************/

Vector3d operator+(Vector3d _lhs, const Vector3d &_rhs)
{
    return Vector3d(_lhs.x+_rhs.x, _lhs.y+_rhs.y, _lhs.z+_rhs.z);
}

Vector3d operator-(Vector3d _lhs, const Vector3d &_rhs)
{
    return Vector3d(_lhs.x-_rhs.x, _lhs.y-_rhs.y, _lhs.z-_rhs.z);
}

Vector3d operator*(Vector3d _lhs, double _rhs)
{
    return Vector3d(_lhs.x*_rhs, _lhs.y*_rhs, _lhs.z*_rhs);
}

Vector3d operator/(Vector3d _lhs, double _rhs)
{
    return Vector3d(_lhs.x/_rhs, _lhs.y/_rhs, _lhs.z/_rhs);
}

double operator*(Vector3d _lhs, const Vector3d &_rhs)
{
    return _lhs.x*_rhs.x + _lhs.y*_rhs.y + _lhs.z*_rhs.z;
}


/*************************************
 *           PRIVATE METHODS         *
 ************************************/


}
