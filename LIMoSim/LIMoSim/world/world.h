#ifndef LIMOSIM_WORLD_H
#define LIMOSIM_WORLD_H

#include "node.h"
#include "building.h"
#include "bounds.h"
#include "heightprofile.h"
#include <map>

namespace LIMoSim
{

class World
{
public:
    World();
    ~World();
    static World* getInstance();

    // initialization
    void loadMap(const std::string &_path);
    void filterNodes();
    void updateBox(const Vector3d &_point);
    void adjustTerrainProfile();

    // creation
    Node* createNode(const std::string &_id, const Vector3d &_position);
    Building* createBuilding(const std::string &_id);

    // destruction
    void clearNodes();

    // accessors
    void setReference(const Vector3d &_origin);
    Vector3d getReference();
    Bounds getBounds();
    Node* getNodeById(const std::string &_id);
    const std::map<std::string, Node*>& getNodes();
    const std::map<std::string, Building*>& getBuildings();
    HeightProfile* getHeightProfile();

private:
    std::map<std::string, Node*> m_nodes;
    std::map<std::string, Building*> m_buildings;
    HeightProfile m_heightProfile;

    Vector3d m_reference;
    Bounds m_bounds;

};

}

#endif // LIMOSIM_WORLD_H
