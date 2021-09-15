#ifndef WORLDUTILS_H
#define WORLDUTILS_H


namespace LIMoSim {
namespace worldUtils {
double controlledDescent(double descentEnd, double descentStart, double descentFinalValue, double descentStartValue, double value, bool symmetric = false);

/**
 * @brief normalizeAngle
 * brings all angles in range [0, 360]
 * @param _angle_deg
 * @return
 */
double normalizeAngle(double _angle_deg);
template <typename T> int signum(T val) {
    return (T(0) < val) - (val < T(0));

}

} // namespace world
} // namespace LIMoSim

#endif // WORLDUTILS_H
