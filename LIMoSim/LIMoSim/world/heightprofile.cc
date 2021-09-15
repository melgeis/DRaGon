#include "heightprofile.h"
#include <iostream>
#include "world.h"
#include "settings/osm/wgs84.h"

namespace LIMoSim
{

HeightProfile::HeightProfile() :
    m_cellSize_m(10.15),
    m_numColumns(0),
    m_numRows(0)
{

}

/*************************************
 *            PUBLIC METHODS         *
 ************************************/

void HeightProfile::loadFromFile(const std::string &_path)
{
    std::cout << "HeightProfile::loadFromFile" << std::endl;

    std::vector<std::string> lines = FileHandler::read(_path);
    std::vector<std::string> items = Parser::split(lines[0], ",");


    Vector3d mapOrigin = Vector3d(atof(items[3].c_str()), atof(items[1].c_str())); //(7.40232, 51.4887);
    m_offset = WGS84::computeOffset(mapOrigin, World::getInstance()->getReference());

    // numRows / numCols?
//    m_numRows = 100;
//    m_numColumns = 100;

    m_cellSize_m = atof(items[5].c_str()); //= 2;
    // TODO: compute min cell size and use as reference

    m_numRows = lines.size() - 2;//137;
    m_numColumns = Parser::split(lines[1], ",").size();//233;

    for(unsigned int y=0; y<lines.size()-1; y++)
    {
        std::vector<std::string> items = Parser::split(lines[y+1], ",");
        for(unsigned int x=0; x<items.size(); x++)
        {
            if(items.size()>1)
            {
                double _y = m_numRows-1-y;
                double v = atof(items[x].c_str());

                HeightElement *cell = new HeightElement;
                cell->center = Vector3d(x*m_cellSize_m+m_cellSize_m/2, _y*m_cellSize_m+m_cellSize_m/2) + m_offset;

//                double in_min = 95;
//                double in_max = 137;

//                double out_min = 0;
//                double out_max = 200;

//                double mapped = (v - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
//                mapped -= out_max/2;

                //cell->center.z = mapped;
                cell->center.z = v;
                m_cells[{_y, x}] = cell;
            }

        }
    }

    // step 2: determine vertices and normal vector based on neighboring cells
    for(unsigned int y=0; y<m_numRows; y++)
    {
        for(unsigned int x=0; x<m_numColumns; x++)
        {
            m_cells[{y,x}]->p0 = computeAverage(y, x, -1, -1);
            m_cells[{y,x}]->p1 = computeAverage(y, x, -1, 1);
            m_cells[{y,x}]->p2 = computeAverage(y, x, 1, 1);
            m_cells[{y,x}]->p3 = computeAverage(y, x, 1, -1);

            m_cells[{y,x}]->normal = computeNormal(m_cells[{y,x}]);
            if(m_cells[{y,x}]->normal.z<0)
                m_cells[{y,x}]->normal = m_cells[{y,x}]->normal * -1;
        }
    }
}

/*************************************
 *              ACCESSORS            *
 ************************************/

HeightElement* HeightProfile::getCell(const Vector3d &_pos)
{
    Vector3d position = _pos - m_offset;
    int x = position.x / m_cellSize_m;
    int y = position.y / m_cellSize_m;

    return getCell(y, x);
}

HeightElement* HeightProfile::getCell(int _y, int _x)
{
    if(m_cells.count({_y, _x})>0)
        return m_cells[{_y, _x}];
    return 0;
}

double HeightProfile::getElevation(const Vector3d &_pos)
{
    HeightElement *cell = getCell(_pos);
    if(cell)
        return cell->center.z;
    else
    {
        // interpolate from nearest cell values
        Vector3d position = _pos - m_offset;
        int x = position.x / m_cellSize_m;
        int y = position.y / m_cellSize_m;

        if(x<0)
            x = 0;
        if(x>m_numColumns-1)
            x = m_numColumns-1;

        if(y<0)
            y = 0;
        if(y>m_numRows-1)
            y = m_numRows-1;

        HeightElement *cell = getCell(y, x);
        if(cell)
            return cell->center.z;
        return 0;
    }
}

Vector3d HeightProfile::computeAverage(int _y, int _x, int _rows, int _columns)
{
    int numCells = 1;
    Vector3d p = getCell(_y, _x)->center;

    HeightElement *cell = getCell(_y+_rows, _x);
    if(cell)
    {
        p = p + cell->center;
        numCells++;
    }

    cell = getCell(_y, _x+_columns);
    if(cell)
    {
        p = p + cell->center;
        numCells++;
    }

    cell = getCell(_y+_rows, _x+_columns);
    if(cell)
    {
        p = p + cell->center;
        numCells++;
    }

    return p/numCells;
}

Vector3d HeightProfile::computeNormal(HeightElement *_cell)
{
    return (_cell->p1-_cell->p0).normed().cross((_cell->p2-_cell->p0).normed());
}

double HeightProfile::getCellSize()
{
    return m_cellSize_m;
}

unsigned int HeightProfile::getNumColumns()
{
    return m_numColumns;
}

unsigned int HeightProfile::getNumRows()
{
    return m_numRows;
}

Vector3d HeightProfile::getOffset()
{
    return m_offset;
}

/*************************************
 *           PRIVATE METHODS         *
 ************************************/


}
