#include "world.h"
#include "LIMoSim/settings/osm/osmparser.h"
#include "LIMoSim/settings/xmlparser.h"
#include <algorithm>
#include <limits>

namespace LIMoSim
{

World::World()
{
}

World::~World()
{
    clearNodes();
}

World* World::getInstance()
{
    static World instance;
    return &instance;
}


/*************************************
 *            PUBLIC METHODS         *
 ************************************/

void World::loadMap(const std::string &_path)
{
    XMLParser xml;
    DOMElement *map = xml.parse(_path);
    if(map)
    {
        std::cout << "parse OSM " << _path << std::endl;
        OSMParser osm;
        osm.parse(map, true);

        //FileHandler::write(osm.toString(), _path + ".limo");
    }
}

void World::updateBox(const Vector3d &_point)
{
    m_bounds.update(_point);
}


void World::adjustTerrainProfile()
{
    for(auto &it : m_buildings)
    {
        Building *building = it.second;
        Vector3d center = building->computeCenter();
        building->setElevation(m_heightProfile.getElevation(center));
    }
}

void World::filterNodes()
{
    std::map<std::string, Node*>::iterator it;
    for(it=m_nodes.begin(); it!=m_nodes.end();)
    {
        Node *node = it->second;
        if(node->getWays().size()==0)
        {
            m_nodes.erase(it++);
            delete node;
        }
        else
            it++;

    }
}

/*************************************
 *          METHODS: CREATION        *
 ************************************/

Node* World::createNode(const std::string &_id, const Vector3d &_position)
{
    Node *node = new Node(_id, _position);
    m_nodes[_id] = node;


    return node;
}

Building* World::createBuilding(const std::string &_id)
{
    Building *building = new Building(_id);
    m_buildings[_id] = building;

    return building;
}

/*************************************
 *        METHODS: DESTRUCTION       *
 ************************************/

void World::clearNodes()
{
    for(auto &it : m_nodes)
        delete it.second;
    m_nodes.clear();
}


/*************************************
 *              ACCESSORS            *
 ************************************/

void World::setReference(const Vector3d &_origin)
{
    m_reference = _origin;

    std::cout << "set ref " << _origin.toString() << std::endl;
}

Vector3d World::getReference()
{
    return m_reference;
}

Bounds World::getBounds()
{
    return m_bounds;
}

Node* World::getNodeById(const std::string &_id)
{
    if(m_nodes.count(_id)>0)
        return m_nodes[_id];
    return 0;
}

const std::map<std::string, Node*>& World::getNodes()
{
    return m_nodes;
}

const std::map<std::string, Building*>& World::getBuildings()
{
    return m_buildings;
}

HeightProfile* World::getHeightProfile()
{
    return &m_heightProfile;
}

/*************************************
 *           PRIVATE METHODS         *
 ************************************/

}
