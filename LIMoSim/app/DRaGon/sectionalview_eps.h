#ifndef SECTIONALVIEW_EPS_H
#define SECTIONALVIEW_EPS_H

#include "LIMoSim/world/vector3d.h"
#include "sectionalview.h"

namespace LIMoSim
{
class SectionalView_eps
{
private:
    DistanceElement m_distElem;
    std::pair<int, int> m_intersectionNumbers;

public:
    SectionalView_eps();

    void generateREMdata(std::string folder, std::string targetFolder, double cell_size_m, double height_UE, double height_eNB, std::vector<int> size, double minlat, double minlon, double maxlat, double maxlon,  bool only_eps=false, bool sv=true, bool tv=true, bool corPos=true, bool terrain=true, bool buildings=true);
    void generateREMdataFromList(std::string path_cell, std::string path_pos, double height_UE, double height_eNB, std::vector<int> size, bool only_eps=false, bool sv=true, bool tv=true, bool corPos=true, bool terrain=true, bool buildings=true);

    void generateDataset(std::string csvPath, std::string targetPath, double height_UE, double height_eNB, std::vector<int> size, bool only_eps=false, bool sv=true, bool tv=true, bool corPos=true, bool terrain=true, bool buildings=true);
    void epsExport(Vector3d _tx, Vector3d _rx, std::string targetPath, std::string id, std::vector<int> size, bool is_info, bool terrain=true, bool buildings=true);
};
}
#endif // SECTIONALVIEW_EPS_H
