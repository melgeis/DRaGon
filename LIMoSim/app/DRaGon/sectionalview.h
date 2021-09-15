#ifndef SECTIONALVIEW_H
#define SECTIONALVIEW_H

#include "LIMoSim/world/vector3d.h"
#include "LIMoSim/world/heightprofile.h"

#include "utilities.h"

#include <limits>

namespace LIMoSim
{

struct BuildingElement
{
    Vector3d pos;
    double dist;

    BuildingElement(Vector3d _pos, double _dist) {
        pos = _pos;
        dist = _dist;
    }
};

struct DistanceElement
{
    double indoorDist = -1;
    double terrainDist = -1;
    double txrxDist = -1;

    DistanceElement() {

    }

    DistanceElement(double _indoorDist, double _terrainDist, double _txrxDist) {
        indoorDist = _indoorDist;
        terrainDist = _terrainDist;
        txrxDist = _txrxDist;
    }

    DistanceElement & operator=(DistanceElement &o) {
        indoorDist = o.indoorDist;
        terrainDist = o.terrainDist;
        txrxDist = o.txrxDist;

        return *this;
    }
};


class SectionalView
{

private:
    Vector3d m_tx;
    Vector3d m_rx;

    double m_borderMultiplier = 0;

    double m_cellSize_m = -1;
    double m_numColumns = -1;
    double m_numRows = -1;

    double m_txrxDist2d = -1;
    double m_txrxDist3d = -1;

    double m_indoorDist = 0;
    double m_terrainDist = 0;

    double m_minElevation = std::numeric_limits<double>::infinity();
    double m_maxElevation = -std::numeric_limits<double>::infinity();

    std::vector<double> m_eDist, m_elevation;
    std::vector<double> m_bDist, m_bAltitude;
    std::vector<double> m_iDist, m_iAltitude;
    std::vector<double> m_tDist, m_tAltitude;

    HeightProfile* m_heightProfile = nullptr;

    bool m_terrain = true;
    bool m_buildings = true;
    bool m_executed = false;

public:
    SectionalView(Vector3d _tx, Vector3d _rx, bool _terrain = true, bool _buildings = true);
    ~SectionalView();

    SectionalView & operator=(SectionalView &o);

    void resetValues();
    void run(bool is_info=true);
    void csvExport(std::string path, std::string filename = "");
    void print();

    void useBuildings(bool _use);
    void useTerrain(bool _use);
    void setBorderMultiplier(double _borderMultiplier);

    double getBuildingAltitude(double _dist);
    double getElevation(double _dist);
    double getDirectPathAltitude(double _dist);
    int getIntersectionNumber();
    std::pair<int, int> getIntersectionNumbers();
    double getMinimumElevation();
    double getMaximumElevation();

    std::pair<std::vector<double>, std::vector<double>> getBuildingVectors();
    std::pair<std::vector<double>, std::vector<double>> getBuildingIntersectionVectors();
    std::pair<std::vector<double>, std::vector<double>> getElevationVectors();
    std::pair<std::vector<double>, std::vector<double>> getTerrainIntersectionVectors();

    DistanceElement getDistances();    

private:
    std::pair<int, int> getCellCoordinates2Pos(Vector3d _pos);
    std::pair<double, double> getPos2CellCoordinates(int _x, int _y);
    void evaluateIntersection(Vector3d _intersection, std::vector<BuildingElement>& _intersections, std::vector<std::pair<double, double>>& _intersectionPoints3D, double _altitude, bool is_info);
    void insertIntersection(std::vector<std::pair<double, double>> &_intersections, const std::pair<double, double> &_entry);
    Vector3d checkInside(Vector3d _v0, Vector3d _v1, Vector3d _pos);

    void elevationInformation(std::pair<int, int> _from_coor, std::pair<int, int> _to_coor);
    void terrainIntersections();
    std::vector<std::pair<double, double>> buildingInformation(bool is_info);
};

}
#endif // SECTIONALVIEW_H

// https://www.openstreetmap.org/way/24710100
// https://www.openstreetmap.org/way/24710099
