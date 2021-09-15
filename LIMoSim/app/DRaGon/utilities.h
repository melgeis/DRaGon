#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <vector>

struct CellElement
{
    int row;
    int col;

    CellElement(int _row=0, int _col=0) {
        row = _row;
        col = _col;
    }
};

int sign(int _val);
double calculateDistance2d(int _sX, int _sY, int _eX, int _eY);
std::pair<double, double> calculate2dIntersection(std::pair<double, double> _g1, std::pair<double, double> _g2, std::pair<double, double> _h1, std::pair<double, double> _h2);
double calculateFunctionValue(std::pair<double, double> _g1, std::pair<double, double> _g2, double _x);
double calculatePositionValue(std::pair<double, double> _g1, std::pair<double, double> _g2, double _y);

std::vector<CellElement> bresenham(int _sX, int _sY, int _eX, int _eY);

#endif // UTILITIES_H
