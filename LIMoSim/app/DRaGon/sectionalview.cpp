#include "sectionalview.h"

#include <algorithm>

#include "LIMoSim/world/world.h"
#include "LIMoSim/world/plane3d.h"
#include "LIMoSim/settings/filehandler.h"

#include "ui/export/epsdocument.h"


namespace LIMoSim
{

SectionalView::SectionalView(Vector3d _tx, Vector3d _rx, bool _terrain, bool _buildings)
    : m_tx(_tx)
    , m_rx(_rx)
    , m_terrain(_terrain)
    , m_buildings(_buildings)
{
    m_txrxDist2d = (m_rx - m_tx).norm2d();
    m_txrxDist3d = (m_rx - m_tx).norm();
}

SectionalView::~SectionalView()
{
    resetValues();
}


SectionalView& SectionalView::operator=(SectionalView &o)
{
    m_tx = o.m_tx;
    m_rx = o.m_rx;

    m_borderMultiplier = o.m_borderMultiplier;

    m_cellSize_m = o.m_cellSize_m;
    m_numRows = o.m_numRows;
    m_numColumns = o.m_numColumns;

    m_txrxDist2d = o.m_txrxDist2d;
    m_txrxDist3d = o.m_txrxDist3d;

    m_indoorDist = o.m_indoorDist;
    m_terrainDist = o.m_terrainDist;

    m_minElevation = o.m_minElevation;
    m_maxElevation = o.m_maxElevation;

    m_eDist = o.m_eDist;
    m_elevation = o.m_elevation;
    m_bDist = o.m_bDist;
    m_bAltitude = o.m_bAltitude;
    m_iDist = o.m_iDist;
    m_iAltitude = o.m_iAltitude;
    m_tDist = o.m_tDist;
    m_tAltitude = o.m_tAltitude;

    m_heightProfile = o.m_heightProfile;

    m_terrain = o.m_terrain;
    m_buildings = o.m_buildings;
    m_executed = o.m_executed;

    return *this;
}


void SectionalView::resetValues()
{
    m_cellSize_m = -1;
    m_numColumns = -1;
    m_numRows = -1;

    m_indoorDist = 0;
    m_terrainDist = 0;

    m_minElevation = std::numeric_limits<double>::infinity();
    m_maxElevation = -std::numeric_limits<double>::infinity();

    m_eDist.clear();
    m_elevation.clear();
    m_bDist.clear();
    m_bAltitude.clear();
    m_iDist.clear();
    m_iAltitude.clear();
    m_tDist.clear();
    m_tAltitude.clear();

    m_heightProfile = nullptr;

    m_executed = false;
}


void SectionalView::useBuildings(bool _use)
{
    m_buildings = _use;
}


void SectionalView::useTerrain(bool _use)
{
    m_terrain = _use;
}


void SectionalView::run(bool is_info)
{
    resetValues();

    // check, if tx and rx position are the same
    if (m_tx.x == m_rx.x && m_tx.y == m_rx.y && m_tx.z == m_rx.z)
        return;


    m_heightProfile = World::getInstance()->getHeightProfile();
    m_cellSize_m = m_heightProfile->getCellSize();
    m_numColumns = m_heightProfile->getNumColumns();
    m_numRows = m_heightProfile->getNumRows();

    // check if tx and rx are within boundaries
    double minX = m_heightProfile->getOffset().x;
    double minY = m_heightProfile->getOffset().y;
    double maxX = m_cellSize_m * m_numColumns + m_heightProfile->getOffset().x;
    double maxY = m_cellSize_m * m_numRows + m_heightProfile->getOffset().y;

    if (m_tx.x < minX || m_tx.x > maxX || m_tx.y < minY || m_tx.y > maxY) {
        std::cout<<"transmitter position is out of range"<<std::endl;
        std::cout<<"lower left boundary: ("<<minX<<","<<minY<<")"<<std::endl;
        std::cout<<"upper right boundary: ("<<maxX<<","<<maxY<<")"<<std::endl;
        std::cout<<"current: ("<<m_tx.x<<","<<m_tx.y<<")"<<std::endl;
        return;
    }
    else if (m_rx.x < minX || m_rx.x > maxX || m_rx.y < minY || m_rx.y > maxY) {
        std::cout<<"receiver position is out of range"<<std::endl;
        std::cout<<"lower left boundary: ("<<minX<<","<<minY<<")"<<std::endl;
        std::cout<<"upper right boundary: ("<<maxX<<","<<maxY<<")"<<std::endl;
        std::cout<<"current: ("<<m_rx.x<<","<<m_rx.y<<")"<<std::endl;
        return;
    }

    Vector3d dir = (m_rx-m_tx).normed();
    Vector3d from = m_tx - (dir * (m_txrxDist3d * m_borderMultiplier));
    Vector3d to = m_rx + (dir * (m_txrxDist3d * m_borderMultiplier));

    // get cell coordinates from vectors
    std::pair<int, int> tx_coor = getCellCoordinates2Pos(m_tx);
    std::pair<int, int> rx_coor = getCellCoordinates2Pos(m_rx);
    std::pair<int, int> from_coor = getCellCoordinates2Pos(from);
    std::pair<int, int> to_coor = getCellCoordinates2Pos(to);

    if (from_coor.first == -1 || to_coor.first == -1) {
        std::cout<<"error: reset positions to original"<<std::endl;
        from = m_tx;
        to = m_rx;
        from_coor = getCellCoordinates2Pos(from);
        to_coor = getCellCoordinates2Pos(to);
    }

    // check if rx or tx is below terrain level
    if ((m_terrain && (m_tx.z < m_heightProfile->getCell(tx_coor.second, tx_coor.first)->center.z || m_rx.z < m_heightProfile->getCell(rx_coor.second, rx_coor.first)->center.z))
            || (!m_terrain && (m_tx.z < 0 || m_rx.z < 0))) {
        std::cout<<m_tx.z<<std::endl;
        std::cout<<m_heightProfile->getCell(tx_coor.second, tx_coor.first)->center.z<<std::endl;
        std::cout<<m_rx.z<<std::endl;
        std::cout<<m_heightProfile->getCell(rx_coor.second, rx_coor.first)->center.z<<std::endl;
        std::cout<<"transmitter or receiver position is below the terrain level"<<std::endl;
    }

    if (m_terrain) {
        // get relevant elevation information
        elevationInformation(from_coor, to_coor);

        if (is_info)
            // calculate distance trough underground terrain
            terrainIntersections();
    }

    if (m_buildings) {
        // get building information
        std::vector<std::pair<double, double>> intersections = buildingInformation(is_info);

        // correct zero alitude values in bHeight
        if (m_terrain) {
            for (unsigned int i = 0; i < m_bAltitude.size(); i++)
                if (m_bAltitude.at(i) == 0)
                    m_bAltitude[i] = m_minElevation;
        }

        // check if tx or rx is inside building
        bool tx_indoor = false;
        bool rx_indoor = false;
        for (unsigned int i = 1; i < m_bDist.size(); i++) {
            if (m_bDist.at(i-1) < 0 && m_bDist.at(i) > 0 && m_bAltitude.at(i-1) > m_tx.z)
                tx_indoor = true;
            else if (m_bDist.at(i-1) < m_txrxDist2d && m_bDist.at(i) > m_txrxDist2d && m_bAltitude.at(i-1) > m_rx.z)
                rx_indoor = true;
        }

        m_indoorDist = 0;
        if (is_info && !intersections.empty()) {
            // save intersection points
            for (unsigned int i = 0; i < intersections.size(); i++) {
                m_iDist.push_back(intersections.at(i).first);
                m_iAltitude.push_back(intersections.at(i).second);

                // calculate indoor distance
                if ((!tx_indoor && i % 2 == 1) || (tx_indoor && i % 2 == 0 && i > 0)) {
                    double curDist = calculateDistance2d(m_iDist.at(i-1), m_iAltitude.at(i-1), m_iDist.at(i), m_iAltitude.at(i));
                    m_indoorDist += curDist;
                }
            }

            if (tx_indoor)
                m_indoorDist += calculateDistance2d(0, m_tx.z, m_iDist.front(), m_iAltitude.front());
            if (rx_indoor)
                m_indoorDist += calculateDistance2d(m_txrxDist2d, m_rx.z, m_iDist.back(), m_iAltitude.back());
        }
    }

    m_executed = true;
}


void SectionalView::print() {
    std::cout<<"indoor distance: "<<m_indoorDist<<std::endl;
    std::cout<<"terrain distance: "<<m_terrainDist<<std::endl;
    std::cout<<"Tx to Rx distance: "<<m_txrxDist3d<<std::endl;
    std::cout<<"LOS distance: "<<m_txrxDist3d-m_indoorDist-m_terrainDist<<std::endl;
    std::cout<<"OLOS distance: "<<m_indoorDist+m_terrainDist<<std::endl;
}


void SectionalView::csvExport(std::string path, std::string filename)
{
    double txrxDist3d = (m_rx-m_tx).norm();

    std::string name = "";
    if (!m_terrain)
        name += "_noTerrain";
    if (!m_buildings)
        name += "_noBuildings";

    if (filename == "") {
        filename = std::to_string(m_tx.x)+"_"+std::to_string(m_tx.y)+"_"+std::to_string(m_tx.z)+"__"+std::to_string(m_rx.x)+"_"+std::to_string(m_rx.y)+"_"+std::to_string(m_rx.z)+name+".csv";
    }

    std::string data;

    data.append("tx;"+std::to_string(m_tx.x)+","+std::to_string(m_tx.y)+","+std::to_string(m_tx.z)+"\n");
    data.append("rx;"+std::to_string(m_rx.x)+","+std::to_string(m_rx.y)+","+std::to_string(m_rx.z)+"\n");
    data.append("indoorDist;"+std::to_string(m_indoorDist)+"\n");
    data.append("terrainDist;"+std::to_string(m_terrainDist)+"\n");
    data.append("txrxDist;"+std::to_string(txrxDist3d)+"\n");
    data.append("dist;");
    for (unsigned int i = 0; i < m_eDist.size(); i++)
        data.append(std::to_string(m_eDist.at(i))+",");
    data.append("\n elevation;");
    for (unsigned int i = 0; i < m_elevation.size(); i++)
        data.append(std::to_string(m_elevation.at(i))+",");
    data.append("\n bDist;");
    for (unsigned int i = 0; i < m_bDist.size(); i++)
        data.append(std::to_string(m_bDist.at(i))+",");
    data.append("\n bAltitude;");
    for (unsigned int i = 0; i < m_bAltitude.size(); i++)
        data.append(std::to_string(m_bAltitude.at(i))+",");
    data.append("\n iDist;");
    for (unsigned int i = 0; i < m_iDist.size(); i++)
        data.append(std::to_string(m_iDist.at(i))+",");
    data.append("\n iAltitude;");
    for (unsigned int i = 0; i < m_iAltitude.size(); i++)
        data.append(std::to_string(m_iAltitude.at(i))+",");
    data.append("\n tDist;");
    for (unsigned int i = 0; i < m_tDist.size(); i++)
        data.append(std::to_string(m_tDist.at(i))+",");
    data.append("\n tAltitude;");
    for (unsigned int i = 0; i < m_tAltitude.size(); i++)
        data.append(std::to_string(m_tAltitude.at(i))+",");


    FileHandler fh;
    fh.write(data, path+filename);

}


double SectionalView::getBuildingAltitude(double _dist)
{
    if (m_bDist.size() == 0)
        return -std::numeric_limits<double>::infinity();
    if (_dist < m_bDist.front() || _dist > m_bDist.back())
        return -std::numeric_limits<double>::infinity();

    for (unsigned int i = 1; i < m_bDist.size(); i++) {
        if (_dist >= m_bDist.at(i-1) && _dist < m_bDist.at(i))
            return m_bAltitude.at(i-1);
    }

    return std::numeric_limits<double>::infinity();
}


double SectionalView::getElevation(double _dist)
{
    if (_dist < m_eDist.front() || _dist > m_eDist.back())
        return std::numeric_limits<double>::infinity();

    for (unsigned int i = 1; i < m_eDist.size(); i++) {
        if (_dist >= m_eDist.at(i-1) && _dist < m_eDist.at(i)) {
            return calculateFunctionValue(std::make_pair(m_eDist.at(i-1), m_elevation.at(i-1)), std::make_pair(m_eDist.at(i), m_elevation.at(i)), _dist);
        }
    }

    return std::numeric_limits<double>::infinity();
}


double SectionalView::getDirectPathAltitude(double _dist)
{
    return calculateFunctionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), _dist);
}


