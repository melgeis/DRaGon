#include "boundingboxexporter.h"
#include <QDebug>
#include <QDateTime>
#include <math.h>

namespace LIMoSim
{

BoundingBoxExporter::BoundingBoxExporter(int _size) :
    m_size(_size),
    m_drawVegetation(false),
    m_drawEntities(false)
{

}

void BoundingBoxExporter::snapshot(const Vector3d &_rx, const Vector3d &_tx, const QString &_file)
{
//    std::cout << "BoundingBoxExporter::snapshot\t" << _file.toStdString() << std::endl;
    // clear node list
    m_nodeList.clear();

    Vector3d dir = (_tx-_rx).normed();
    m_phi = dir.computePhi();

    // relative distance: RX/TX
    m_width = (_tx-_rx).norm() * (1 + 0.2);
    m_height = m_width;
    double inc = (_tx-_rx).norm() * (0.1); // horizontal offset
    double h = m_height / 2; // vertical offset

    if(m_size>-1)
    {
        // absolute distance: centered around the receiver
        m_width = m_size;
        m_height = m_size;
        inc = m_width / 2; // horizontal offset
        h = m_height / 2; // vertical offset
    }

    m_eps.init(m_width, m_height);

    // draw the box
    m_origin = _rx;
    m_box.leftTop = Vector3d(-inc, h);
    m_box.leftBottom = Vector3d(-inc, -h);
    m_box.rightTop = Vector3d(inc, h);
    m_box.rightBottom = Vector3d(inc, -h);

    // draw the buildings
    World *world = World::getInstance();
    std::map<std::string, Building*> buildings = world->getBuildings();
    for(std::map<std::string, Building*>::iterator it=buildings.begin(); it!=buildings.end(); it++) {
        // check bounds first -> accelerate search
        if(checkCandidate(it->second->getBounds())) {
            // add building to list
            int index = m_nodeList.size();
            if (m_nodeList.size() == 0)
                m_nodeList.push_back(BuildingNode(it->second->getId()));
            else {
                for (unsigned int i = 0; i < m_nodeList.size(); i++) {
                    if (it->second->getId() == m_nodeList.at(i).id) {
                        index = i;
                        break;
                    }
                    else if (i == m_nodeList.size() - 1)
                        m_nodeList.push_back(BuildingNode(it->second->getId()));
                }
            }

            m_nodeList.at(index).outer = exportWay(it->second->getNodes(), "black");

            for (unsigned int i = 0; i < it->second->getInnerNodes().size(); i++) {
                m_nodeList.at(index).inner_nodes = true;
                m_nodeList.at(index).inner.push_back(exportWay(it->second->getInnerNodes().at(i), "white"));
            }
        }
    }


    // draw buildings - eps file
    std::vector<int> ids_inner;
    std::vector<int> ids_outer;

    // save indices of buildings with inner polygons
    for (unsigned int i = 0; i < m_nodeList.size(); i++) {
        if (m_nodeList.at(i).inner_nodes)
            ids_inner.push_back(i);
        else
            ids_outer.push_back(i);
    }

    if (ids_inner.size() == 0) {
        // no buildings with inner polygons -> draw buildings classic
        for (unsigned int i = 0; i < m_nodeList.size(); i++) {
            if (m_nodeList.at(i).outer.size() > 0) {
                m_eps.startPath();
                m_eps.moveTo(m_nodeList.at(i).outer.at(0));
                for (unsigned int j = 1; j < m_nodeList.at(i).outer.size(); j++)
                    m_eps.lineTo(m_nodeList.at(i).outer.at(j));
                m_eps.closePath();
                m_eps.setColor("black");
                m_eps.fill();
            }
        }
    }
    else {
        // buildings with inner polygons -> reorder elements
        std::vector<BuildingNode> nodeList_sorted;
        std::vector<int> tmp;

        for (unsigned int i = 0; i < ids_inner.size(); i++) {
            // get BuildingNode to current index
            BuildingNode curBuilding = m_nodeList.at(ids_inner.at(i));

            // iterate through all inner paths of current building
            std::vector<std::string> ids;
            for (unsigned int k = 0; k < curBuilding.inner.size(); k++) {
                if (curBuilding.inner.at(k).size() > 0) {
                    // bounds of inner polygon
                    double min_x = curBuilding.inner.at(k).at(0).rx();
                    double max_x = min_x;
                    double min_y = curBuilding.inner.at(k).at(0).ry();
                    double max_y = min_y;
                    for (unsigned int j = 1; j < curBuilding.inner.at(k).size(); j++) {
                        min_x = std::min(min_x, curBuilding.inner.at(k).at(j).rx());
                        max_x = std::max(max_x, curBuilding.inner.at(k).at(j).rx());
                        min_y = std::min(min_y, curBuilding.inner.at(k).at(j).ry());
                        max_y = std::max(max_y, curBuilding.inner.at(k).at(j).ry());
                    }

                    // iterate through other buildings containing inner polygons
                    for (unsigned int j = 0; j < ids_inner.size(); j++) {
                        if (ids_inner.at(i) == ids_inner.at(j))
                            continue;

                        // check if any of the outer nodes is inside of inner polygon
                        for (unsigned int l = 0; l < m_nodeList.at(ids_inner.at(j)).outer.size(); l++) {
                            QPointF p = m_nodeList.at(ids_inner.at(j)).outer.at(l);
                            if (p.rx() >= min_x && p.rx() <= max_x && p.ry() >= min_y && p.ry() <= max_y) {
                                // inside is true -> save building id of the corresponding building
                                ids.push_back(m_nodeList.at(ids_inner.at(j)).id);
                                break;
                            }
                        }
                    }
                }
            }

            if (ids.size() > 0) {
                if (nodeList_sorted.size() == 0) {
                    nodeList_sorted.push_back(curBuilding);
                }
                else {
                    // add building node to sorted list first
                    bool found = false;
                    for (std::vector<BuildingNode>::iterator it = nodeList_sorted.begin(); it != nodeList_sorted.end(); it++) {
                        for (unsigned int j = 0; j < ids.size(); j++) {
                            if ((*it).id == ids.at(j)) {
                                nodeList_sorted.insert(it, curBuilding);
                                found = true;
                                break;
                            }
                        }
                        if (found == true)
                            break;
                        else if (it == nodeList_sorted.end()-1) {
                            nodeList_sorted.push_back(curBuilding);
                            break;
                        }
                    }
                }
            }
            else {
                // save building node id for later
                tmp.push_back(ids_inner.at(i));
            }
        }

        // add other building nodes to sorted node list based on priority
        for (unsigned int i = 0; i < tmp.size(); i++)
            nodeList_sorted.push_back(m_nodeList.at(tmp.at(i)));
        for (unsigned int i = 0; i < ids_outer.size(); i++)
            nodeList_sorted.push_back(m_nodeList.at(ids_outer.at(i)));

        for (unsigned int i = 0; i < nodeList_sorted.size(); i++) {
            // draw outer polygon - black
            if (nodeList_sorted.at(i).outer.size() > 0) {
                m_eps.startPath();
                m_eps.moveTo(nodeList_sorted.at(i).outer.at(0));
                for (unsigned int j = 1; j < nodeList_sorted.at(i).outer.size(); j++)
                    m_eps.lineTo(nodeList_sorted.at(i).outer.at(j));
                m_eps.closePath();
                m_eps.setColor("black");
                m_eps.fill();
            }
            // draw inner polygons - white
            if (nodeList_sorted.at(i).inner.size() > 0) {
                for (unsigned int j = 0; j < nodeList_sorted.at(i).inner.size(); j++) {
                    if (nodeList_sorted.at(i).inner.at(j).size() > 0) {
                        m_eps.startPath();
                        m_eps.moveTo(nodeList_sorted.at(i).inner.at(j).at(0));
                        for (unsigned int k = 1; k < nodeList_sorted.at(i).inner.at(j).size(); k++)
                            m_eps.lineTo(nodeList_sorted.at(i).inner.at(j).at(k));
                        m_eps.closePath();
                        m_eps.setColor("white");
                        m_eps.fill();
                    }
                }
            }
        }
    }

    // save file
    m_eps.save(_file);

}

bool BoundingBoxExporter::checkCandidate(Bounds bounds)
{
    Vector3d min = rot(bounds.getMin());
    Vector3d max = rot(bounds.getMax());

    if (isInside(min) || isInside(max) || isInside(Vector3d(min.x, max.y)) || isInside(Vector3d(max.x, min.y)))
        return true;

    if (min.x < m_box.leftBottom.x && max.x > m_box.rightBottom.x) {
        if ((min.y < m_box.leftTop.y && min.y > m_box.leftBottom.y) || (max.y < m_box.leftTop.y && max.y >m_box.leftBottom.y))
            return true;
        else if (min.y < m_box.leftBottom.y && max.y > m_box.leftTop.y)
            return true;
    }

    if (min.y < m_box.leftBottom.y && max.y > m_box.leftTop.y) {
        if ((min.x < m_box.leftBottom.x && min.x > m_box.rightBottom.x) || (max.x < m_box.leftBottom.x && max.x >m_box.rightBottom.x))
            return true;
        else if (min.x < m_box.leftBottom.x && max.x > m_box.rightBottom.x)
            return true;
    }

    return false;
}

std::vector<QPointF> BoundingBoxExporter::exportWay(const std::vector<Node*> &_nodes, const QString &_color)
{

    // find the first node, which is inside the bounding box
    int index = -1;
    std::vector<Node*> nodes = _nodes;
    for(unsigned int i=0; i<nodes.size(); i++)
    {
        if(isInside(rot(nodes[i]->getPosition())))
        {
            index = i;
            break;
        }
    }

    std::vector<QPointF> tmp;
    //
    if(index>-1)
    {
        // first position inside
        //m_eps.startPath();
        Vector3d lastPosition = rot(nodes[index]->getPosition());
        int lastIntersectionType = INTERSECTION_TYPE::NONE;
        //m_eps.moveTo(toPoint(lastPosition));
        tmp.push_back(toPoint(lastPosition));

        bool wasIn = true;
        for(unsigned int i=0; i<nodes.size(); i++)
        {
            int nodeIndex = (i + index + 1) % nodes.size();
            Vector3d p = rot(nodes[nodeIndex]->getPosition());

            bool isIn = isInside(p);
            if(wasIn && isIn) // regular
                //m_eps.lineTo(toPoint(p));
                tmp.push_back(toPoint(p));
            else if((wasIn && !isIn) || (!wasIn && isIn)) // in <-> out transition
            {
                Vector3d origin = lastPosition;
                Vector3d dir = p - lastPosition;

                // check intersections to all bounding box axes
                int intersectionType = INTERSECTION_TYPE::NONE;
                Vector3d intersection = checkIntersection(origin, dir, m_box.leftTop, m_box.rightTop);
                if(intersection.valid)
                    intersectionType = INTERSECTION_TYPE::TOP;
                else
                {
                    intersection = checkIntersection(origin, dir, m_box.leftBottom, m_box.rightBottom);
                    if(intersection.valid)
                        intersectionType = INTERSECTION_TYPE::BOTTOM;
                    else
                    {
                        intersection = checkIntersection(origin, dir, m_box.leftBottom, m_box.leftTop);
                        if(intersection.valid)
                            intersectionType = INTERSECTION_TYPE::LEFT;
                        else
                        {
                            intersection = checkIntersection(origin, dir, m_box.rightBottom, m_box.rightTop);
                            if(intersection.valid)
                                intersectionType = INTERSECTION_TYPE::RIGHT;
                        }
                    }
                }



                if(intersection.valid)
                {
                    if(isIn)
                    {
                        Vector3d intermediate;
                        if(checkCondition(intersectionType, lastIntersectionType, INTERSECTION_TYPE::TOP, INTERSECTION_TYPE::LEFT))
                            intermediate = m_box.leftTop;
                        else if(checkCondition(intersectionType, lastIntersectionType, INTERSECTION_TYPE::TOP, INTERSECTION_TYPE::RIGHT))
                            intermediate = m_box.rightTop;
                        else if(checkCondition(intersectionType, lastIntersectionType, INTERSECTION_TYPE::BOTTOM, INTERSECTION_TYPE::LEFT))
                            intermediate = m_box.leftBottom;
                        else if(checkCondition(intersectionType, lastIntersectionType, INTERSECTION_TYPE::BOTTOM, INTERSECTION_TYPE::RIGHT))
                            intermediate = m_box.rightBottom;
                        else
                            intermediate.valid = false;

                        if(intermediate.valid)
                            //m_eps.lineTo(toPoint(intermediate));
                            tmp.push_back(toPoint(intermediate));

                        //m_eps.lineTo(toPoint(intersection));
                        tmp.push_back(toPoint(intersection));
                        //m_eps.lineTo(toPoint(p));
                        tmp.push_back(toPoint(p));
                    }
                    else
                        //m_eps.lineTo(toPoint(intersection));
                        tmp.push_back(toPoint(intersection));
                }
                wasIn = isIn;
                lastIntersectionType = intersectionType;
            }

            lastPosition = p;
        }

        //m_eps.closePath();
        //m_eps.setColor(_color);
        //m_eps.fill();
    }
    return tmp;
}

Vector3d BoundingBoxExporter::checkIntersection(const Vector3d &_origin, const Vector3d &_dir, const Vector3d &_p0, const Vector3d &_p1)
{
    Vector3d intersection;
    intersection.valid = false;

    Vector3d v1 = _origin - _p0;
    Vector3d v2 = _p1 - _p0;
    Vector3d v3(-_dir.y, _dir.x);

    double d = v2.cross2d(v1);
    double t1 = d / (v2*v3);
    double t2 = (v1*v3) / (v2*v3);

    if(t1>=0 && t2<=1 && t2>=0)
    {
        Vector3d point = _origin + _dir * t1;

        // is the point between p0 and p1?
        Vector3d p0 = _origin;
        Vector3d p1 = _origin + _dir;
        double epsilon = fabs((p1-p0).norm()-((p0-point).norm()+(p1-point).norm()));
        if(epsilon< 0.001 )
        {
            intersection = point;
            intersection.valid = true;
        }

    }
    return intersection;
}

Vector3d BoundingBoxExporter::rot(const Vector3d &_p)
{
    Vector3d p = _p - m_origin;

    double phi = Vector3d::toRad(-m_phi);
    double x = cos(phi) * p.x - sin(phi) * p.y;
    double y = sin(phi) * p.x + cos(phi) * p.y;

    return Vector3d(x,y);
}

QPointF BoundingBoxExporter::toPoint(const Vector3d &_p)
{
    Vector3d p = _p - m_box.leftBottom;

    if(false) // scale
    {
        p = _p;
        p.x = map(p.x, m_box.leftBottom.x, m_box.rightBottom.x, 0, m_width);
        p.y = map(p.y, m_box.leftBottom.y, m_box.leftTop.y, 0, m_height);
    }

    return QPointF(p.x, p.y);
}

bool BoundingBoxExporter::isInside(const Vector3d &_p)
{
    bool x = (_p.x>=m_box.leftBottom.x) && (_p.x<=m_box.rightBottom.x);
    bool y = (_p.y>=m_box.leftBottom.y) && (_p.y<=m_box.leftTop.y);

    if(x && y)
        return true;
    return false;
}

double BoundingBoxExporter::map(double _x, double _inMin, double _inMax, double _outMin, double _outMax)
{
    return (_x - _inMin) * (_outMax - _outMin) / (_inMax - _inMin) + _outMin;
}

bool BoundingBoxExporter::checkCondition(int _var0, int _var1, int _value0, int _value1)
{
    return (_var0==_value0 && _var1==_value1) || (_var0==_value1 && _var1==_value0);
}

}
