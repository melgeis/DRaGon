#include "osmparser.h"
#include <iostream>
#include "wgs84.h"

namespace LIMoSim
{

OSMParser::OSMParser() :
    p_world(0),
    m_system(COORDINATE_SYSTEM::CARTESIAN),
    m_buildings(false)
{
    p_world = World::getInstance();
}


/*************************************
 *            PUBLIC METHODS         *
 ************************************/

void OSMParser::parse(DOMElement *_document, bool _buildings)
{
    m_buildings = _buildings;

    std::string generator = _document->getAttribute("generator").toString();
    if(generator!="LIMoSim")
        m_system = COORDINATE_SYSTEM::WGS84;

    for(unsigned int i=0; i<_document->childNodes.size(); i++)
    {
        DOMElement *element = _document->childNodes.at(i)->toElement();
        std::string type = element->tagName;

        if(type=="bounds")
        {
            float minLat = element->getAttribute("minlat").toDouble();
            float minLon = element->getAttribute("minlon").toDouble();
            float maxLat = element->getAttribute("maxlat").toDouble();
            float maxLon = element->getAttribute("maxlon").toDouble();

            Vector3d distances_m = WGS84::computeOffset(Vector3d(maxLon, maxLat), Vector3d(minLon, minLat));
            std::cout << "OSMParser::parse " << distances_m.toString() << std::endl;

            p_world->setReference(Vector3d(minLon, minLat));
        }
        else if(type=="node")
            parseNode(element);
        else if(type=="way")
            parseWay(element);
        else if(type=="relation")
            parseRelation(element);
    }

    std::cout<<"height_counter "<<height_counter<<std::endl;
    std::cout<<"level_counter "<<level_counter<<std::endl;
    std::cout<<"buildings "<<buildings<<std::endl;
}

void OSMParser::parseNode(DOMElement *_node)
{
    std::string id = _node->getAttribute("id").toString();
    Vector3d position(_node->getAttribute("x").toDouble(), _node->getAttribute("y").toDouble());
    Node *node = p_world->createNode(id, position);

    std::map<std::string, Variant> tags = readTags(_node);
    if(m_system==COORDINATE_SYSTEM::WGS84)
    {
        Vector3d position(_node->getAttribute("lon").toDouble(), _node->getAttribute("lat").toDouble());
        Vector3d origin = p_world->getReference();
        if(origin.norm()==0)
        {
            std::cout << "OSMParser::parseNode WARNING world origin not set, using first node" << std::endl;

            p_world->setReference(position);
            origin = position;
        }

        Vector3d cartesian = WGS84::computeOffset(position, origin);
        node->setPosition(cartesian);
    }
}

void OSMParser::parseWay(DOMElement *_way)
{
    std::string id = _way->getAttribute("id").toString();
    std::vector<std::string> nodes;
    std::map<std::string, Variant> tags = readTags(_way);

    // read all nodes
    for(auto domNode : _way->childNodes)
    {
        DOMElement *child = domNode->toElement();
        if(child->tagName=="nd")
        {
            std::string node = child->getAttribute("ref").toString();
            nodes.push_back(node);
        }
    }

    //
    Way *way = 0;

    if(tags.count("building")>0 && m_buildings)
        way = parseBuilding(id, tags);

    if(way) // once the way type is known, add the nodes
    {
        for(auto nodeId : nodes)
        {
            Node *node = p_world->getNodeById(nodeId);
            way->addNode(node);

            if(way->getType()=="Building")
                p_world->updateBox(node->getPosition());

        }
    }

    m_ways[id] = _way;
}

void OSMParser::parseRelation(DOMElement *_relation)
{
    std::string id = _relation->getAttribute("id").toString();

    std::map<std::string, Variant> tags = readTags(_relation);
    if(tags.count("building")>0)
    {
        for(auto it : _relation->childNodes)
        {
            DOMElement *child = it->toElement();
            if(child->tagName=="member")
            {
                std::string role = child->getAttribute("role").toString();
                if(role=="outer") // TODO: we do not parse multipolygons here, just the outline is used to define the building
                {
                    std::string ref = child->getAttribute("ref").toString();
                    Building *building;
                    if (p_world->getBuildings().count(id)>0)
                        building = p_world->getBuildings().at(id);
                    else
                        building = p_world->createBuilding(id);

                    if(m_ways.count(ref)>0)
                    {
                        DOMElement *way = m_ways[ref];
                        for(auto domNode : way->childNodes)
                        {
                            DOMElement *child = domNode->toElement();
                            if(child->tagName=="nd")
                            {
                                std::string nodeId = child->getAttribute("ref").toString();
                                Node *node = p_world->getNodeById(nodeId);
                                building->addNode(node);
                            }
                        }
                    }
                    else
                        std::cout << "OSMParser::parseRelation missing way entry " << ref << std::endl;
                }
                else if (role == "inner") {
                    std::string ref = child->getAttribute("ref").toString();
                    Building *building;
                    if (p_world->getBuildings().count(id)>0)
                        building = p_world->getBuildings().at(id);
                    else
                        building = p_world->createBuilding(id);

                    std::vector<Node*> nodeList;
                    if(m_ways.count(ref)>0)
                    {
                        DOMElement *way = m_ways[ref];
                        for(auto domNode : way->childNodes)
                        {
                            DOMElement *child = domNode->toElement();
                            if(child->tagName=="nd")
                            {
                                std::string nodeId = child->getAttribute("ref").toString();
                                Node *node = p_world->getNodeById(nodeId);
                                nodeList.push_back(node);
                            }
                        }
                        building->addInnerNodes(nodeList);
                    }
                    else
                        std::cout << "OSMParser::parseRelation missing way entry " << ref << std::endl;

                }

                if (role == "inner") {
                    Building *building = p_world->getBuildings().at(id);
                    std::vector<std::vector<Node*>> innerNodes = building->getInnerNodes();
                }

            }
        }
    }
}

Building* OSMParser::parseBuilding(const std::string &_id, std::map<std::string, Variant> &_tags)
{
    Building *building = p_world->createBuilding(_id);

    // default: pseudo-random height
    int s = _id.size();
    std::string id = _id.substr(s-2, s-1);
    double v = -0.5 + atof(id.c_str())/99.0;
    double delta = 10 * 2;
    double height = 20 + v * delta;
    building->setHeight(height);

    for(auto it : _tags)
    {
        std::string key = it.first;
        Variant value = it.second;

        bool height = false;
        bool mapped_height = false;

        if(key=="name")
            building->setName(value.toString());
        else if (!height) {
            if(key=="height") {
                building->setHeight(value.toDouble());
                height = true;
                height_counter ++;
            }
            else if(!mapped_height && key=="building:levels") {
                building->setHeight(value.toDouble() * 3.0);
                level_counter ++;
            }
            else if (key == "mapped_height" and value.toDouble() > 0) {
                building->setHeight(value.toDouble());
                mapped_height = true;
            }
        }
    }

    buildings ++;

    return building;
}

std::map<std::string, Variant> OSMParser::readTags(DOMElement *_element)
{
    std::map<std::string, Variant> tags;

    for(auto node : _element->childNodes)
    {
        DOMElement *child = node->toElement();
        if(child->tagName=="tag")
        {
            std::string key = child->getAttribute("k").toString();
            Variant value = child->getAttribute("v");
            tags[key] = value;
        }
    }

    return tags;
}

}

