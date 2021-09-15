#ifndef LIMOSIM_BUILDING_H
#define LIMOSIM_BUILDING_H

#include "way.h"
#include "bounds.h"

namespace LIMoSim
{

class Building : public Way
{
public:
    Building(const std::string &_id);

    //
    virtual void addNode(Node *_node);
    void addInnerNodes(std::vector<Node*> _nodeList);
    Vector3d computeCenter();

    //
    void setName(const std::string &_name);
    void setHeight(double _height_m);
    void setElevation(double _elevation_m);
    std::string getName();
    double getHeight();
    double getElevation();
    Bounds getBounds();

    const std::vector<std::vector<Node*>>& getInnerNodes();

private:
    std::string m_name;
    double m_height_m;
    double m_elevation_m;

    Bounds m_bounds;

    std::vector<std::vector<Node*>> m_innerNodes;
};

}

#endif // BUILDING_H
