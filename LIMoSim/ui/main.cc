#include "LIMoSim/settings/filehandler.h"
#include "LIMoSim/settings/osm/wgs84.h"
#include "LIMoSim/world/world.h"
#include "LIMoSim/world/plane3d.h"
#include "LIMoSim/world/heightprofile.h"

#include <stdlib.h>

#include "ui/export/epsdocument.h"
#include "app/DRaGon/sectionalview.h"
#include "app/DRaGon/sectionalview_eps.h"


using namespace LIMoSim;

void initializeEnvironment(const std::string &_map)
{
    World *world = World::getInstance();
    world->loadMap(_map);
    world->filterNodes();

    std::cout << "initializeEnvironment" << std::endl;
}


void load3dModel(std::string _osmPath, std::string _elevationPath)
{
    initializeEnvironment(_osmPath);
    HeightProfile *hp = World::getInstance()->getHeightProfile();
    hp->loadFromFile(_elevationPath);
    World::getInstance()->adjustTerrainProfile();
}


void generateDataset(std::string _osmPath, std::string _elevationPath, std::string _pathToFolder, std::string _targetPath, std::vector<int> _sizes, double _UE_height = 1.5, double _eNB_height = 0, bool _sv = true, bool _tv = true, bool _only_eps = false, bool _terrain = true, bool _buildings = true)
{
    load3dModel(_osmPath, _elevationPath);

    std::cout<<"start dataset generation"<<std::endl;

    SectionalView_eps sv_eps;
    sv_eps.generateDataset(_pathToFolder, _targetPath, _UE_height, _eNB_height, _sizes, _only_eps, _sv, _tv, _terrain, _buildings);
}

void generateDatasetREM(std::string _osmPath, std::string _elevationPath, std::string _pathToFolder, std::string _targetPath, std::vector<int> _sizes, double cell_size_m, double minlat, double minlon, double maxlat, double maxlon, double _UE_height = 1.5, double _eNB_height = 0, bool _sv = true, bool _tv = true, bool _only_eps = false, bool _terrain = true, bool _buildings = true)
{
    load3dModel(_osmPath, _elevationPath);

    std::cout<<"start dataset generation (REM)"<<std::endl;

    SectionalView_eps sv_eps;
    sv_eps.generateREMdata(_pathToFolder, _targetPath, cell_size_m, _UE_height, _eNB_height, _sizes, minlat, minlon, maxlat, maxlon,_only_eps, _sv, _tv, _terrain, _buildings);
}

void generateDatasetFromListREM(std::string _osmPath, std::string _elevationPath, std::string _pathToCell, std::string _pathToPos, std::vector<int> _sizes, double _UE_height = 1.5, double _eNB_height = 0, bool _sv = true, bool _tv = true, bool _only_eps = false, bool _terrain = true, bool _buildings = true)
{
    load3dModel(_osmPath, _elevationPath);

    std::cout<<"start dataset generation (REM from list)"<<std::endl;

    SectionalView_eps sv_eps;
    sv_eps.generateREMdataFromList(_pathToCell, _pathToPos, _UE_height, _eNB_height, _sizes, _only_eps, _sv, _tv, _terrain, _buildings);
}


int main(int argc, char *argv[])
{
    std::cout << std::endl;
    srand (1);

    std::string osm_path = "Z:/DRaGon/data/osm_files/urban_height.osm";
    std::string elevation_path = "Z:/DRaGon/data/hp_csv/hp_urban_converted.csv";

    std::string targetPath = "Z:/DRaGon/data/dataset_engineered/dortmund/REM/";
    std::string path = "Z:/DRaGon/data/dataset/dortmund/urban/";

    double UE_height = 1.5;
    double eNB_height = 4;

    std::vector<int> sizes;
    sizes.push_back(300);

    // generateDataset(osm_path, elevation_path, path, targetPath, sizes, UE_height, eNB_height);

//    // bounds of urban scenario
//    double minlat = 51.5011610;
//    double minlon = 7.4391960;
//    double maxlat = 51.5321780;
//    double maxlon = 7.4911840;
//    generateDatasetREM(osm_path, elevation_path, path, targetPath, sizes, 10, minlat, minlon, maxlat, maxlon, UE_height, eNB_height);


    std::string pathToPos = "Z:/DRaGon/data/dataset_REM/Dortmund/test/";
    std::string pathToCell = "Z:/DRaGon/data/dataset_REM/Dortmund/test/o2/";
    generateDatasetFromListREM(osm_path, elevation_path, pathToCell, pathToPos, sizes, UE_height, eNB_height);

}

