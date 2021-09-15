#include "utilities.h"
#include <math.h>

int sign(int _val)
{
    if (_val == 0)
        return 0;
    else if (_val > 0)
        return 1;
    else
        return -1;
}


double calculateDistance2d(int _sX, int _sY, int _eX, int _eY)
{
    double x = _eX - _sX;
    double y = _eY - _sY;

    return sqrt(x*x + y*y);
}


std::pair<double, double> calculate2dIntersection(std::pair<double, double> _g1, std::pair<double, double> _g2, std::pair<double, double> _h1, std::pair<double, double> _h2)
{
    // first straight
    double m_g = (_g2.second - _g1.second) / (_g2.first - _g1.first);
    double n_g = _g1.second - (m_g * _g1.first);

    // second straight
    double m_h = (_h2.second - _h1.second) / (_h2.first - _h1.first);
    double n_h = _h1.second - (m_h * _h1.first);

    // calculate intersection
    double x = (n_h - n_g) / (m_g - m_h);
    double y = m_g * x + n_g;

    return std::make_pair(x,y);
}


double calculateFunctionValue(std::pair<double, double> _g1, std::pair<double, double> _g2, double _x)
{
    double m_g = (_g2.second - _g1.second) / (_g2.first - _g1.first);
    double n_g = _g1.second - (m_g * _g1.first);

    return m_g * _x + n_g;
}


double calculatePositionValue(std::pair<double, double> _g1, std::pair<double, double> _g2, double _y)
{
    double m_g = (_g2.second - _g1.second) / (_g2.first - _g1.first);
    double n_g = _g1.second - (m_g * _g1.first);

    return (_y - n_g) / m_g;
}


std::vector<CellElement> bresenham(int _sX, int _sY, int _eX, int _eY)
{
    std::vector<CellElement> pointList;
    int dy = _eY - _sY;
    int dx = _eX - _sX;
    double error = 0.0;

    // check, which direction the "fast" one is
    if (abs(dy) < abs(dx)) {
        int y = _sY;
        double delta_err = abs((double)dy / (double)dx);

        // check, wether to increment or decrement
        if (_sX < _eX)
            for (int x = _sX; x <= _eX; x++) {
                pointList.push_back(CellElement(x,y));
                error += delta_err;

                // check, if variable in "slow" direction needs to change
                if (error >= 0.5) {
                    y = floor(y + sign(dy));
                    error--;
                }
            }
        else
            for (int x = _sX; x >= _eX; x--) {
                pointList.push_back(CellElement(x,y));
                error += delta_err;

                // check, if variable in "slow" direction needs to change
                if (error >= 0.5) {
                    y = ceil(y + sign(dy));
                    error--;
                }
            }
    }
    else {
        int x = _sX;
        double delta_err = abs((double)dx / (double)dy);

        // check, wether to increment or decrement
        if (_sY < _eY)
            for (int y = _sY; y <= _eY; y++) {
                pointList.push_back(CellElement(x,y));
                error += delta_err;

                // check, if variable in "slow" direction needs to change
                if (error >= 0.5) {
                    x = floor(x + sign(dx));
                    error--;
                }
            }
        else
            for (int y = _sY; y >= _eY; y--) {
                pointList.push_back(CellElement(x,y));
                error += delta_err;

                // check, if variable in "slow" direction needs to change
                if (error >= 0.5) {
                    x = ceil(x + sign(dx));
                    error--;
                }
            }
    }

    return pointList;
}