DistanceElement SectionalView::getDistances()
{
    return DistanceElement(m_indoorDist, m_terrainDist, m_txrxDist3d);
}


int SectionalView::getIntersectionNumber()
{
    return m_iDist.size() + m_tDist.size();
}


std::pair<int, int> SectionalView::getIntersectionNumbers()
{
    return std::make_pair(m_iDist.size(), m_tDist.size());
}


void SectionalView::setBorderMultiplier(double _borderMultiplier)
{
    if (_borderMultiplier >= 0)
        m_borderMultiplier = _borderMultiplier;
}


double SectionalView::getMinimumElevation()
{
    if (!m_terrain)
        return 0.0;
    return m_minElevation;
}


double SectionalView::getMaximumElevation()
{
    if (!m_terrain)
        return 0.0;
    return m_maxElevation;
}


std::pair<std::vector<double>, std::vector<double>> SectionalView::getBuildingVectors()
{
    return std::make_pair(m_bDist, m_bAltitude);
}


std::pair<std::vector<double>, std::vector<double>> SectionalView::getBuildingIntersectionVectors()
{
    return std::make_pair(m_iDist, m_iAltitude);
}


std::pair<std::vector<double>, std::vector<double>> SectionalView::getElevationVectors()
{
    return std::make_pair(m_eDist, m_elevation);
}


