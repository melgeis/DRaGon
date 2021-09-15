#include "worldutils.h"
#include <algorithm>

namespace LIMoSim {
namespace worldUtils {

double controlledDescent(double descentEnd, double descentStart, double descentFinalValue, double descentStartValue, double value, bool symmetric)
{
    if (descentStart < value) {
        return value;
    }

    double mapped = (value - descentEnd) * (descentStartValue-descentFinalValue)/(descentStart-descentEnd) + descentFinalValue;
    mapped = std::max(std::min(descentStartValue, descentFinalValue), mapped);
    return mapped;
}

double normalizeAngle(double _angle_deg)
{
    while (_angle_deg < 0) {
        _angle_deg += 360;
    }
    while (_angle_deg > 360) {
        _angle_deg -= 360;
    }

    return _angle_deg;
}



} // namespace world
} // namespace LIMoSim
