#ifndef LIMOSIM_HEIGHTPROFILE_H
#define LIMOSIM_HEIGHTPROFILE_H

#include "settings/parser.h"
#include "vector3d.h"
#include <tuple>
#include <map>
#include <sstream>

namespace LIMoSim
{

struct HeightElement
{
    Vector3d p0; // bottom left
    Vector3d p1; // bottom right
    Vector3d p2; // top right
    Vector3d p3; // top left

    Vector3d normal;
    Vector3d center;

};

class HeightProfile
{
public:
    HeightProfile();

    void example();

    void loadFromFile(const std::string &_path);

    HeightElement* getCell(const Vector3d &_pos);
    HeightElement* getCell(int _y, int _x);
    double getElevation(const Vector3d &_pos);

    Vector3d computeAverage(int _y, int _x, int _rows, int _columns);
    Vector3d computeNormal(HeightElement *_cell);

    double getCellSize();
    unsigned int getNumColumns();
    unsigned int getNumRows();

    Vector3d getOffset();

private:
    Vector3d m_offset;
    double m_cellSize_m;

    std::map<std::tuple<int,int>, HeightElement*> m_cells;
    unsigned int m_numColumns;
    unsigned int m_numRows;

};

}

#endif // LIMOSIM_HEIGHTPROFILE_H