std::pair<std::vector<double>, std::vector<double>> SectionalView::getTerrainIntersectionVectors()
{
    return std::make_pair(m_tDist, m_tAltitude);
}



// ------- private ------------
std::pair<int, int> SectionalView::getCellCoordinates2Pos(Vector3d _pos)
{
    int x = (_pos.x - m_heightProfile->getOffset().x) / m_cellSize_m;
    int y = (_pos.y - m_heightProfile->getOffset().y) / m_cellSize_m;

    if (x < 0 ||x > m_numColumns
            || y < 0 || y > m_numRows) {
        x = -1;
        y = -1;
    }

    return std::make_pair(x, y);
}


std::pair<double, double> SectionalView::getPos2CellCoordinates(int _x, int _y)
{
    double pos_x = _x * m_cellSize_m + m_heightProfile->getOffset().x;
    double pos_y = _y * m_cellSize_m + m_heightProfile->getOffset().y;
    pos_x += m_cellSize_m/2;
    pos_y += m_cellSize_m/2;

    return std::make_pair(pos_x, pos_y);
}


void SectionalView::insertIntersection(std::vector<std::pair<double, double>> &_intersections, const std::pair<double, double> &_entry)
{
    if(_intersections.empty())
        _intersections.push_back(_entry);
    else {
        for (std::vector<std::pair<double, double>>::iterator it = _intersections.begin(); it != _intersections.end(); ++it) {
            if (_entry.first < it->first) {
                _intersections.insert(it, _entry);
                break;
            }
            else if (it == _intersections.end()-1) {
                _intersections.push_back(_entry);
                break;
            }
        }
    }
}


