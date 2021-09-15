#include "building.h"

namespace LIMoSim
{

Building::Building(const std::string &_id) :
    Way(_id, "Building"),
    m_height_m(10),
    m_elevation_m(0)
{

}

/*************************************
 *            PUBLIC METHODS         *
 ************************************/

void Building::addNode(Node *_node)
{
    m_bounds.update(_node->getPosition());
    Way::addNode(_node);
}

void Building::addInnerNodes(std::vector<Node*> _nodeList)
{
    m_innerNodes.push_back(_nodeList);

    for (unsigned int i = 0; i < _nodeList.size(); i++)
        _nodeList.at(i)->registerWay(this);
}

const std::vector<std::vector<Node*>>& Building::getInnerNodes()
{
    return m_innerNodes;
}

Vector3d Building::computeCenter()
{
    Vector3d center;
    for(auto node : m_nodes)
        center = center + node->getPosition();
    center = center / m_nodes.size();

    return center;
}

void Building::setName(const std::string &_name)
{
    m_name = _name;
}

void Building::setHeight(double _height_m)
{
    m_height_m = _height_m;
}

void Building::setElevation(double _elevation_m)
{
    m_elevation_m = _elevation_m;
}

std::string Building::getName()
{
    return m_name;
}

double Building::getHeight()
{
    return m_height_m;
}

double Building::getElevation()
{
    return m_elevation_m;
}

Bounds Building::getBounds()
{
    return m_bounds;
}

}
