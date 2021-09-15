#ifndef LIMOSIM_VECTOR3D_H
#define LIMOSIM_VECTOR3D_H

#include <sstream>

namespace LIMoSim
{

class Vector3d
{
public:
    Vector3d(double _x=0, double _y=0, double _z=0);
    static Vector3d fromSphere(double _phi, double _theta, double _r);
    double norm() const;
    double norm2d();
    Vector3d normed();
    Vector3d rotateRight();    
    Vector3d rotateTheta(double _offset);
    Vector3d rotateLeft();
    Vector3d cross(const Vector3d &_v);
    Vector3d abs();

    //
    double cross2d(const Vector3d &_v);
    double computePhi();
    static double computeAngleDifference(double _from, double _to);
    static double computeMinimumAngleDifference(double _from, double _to);

    //
    static double toRad(double _grad);
    static double toGrad(double _rad);

    //
    std::string toString() const;
    void info();

    // Orientation View
    double pitch();
    double roll();
    double yaw();
    void setPitch(double _pitch);
    void setRoll(double _roll);
    void setYaw(double _yaw);

    //
    static Vector3d orientationFromDirection(Vector3d _direction);


public:
    double x;
    double y;
    double z;
    bool valid;
};

// operators
Vector3d operator+(Vector3d _lhs, const Vector3d &_rhs);
Vector3d operator-(Vector3d _lhs, const Vector3d &_rhs);
Vector3d operator*(Vector3d _lhs, double _rhs);
Vector3d operator/(Vector3d _lhs, double _rhs);
double operator*(Vector3d _lhs, const Vector3d &_rhs);

}

#endif // VECTOR3D