Vector3d SectionalView::checkInside(Vector3d _v0, Vector3d _v1, Vector3d _pos)
{
    double x_min =_v0.x <_v1.x ?_v0.x :_v1.x;
    int x_min_int = int(x_min);
    x_min -= x_min_int;
    x_min = (int)(x_min * 1000000.0);

    double x_max =_v0.x >_v1.x ?_v0.x :_v1.x;
    int x_max_int = int(x_max);
    x_max -= x_max_int;
    x_max = (int)(x_max * 1000000.0);

    double y_min =_v0.y <_v1.y ?_v0.y :_v1.y;
    int y_min_int = int(y_min);
    y_min -= y_min_int;
    y_min = (int)(y_min * 1000000.0);

    double y_max =_v0.y >_v1.y ?_v0.y :_v1.y;
    int y_max_int = int(y_max);
    y_max -= y_max_int;
    y_max = (int)(y_max * 1000000.0);

    double pos_x = _pos.x;
    int pos_x_int = int(_pos.x);
    pos_x -= pos_x_int;
    pos_x = (int)(pos_x * 1000000.0);

    double pos_y = _pos.y;
    int pos_y_int = int(_pos.y);
    pos_y -= pos_y_int;
    pos_y = (int)(pos_y * 1000000.0);


    bool x = false;
    bool y = false;
    bool z = false;

    if (pos_x_int > x_min_int || (pos_x_int == x_min_int && pos_x >= x_min))
        if (pos_x_int < x_max_int || (pos_x_int == x_max_int && pos_x <= x_max))
            x = true;

    if (pos_y_int > y_min_int || (pos_y_int == y_min_int && pos_y >= y_min))
        if (pos_y_int < y_max_int || (pos_y_int == y_max_int && pos_y <= y_max))
            y = true;

    return Vector3d(x, y, z);
}

void SectionalView::elevationInformation(std::pair<int, int> _from_coor, std::pair<int, int> _to_coor)
{
    // get cells on line between transmitter and receiver
    std::vector<CellElement> cellsOfInterest = bresenham(_from_coor.first, _from_coor.second, _to_coor.first, _to_coor.second);

    for (unsigned int i = 0; i < cellsOfInterest.size(); i++) {
        // get elevation information
        double curElevation = m_heightProfile->getCell(cellsOfInterest.at(i).col, cellsOfInterest.at(i).row)->center.z;

        // update minimum and maximum
        if (curElevation > m_maxElevation)
            m_maxElevation = curElevation;
        if (curElevation < m_minElevation)
            m_minElevation = curElevation;

        // calculate distance between start & current point considerung also negative directions
        std::pair<double, double> pos = getPos2CellCoordinates(cellsOfInterest.at(i).row, cellsOfInterest.at(i).col);
        double curDist = calculateDistance2d(m_tx.x, m_tx.y, pos.first, pos.second);
        double rxDist = calculateDistance2d(m_rx.x, m_rx.y, pos.first, pos.second);

        if (rxDist > m_txrxDist2d && rxDist > curDist)
            curDist *= -1;

        m_elevation.push_back(curElevation);
        m_eDist.push_back(curDist);
    }

    // check if tx and rx are inside range -> otherwise add elements
    if (m_txrxDist2d > m_eDist.back()) {
        Vector3d dir = (m_rx-m_tx).normed();
        Vector3d tmp = m_rx + (dir * m_cellSize_m);
        std::pair<int, int> cell = getCellCoordinates2Pos(tmp);
        std::pair<double, double> pos = getPos2CellCoordinates(cell.first, cell.second);
        double curElevation = m_heightProfile->getCell(cell.second, cell.first)->center.z;
        double curDist = calculateDistance2d(m_tx.x, m_tx.y, pos.first, pos.second);
        m_elevation.push_back(curElevation);
        m_eDist.push_back(curDist);

        // update minimum and maximum
        if (curElevation > m_maxElevation)
            m_maxElevation = curElevation;
        else if (curElevation < m_minElevation)
            m_minElevation = curElevation;
    }
    if (0 < m_eDist.front()) {
        Vector3d dir = (m_rx-m_tx).normed();
        Vector3d tmp = m_tx - (dir * m_cellSize_m);
        std::pair<int, int> cell = getCellCoordinates2Pos(tmp);
        std::pair<double, double> pos = getPos2CellCoordinates(cell.first, cell.second);
        double curElevation = m_heightProfile->getCell(cell.second, cell.first)->center.z;
        double curDist = calculateDistance2d(m_tx.x, m_tx.y, pos.first, pos.second);
        m_elevation.insert(m_elevation.begin(), curElevation);
        m_eDist.insert(m_eDist.begin(), -curDist);

        // update minimum and maximum
        if (curElevation > m_maxElevation)
            m_maxElevation = curElevation;
        else if (curElevation < m_minElevation)
            m_minElevation = curElevation;
    }
}


