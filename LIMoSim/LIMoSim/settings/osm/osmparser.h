#ifndef LIMOSIM_OSMPARSER_H
#define LIMOSIM_OSMPARSER_H

#include "LIMoSim/settings/domelement.h"
#include "LIMoSim/world/world.h"
#include <map>

namespace LIMoSim
{

namespace COORDINATE_SYSTEM
{
    enum{
        WGS84,
        CARTESIAN
    };
}

class OSMParser
{
public:
    OSMParser();

    void parse(DOMElement *_document, bool _buildings = false);
    void parseNode(DOMElement *_node);
    void parseWay(DOMElement *_way);
    void parseRelation(DOMElement *_relation);

    Building* parseBuilding(const std::string &_id, std::map<std::string, Variant> &_tags);

    std::map<std::string, Variant> readTags(DOMElement *_element);


private:
    World *p_world;
    int m_system;
    bool m_buildings;

    std::map<std::string, DOMElement*> m_ways;

    int height_counter = 0;
    int level_counter = 0;
    int buildings = 0;
};

}


#endif // LIMOSIM_OSMPARSER_H
