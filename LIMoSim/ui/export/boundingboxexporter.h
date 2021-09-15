#ifndef BOUNDINGBOXEXPORTER_H
#define BOUNDINGBOXEXPORTER_H

#include "ui/export/epsdocument.h"
#include "LIMoSim/world/world.h"
#include "boundingbox.h"

/* w,h const
 *
 * 1. determine box between TX and RX
 * 2. scale the box
 * 3. the find all obstacles within
 * 3. rotate the box
 * 4. export 2D eps
 *
*/



namespace LIMoSim
{

namespace INTERSECTION_TYPE
{
    enum{
        NONE,
        LEFT,
        RIGHT,
        BOTTOM,
        TOP
    };
}

struct BuildingNode
{
    std::string id = "";
    bool inner_nodes = false;

    std::vector<QPointF> outer;
    std::vector<std::vector<QPointF>> inner;

    BuildingNode(std::string _id) {
        id = _id;
    }
};



class BoundingBoxExporter
{
public:
    BoundingBoxExporter(int _size);

    void snapshot(const Vector3d &_rx, const Vector3d &_tx, const QString &_file);

    std::vector<QPointF> exportWay(const std::vector<Node*> &_nodes, const QString &_color);

    Vector3d checkIntersection(const Vector3d &_origin, const Vector3d &_dir, const Vector3d &_p0, const Vector3d &_p1);

    Vector3d rot(const Vector3d &_p);
    QPointF toPoint(const Vector3d &_p);
    bool isInside(const Vector3d &_p);
    double map(double _x, double _inMin, double _inMax, double _outMin, double _outMax);

    bool checkCondition(int _var0, int _var1, int _value0, int _value1);

    bool checkCandidate(Bounds bounds);

private:
    EpsDocument m_eps;

    BoundingBox m_box;
    double m_phi;
    double m_width;
    double m_height;

    int m_size;
    bool m_drawVegetation;
    bool m_drawEntities;

    Vector3d m_origin;

    std::vector<BuildingNode> m_nodeList;
};

}

#endif // BOUNDINGBOXEXPORTER_H