void SectionalView::terrainIntersections()
{
    bool below = false;
    m_terrainDist = 0;

    for (unsigned int i = 0; i < m_elevation.size(); i++) {
        double curDist = m_eDist.at(i);

        // get txrx altitude at curDist
        double pos_z = calculateFunctionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), curDist);

        if (below && pos_z >= m_elevation.at(i)) {
            // get intersection of elevation profile and direct path
            std::pair<double, double> is = calculate2dIntersection(std::make_pair(m_eDist.at(i-1), m_elevation.at(i-1)), std::make_pair(m_eDist.at(i), m_elevation.at(i)), std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z));

            if ((m_tDist.size() > 0 && is.first <= m_txrxDist2d && is.first >= 0) || (is.first <= m_txrxDist2d && is.first >= 0.2)){
                m_tDist.push_back(is.first);
                m_tAltitude.push_back(is.second);

                if (m_tDist.size() > 1)
                    m_terrainDist += calculateDistance2d(is.first, is.second, m_tDist.at(m_tDist.size()-2), m_tAltitude.at(m_tAltitude.size()-2));
                else
                    m_terrainDist += calculateDistance2d(is.first, is.second, 0, m_tx.z);
            }
//            else
//                std::cout<<"error: terrain intersections"<<std::endl;

            below = false;
        }
        else if (!below && pos_z <= m_elevation.at(i)) {
            if (i > 0) {
                // get intersection of elevation profile and direct path
                std::pair<double, double> is = calculate2dIntersection(std::make_pair(m_eDist.at(i-1), m_elevation.at(i-1)), std::make_pair(m_eDist.at(i), m_elevation.at(i)), std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z));

                if (is.first <= m_txrxDist2d && is.first >= 0) {
                    m_tDist.push_back(is.first);
                    m_tAltitude.push_back(is.second);
                }
            }
            below = true;
        }
    }
}

void SectionalView::evaluateIntersection(Vector3d _intersection, std::vector<BuildingElement>& _intersections, std::vector<std::pair<double, double>>& _intersectionPoints3D, double _altitude, bool is_info)
{

    // calculate distance from tx considerung also negative directions
    double curDist = calculateDistance2d(m_tx.x, m_tx.y, _intersection.x, _intersection.y);
    double rxDist = calculateDistance2d(m_rx.x, m_rx.y, _intersection.x, _intersection.y);

    if (rxDist > m_txrxDist2d && rxDist > curDist)
        curDist *= -1;

    // add to _intersection vector by ascending x (as second sorting rule: ascending y)
    if (_intersections.empty())
        _intersections.push_back(BuildingElement(_intersection, curDist));
    else {
        for (std::vector<BuildingElement>::iterator it = _intersections.begin(); it != _intersections.end(); ++it) {
            if (curDist < it->dist) {
                _intersections.insert(it, BuildingElement(_intersection, curDist));
                break;
            }
            else if (it == _intersections.end()-1) {
                _intersections.push_back(BuildingElement(_intersection, curDist));
                break;
            }
        }
    }


    if (is_info) {
        // check, if 3D valid intersection
        if (m_terrain) {
            for (unsigned int j = 0; j < m_eDist.size(); j++) {
                if (m_eDist.at(j) >= curDist && j > 0) {
                    double curElevation = calculateFunctionValue(std::make_pair(m_eDist.at(j-1), m_elevation.at(j-1)), std::make_pair(m_eDist.at(j), m_elevation.at(j)), curDist);
                    double z = calculateFunctionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), curDist);

                    if (z >= curElevation && z <= _altitude && curDist <= m_txrxDist2d && 0 <= curDist) {
                        insertIntersection(_intersectionPoints3D, std::make_pair(curDist, z));
                    }
                    break;
                }
            }
        }
        else {
            double z = calculateFunctionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), curDist);

            if (z >= 0 && z <= _altitude && curDist <= m_txrxDist2d && 0 <= curDist) {
                insertIntersection(_intersectionPoints3D, std::make_pair(curDist, z));
            }
        }
    }

}


std::vector<std::pair<double, double>> SectionalView::buildingInformation(bool is_info)
{
    // this function calculates the intersections between the direct path and the buildings
    // in addition information about the buildings, which occur on the 2D-projection, are returned

    // set bounds
    Vector3d min;
    Vector3d max;

    if (m_tx.x > m_rx.x) {
        min.x = m_rx.x;
        max.x = m_tx.x;
    }
    else {
        min.x = m_tx.x;
        max.x = m_rx.x;
    }

    if (m_tx.y > m_rx.y) {
        min.y = m_rx.y;
        max.y = m_tx.y;
    }
    else {
        min.y = m_tx.y;
        max.y = m_rx.y;
    }

    std::vector<std::pair<double, double>> buildingIntersections;

    // iterate through all buildings
    for(auto it : World::getInstance()->getBuildings())
    {
        // save 3d intersection points
        std::vector<std::pair<double, double>> intersectionPoints3D;

        // save building information
        Building *building = it.second;
        std::vector<Node*> nodes = building->getNodes();
        std::vector<std::vector<Node*>> innerNodes = building->getInnerNodes();
        std::vector<BuildingElement> intersections;
        double altitude = m_terrain ? building->getElevation() + building->getHeight() : building->getHeight();

        // check, if starting and end point is outside of the building
        Bounds b = building->getBounds();

        if (b.getMax().x < min.x || b.getMax().y < min.y || b.getMin().x > max.x || b.getMin().y > max.y)
            continue;

        Vector3d dir = (m_rx-m_tx).normed();
        Vector3d from = m_tx;
        Vector3d to = m_rx;

        Vector3d tx_check = b.checkInside(from);
        Vector3d rx_check = b.checkInside(to);

        while (tx_check.x && tx_check.y) {
            from = from - dir;
            tx_check = b.checkInside(from);
        }

        while (rx_check.x && rx_check.y) {
            to = to + dir;
            rx_check = b.checkInside(to);
        }

        // iterate through all nodes of building under investigation
        for (unsigned int i = 0; i < nodes.size(); i++) {
            // save points, which describe current wall, and add elevation to z coordinate
            Vector3d v0 = nodes.at(i)->getPosition();
            Vector3d v1 = nodes.at((i+1) % nodes.size())->getPosition();
            if (m_terrain) {
                v0.z += building->getElevation();
                v1.z += building->getElevation();
            }

            if(v0.toString() != v1.toString())
            {
                Plane3d plane(v0, v1, v0+Vector3d(0, 0, building->getHeight()));
                Vector3d intersection = plane.computeIntersection(from, to);

                if(!intersection.valid)
                    continue;

                Vector3d c = checkInside(v0, v1, intersection);

                // check, if intersection is valid with regard to the x and y coordinate
                if (c.x && c.y)
                    evaluateIntersection(intersection, intersections, intersectionPoints3D, altitude, is_info);
            }
        }

        // check also inner paths
        if (innerNodes.size() > 0) {
            for (unsigned int i = 0; i < innerNodes.size(); i++) {
                for (unsigned int j = 0; j < innerNodes.at(i).size(); j++) {
                    Vector3d v0 = innerNodes.at(i).at(j)->getPosition();
                    Vector3d v1 = innerNodes.at(i).at((j+1) % innerNodes.at(i).size())->getPosition();
                    if (m_terrain) {
                        v0.z += building->getElevation();
                        v1.z += building->getElevation();
                    }

                    if(v0.toString() != v1.toString())
                    {
                        Plane3d plane(v0, v1, v0+Vector3d(0, 0, building->getHeight()));
                        Vector3d intersection = plane.computeIntersection(from, to);

                        if(!intersection.valid)
                            continue;

                        Vector3d c = checkInside(v0, v1, intersection);

                        // check, if intersection is valid with regard to the x and y coordinate
                        if (c.x && c.y)
                            evaluateIntersection(intersection, intersections, intersectionPoints3D, altitude, is_info);
                    }
                }
            }
        }

        if (is_info) {
            // check for missing roof and ground intersections
            if ((intersectionPoints3D.size() % 2 == 1 && intersections.size() != intersectionPoints3D.size()) || (intersections.size() > 0 && intersections.at(0).dist < 0)) {
                bool onlyNegative = true;
                for (unsigned int k = 0; k < intersections.size(); k ++) {
                    if (intersections.at(k).dist > 0) {
                        onlyNegative = false;
                        break;
                    }
                }

                if (!onlyNegative) {
                    unsigned int j = -1;
                    bool found = false;
                    std::vector<std::pair<double, double>> validIs;
                    for (std::vector<std::pair<double, double>>::iterator it = intersectionPoints3D.begin(); it != intersectionPoints3D.end(); ++it) {
                        if (it->first <= m_txrxDist2d && 0 <= it->first)
                            insertIntersection(validIs, (*it));
    //                        insertIntersection(buildingIntersections, (*it));

                        if (!found) {
                            j++; // increased at least about one
                            // get corresponding 2D intersection
                            while (intersections.at(j).dist != it->first)
                                j++;

                            double z, startDist, endDist;

                            if (j % 2 == 1) {
                                // odd index of 2D intersection -> end of building
                                if (it == intersectionPoints3D.begin() || intersections.at(j-1).dist != (it-1)->first) {
                                    // start of building missing -> check z coordinate of previous 2D-intersection
                                    startDist = intersections.at(j-1).dist;
                                    endDist = intersections.at(j).dist;
                                    z = calculateFunctionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), startDist);
                                }
                                else
                                    continue;
                            }
                            else {
                                // even index of 2D intersection -> start of building
                                if (it == intersectionPoints3D.end()-1 || intersections.at(j+1).dist != (it+1)->first) {
                                    // end of building missing -> check z coordinate of following 2D-intersection
                                    startDist = intersections.at(j).dist;
                                    if (j == intersections.size())
                                        continue;
                                    endDist = intersections.at(j+1).dist;
                                    z = calculateFunctionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), endDist);
                                }
                                else
                                    continue;
                            }

                            // roof intersection
                            if (z >= altitude) {
                                double isDist = calculatePositionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), altitude);
                                if (isDist <= m_txrxDist2d && 0 <= isDist)
                                    insertIntersection(validIs, std::make_pair(isDist, altitude));
                                found = true;
                            }
                            // ground intersection
                            else {
                                if (m_terrain) {
                                    for (unsigned int i = 0; i < m_tDist.size(); i++) {
                                        if (m_tDist.at(i) >= startDist && m_tDist.at(i) <= endDist) {
                                            insertIntersection(validIs, std::make_pair(m_tDist.at(i), m_tAltitude.at(i)));
                                            break;
                                        }

                                    }
                                }
                                else {
                                    double isDist = calculatePositionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), 0);
                                    if (isDist <= m_txrxDist2d && 0 <= isDist)
                                        insertIntersection(validIs, std::make_pair(isDist, 0));
                                    found = true;
                                }
    //                            found = true;
                            }
                        }
                    }
                    if (validIs.size() % 2 == 0) {
                        for (std::vector<std::pair<double, double>>::iterator it = validIs.begin(); it != validIs.end(); ++it)
                            insertIntersection(buildingIntersections, (*it));
                    }
                    else {
                        if (validIs.front().first == 0)
                            validIs.erase(validIs.begin());
                        for (std::vector<std::pair<double, double>>::iterator it = validIs.begin(); it != validIs.end(); ++it)
                            insertIntersection(buildingIntersections, (*it));
                    }
                }
            }
            else {
                for (std::vector<std::pair<double, double>>::iterator it = intersectionPoints3D.begin(); it != intersectionPoints3D.end(); ++it)
                    if (it->first <= m_txrxDist2d && 0 <= it->first)
                        insertIntersection(buildingIntersections, (*it));
            }

            // check exceptional case: roof and ground intersection on same building
            for (unsigned int i = 1; i < intersections.size(); i+=2) {
                if (m_terrain) {
                    for (unsigned int j = 0; j < m_tDist.size(); j++) {
                        if (intersections.at(i-1).dist <= m_tDist.at(j) && intersections.at(i).dist >= m_tDist.at(j)) {
                            if (intersections.at(i-1).pos.z >= altitude || intersections.at(i).pos.z >= altitude) {
                                insertIntersection(buildingIntersections, std::make_pair(m_tDist.at(j), m_tAltitude.at(j)));
                                double isDist = calculatePositionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), altitude);
                                insertIntersection(buildingIntersections, std::make_pair(isDist, altitude));
                            }
                        }
                    }
                }
                else {
                    if ((intersections.at(i-1).dist < 0 && intersections.at(i).dist > altitude) || (intersections.at(i-1).dist > altitude && intersections.at(i).dist < 0)) {
                        double isDist = calculatePositionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), 0);
                        if (isDist <= m_txrxDist2d && 0 <= isDist)
                            insertIntersection(buildingIntersections, std::make_pair(isDist, 0));
                        isDist = calculatePositionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), altitude);
                        if (isDist <= m_txrxDist2d && 0 <= isDist)
                            insertIntersection(buildingIntersections, std::make_pair(isDist, altitude));
                    }

                }
            }
        }

        if (intersections.size() % 2 == 1) {
            std::cout<<"error: odd number of intersections found"<<std::endl;
        }
        else {
            if (is_info) {
                // check if rx or tx indoor -> check for missing intersections
                if (intersections.size() % 2 == 0) {
                    for (unsigned int i = 1; i < intersections.size(); i++) {
                        double z = std::numeric_limits<double>::infinity();
                        if (intersections.at(i-1).dist < 0 && intersections.at(i).dist > 0 && altitude > m_tx.z)
                            // tx indoor
                            z = calculateFunctionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), intersections.at(i).dist);
                        else if (intersections.at(i-1).dist <= m_txrxDist2d - 0.00001 && intersections.at(i).dist >= m_txrxDist2d + 0.00001 && altitude > m_rx.z)
                            // rx indoor
                            z = calculateFunctionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), intersections.at(i-1).dist);

                        if (z != std::numeric_limits<double>::infinity()) {
                            // roof intersection
                            if (z >= altitude) {
                                double isDist = calculatePositionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), altitude);
                                insertIntersection(buildingIntersections, std::make_pair(isDist, altitude));
                            }
                            // ground intersection
                            else {
                                if (m_terrain) {
                                    for (unsigned int j = 0; j < m_tDist.size(); j++)
                                        if (intersections.at(i-1).dist <= m_tDist.at(j) && intersections.at(i).dist >= m_tDist.at(j))
                                            insertIntersection(buildingIntersections, std::make_pair(m_tDist.at(j), m_tAltitude.at(j)));
                                }
                                else {
                                    double isDist = calculatePositionValue(std::make_pair(0, m_tx.z), std::make_pair(m_txrxDist2d, m_rx.z), 0);
                                    insertIntersection(buildingIntersections, std::make_pair(isDist, 0));
                                }
                            }
                        }
                    }
                }
            }

            // merge new intersections to vectors
            if (m_bDist.empty()) {
                for (unsigned int i = 0; i < intersections.size(); i++) {
                    if (i % 2 == 0) {
                        m_bDist.push_back(intersections.at(i).dist);
                        m_bAltitude.push_back(altitude);
                    }
                    else {
                        m_bDist.push_back(intersections.at(i).dist);
                        m_bAltitude.push_back(0);
                    }
                }
            }
            else {
                int tmp = 0;
                std::vector<double> tmpAltitude;

                for (unsigned int i = 0; i < intersections.size(); i += 2) {
                    double dist = intersections.at(i).dist;
                    unsigned int j = tmp;

                    while (j < m_bDist.size()) {
                        if (dist < m_bDist.at(j)) {
                            // insert start
                            if (j == 0)
                                m_bAltitude.insert(m_bAltitude.begin() + j, altitude);
                            else
                                m_bAltitude.insert(m_bAltitude.begin() + j, (m_bAltitude.at(j-1) > altitude) ? m_bAltitude.at(j-1) : altitude);
                            m_bDist.insert(m_bDist.begin() + j, dist);

                            // insert end
                            if (m_bDist.at(j + 1) > intersections.at(i+1).dist) {
                                m_bDist.insert(m_bDist.begin() + j+1, intersections.at(i+1).dist);
                                if (j == 0)
                                    m_bAltitude.insert(m_bAltitude.begin() + j + 1, 0);
                                else
                                    m_bAltitude.insert(m_bAltitude.begin() + j + 1, (m_bAltitude.at(j-1) > 0) ? m_bAltitude.at(j-1) : 0);
                                tmp = j+1;
                                break;
                            }
                            else {
                                unsigned int k = j + 1;
                                tmpAltitude = m_bAltitude;
                                while (k < m_bDist.size() && m_bDist.at(k) < intersections.at(i+1).dist) {
                                    if (tmpAltitude.at(k) < altitude)
                                        tmpAltitude[k] = altitude;
                                    k++;
                                }
                                tmpAltitude.insert(tmpAltitude.begin() + k, (m_bAltitude.at(k-1) > 0) ? m_bAltitude.at(k-1) : 0);
                                m_bAltitude = tmpAltitude;
                                m_bDist.insert(m_bDist.begin()+ k, intersections.at(i+1).dist);
                                tmp = k;
                                break;
                            }
                        }
                        else if (j == m_bDist.size() - 1) {
                            m_bDist.insert(m_bDist.begin() + j+1, dist);
                            m_bAltitude.insert(m_bAltitude.begin() + j+1, altitude);
                            m_bDist.insert(m_bDist.begin() + j+2, intersections.at(i+1).dist);
                            m_bAltitude.insert(m_bAltitude.begin() + j+2,0);
                            tmp = j+2;
                            break;
                        }
                        else {
                            j++;
                        }
                    }
                }
            }
        }
    }

    // add small range at the start and end
    double altitude = 0;
    if (m_terrain) {
        while (m_bDist.size() > 0 && m_bDist.back() > m_eDist.back()) {
            m_bDist.pop_back();
            m_bAltitude.pop_back();
        }
        while (m_bDist.size() > 0 && m_bDist.front() < m_eDist.front()) {
            m_bDist.erase(m_bDist.begin());
            altitude = m_bAltitude.front();
            m_bAltitude.erase(m_bAltitude.begin());
        }
        m_bDist.insert(m_bDist.begin(), m_eDist.front());
        m_bDist.push_back(m_eDist.back());
    }
    else {
        while (m_bDist.size() > 0 && m_bDist.back() > m_txrxDist2d) {
            m_bDist.pop_back();
            m_bAltitude.pop_back();
        }
        while (m_bDist.size() > 0 && m_bDist.front() < 0) {
            m_bDist.erase(m_bDist.begin());
            altitude = m_bAltitude.front();
            m_bAltitude.erase(m_bAltitude.begin());
        }
        m_bDist.insert(m_bDist.begin(), 0);
        m_bDist.push_back(m_txrxDist2d);
    }

    m_bAltitude.insert(m_bAltitude.begin(), altitude);
    m_bAltitude.push_back(0);

    return buildingIntersections;
}

}
