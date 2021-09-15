#include "sectionalview_eps.h"
#include "ui/export/epsdocument.h"
#include "ui/export/boundingboxexporter.h"
#include "LIMoSim/world/heightprofile.h"
#include "LIMoSim/world/world.h"
#include "LIMoSim/settings/osm/wgs84.h"
#include "math.h"
#include <string>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>


namespace LIMoSim
{

SectionalView_eps::SectionalView_eps()
{

}


void SectionalView_eps::generateREMdata(std::string folder, std::string targetFolder, double cell_size_m, double height_UE, double height_eNB, std::vector<int> size, double minlat, double minlon, double maxlat, double maxlon, bool only_eps, bool sv, bool tv, bool corPos, bool terrain, bool buildings)
{
    // calculate scenario dimension
    Vector3d dist = WGS84::computeOffset(Vector3d(maxlon, maxlat), Vector3d(minlon, minlat));

    // get elevation profile
    HeightProfile* hp = World::getInstance()->getHeightProfile();

    // get row and column count
    int num_cols = floor(dist.x / cell_size_m);
    int num_rows = floor(dist.y / cell_size_m);

    // update dist vector to grid bounds
    dist.x = num_cols * cell_size_m;
    dist.y = num_rows * cell_size_m;

    // calculate lat and lon increments
    double lat_inc = (maxlat - minlat) / num_rows;
    double lon_inc = (maxlon - minlon) / num_cols;

    // iterate through cells
    QDir directory(QString::fromStdString(folder));
    std::string filename = "cells.csv";
    if (!corPos)
        filename = "cells_noCor.csv";
    QFile cellFile(QString::fromStdString(folder + filename));

    if (!cellFile.exists())
        return;

    if (!cellFile.open(QIODevice::ReadOnly)) {
        qDebug() << cellFile.errorString();
        return;
    }

    // read cells file line by line
    QString cellHeader = cellFile.readLine();

    while (!cellFile.atEnd()) {
        std::vector<std::vector<std::string>> features;
        QByteArray cellLine = cellFile.readLine();
        QList<QByteArray> cellInfo = cellLine.split(',');

        // read cell values
        QString tmp = cellInfo.first();
        int cellId = static_cast<int>(tmp.toDouble());
        double cellLat = cellInfo.at(1).toDouble();
        double cellLon = cellInfo.at(2).toDouble();
        QString bandwidth = cellInfo.at(3);
        QString freq = cellInfo.at(4);
        QString offset = cellInfo.at(7);
        double buildingHeight = cellInfo.at(5).toDouble();
        double cellElevation = cellInfo.at(6).toDouble();

        // check if target folders exist
        if (!QDir(QString::fromStdString(targetFolder) + QString::number(cellId)).exists())
            QDir().mkdir(QString::fromStdString(targetFolder) + QString::number(cellId));
        if (!QDir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered").exists())
            QDir().mkdir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered");
        if (sv && !QDir(QString::fromStdString(targetFolder) + QString::number(cellId) + "/engineered/sv").exists())
            QDir().mkdir(QString::fromStdString(targetFolder) + QString::number(cellId) + "/engineered/sv");
        if (tv && !QDir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered/tv").exists())
            QDir().mkdir(QString::fromStdString(targetFolder) + QString::number(cellId) + "/engineered/tv");
        if (!only_eps && !QDir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered/features").exists())
            QDir().mkdir(QString::fromStdString(targetFolder) + QString::number(cellId) + "/engineered/features");

        for (unsigned int i = 0; i < size.size(); i++) {
            if (size[i] == -1) {
                if (sv && !QDir(QString::fromStdString(targetFolder) + QString::number(cellId) + "/engineered/sv/full").exists())
                    QDir().mkdir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered/sv/full");
                if (tv && !QDir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered/tv/full").exists())
                    QDir().mkdir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered/tv/full");
            }
            else {
                if (sv && !QDir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered/sv/" + QString::number(size[i]) + "m").exists())
                    QDir().mkdir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered/sv/" + QString::number(size[i]) + "m");
                if (tv && !QDir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered/tv/" + QString::number(size[i]) + "m").exists())
                   QDir().mkdir(QString::fromStdString(targetFolder) + QString::number(cellId) +"/engineered/tv/" + QString::number(size[i]) + "m");
            }
        }

        // calculate transmitter position in 3D space
        Vector3d tx = WGS84::computeOffset(Vector3d(cellLon, cellLat), World::getInstance()->getReference());
        tx.z = cellElevation + buildingHeight + height_eNB;
        if (tx.x < 0.0 && tx.x > -0.01)
            tx.x = 0.0;
        if (tx.y < 0.0 && tx.y > -0.01)
            tx.y = 0.0;

        // range 300 m , cells = 25x25m -> 300/25 = 12 indices to ignore
        int border = 300 / cell_size_m;

        // iterate through REM cells
        for (int col = border; col < num_cols-border; col++) {
            for (int row = border; row < num_rows-border; row++) {
                std::vector<std::string> feature;

                // calculate receiver position in 3D space
                double x = col * cell_size_m + 0.5 * cell_size_m;
                double y = (num_rows - row - 1) * cell_size_m  + 0.5 * cell_size_m;
                Vector3d rx(x,y);
                rx.z = hp->getElevation(rx) + height_UE;

                std::string id = std::to_string(col) + "_" + std::to_string(row);

                // sv
                if (sv) {
                    if (only_eps)
                        epsExport(tx, rx, targetFolder + std::to_string(cellId) + "/engineered/", id, size, false, terrain, buildings);
                    else
                        epsExport(tx, rx, targetFolder + std::to_string(cellId) + "/engineered/", id, size, true, terrain, buildings);
                }

                // tv
                if (tv) {
                    for (unsigned int i = 0; i < size.size(); i++) {
                        int cur_size = size[i];
                        std::string _path = targetFolder + std::to_string(cellId) + "/engineered/tv/" + std::to_string(cur_size) + "m/" + id + ".eps";
                        if (cur_size == -1)
                            _path = targetFolder + std::to_string(cellId) + "/engineered/tv/full/" + id + ".eps";
                        BoundingBoxExporter box(cur_size);
                        box.snapshot(rx, tx, QString::fromStdString(_path));
                    }
                }

                if (!only_eps) {
                    // features
                    double lat = minlat + (num_rows - row - 1) * lat_inc + 0.5 * lat_inc;
                    double lon = minlon + col * lon_inc + 0.5 * lon_inc;
                    Vector3d dist = WGS84::computeOffset(Vector3d(lon, lat), Vector3d(cellLon, cellLat));

                    feature.push_back("0"); // RSRP
                    feature.push_back(std::to_string(rx.z)); // altitude
                    feature.push_back(bandwidth.toStdString()); // bandwidth
                    feature.push_back(std::to_string(tx.z)); // cell altitude
                    feature.push_back(std::to_string(buildingHeight + height_eNB)); // cell height
                    feature.push_back(std::to_string(cellLat)); // cell lat
                    feature.push_back(std::to_string(cellLon)); // cell lon
                    feature.push_back(std::to_string(m_distElem.txrxDist)); // dist 3D
                    feature.push_back(freq.toStdString()); // freq
                    feature.push_back(std::to_string(height_UE)); // height
                    feature.push_back(std::to_string(m_distElem.indoorDist)); // indoor Dist
                    feature.push_back(std::to_string(m_intersectionNumbers.first)); // indoor intersections
                    feature.push_back(std::to_string(lat)); // lat
                    feature.push_back(std::to_string(dist.x)); // lat dist
                    feature.push_back(std::to_string(lon)); // lon
                    feature.push_back(std::to_string(dist.y)); // lon dist
                    feature.push_back(offset.split("\r").first().toStdString()); // offset
                    feature.push_back(std::to_string(m_distElem.terrainDist)); // terrain dist
                    feature.push_back(std::to_string(m_intersectionNumbers.second)); // terrain intersections
                    feature.push_back(id); // sample id

                    features.push_back(feature);
                }
            }
        }

        if (!only_eps) {
            QFile file(QString::fromStdString(targetFolder) + cellId + "/features/feature.csv");

            if (file.open(QIODevice::ReadWrite)) {
                QTextStream stream(&file);
                stream<<"RSRP,altitude,bandwidth,cell_altitude,cell_height,cell_lat,cell_lon,dist3d,frequency,height,indoorDist,indoorIntersections,lat,lat_dist,lon,lon_dist,offset,terrainDist,terrainIntersections,id,\n";

                for (unsigned int i = 0; i < features.size(); i++) {
                    for (unsigned int j = 0; j < features[i].size(); j++)
                        stream <<QString::fromStdString(features[i][j])<<",";
                    stream<<"\n";
                }
            }
            file.close();
        }
    }
}


void SectionalView_eps::generateREMdataFromList(std::string path_cell, std::string path_pos, double height_UE, double height_eNB, std::vector<int> size, bool only_eps, bool sv, bool tv, bool corPos, bool terrain, bool buildings)
{
    // read positions
    QFile posFile(QString::fromStdString(path_pos) + "rem.csv");

    if (!posFile.exists())
        return;

    if (!posFile.open(QIODevice::ReadOnly)) {
        qDebug() << posFile.errorString();
        return;
    }

    // reade position header
    // QString posHeader = posFile.readLine();

    // read positions and save as list
    std::vector<QPair<double, double>> positions;
    while (!posFile.atEnd()) {
        QByteArray cellLine = posFile.readLine();
        QList<QByteArray> posInfo = cellLine.split(',');
        positions.push_back(QPair<double, double>(posInfo.first().toDouble(), posInfo.last().toDouble()));
    }

    // get elevation profile
    HeightProfile* hp = World::getInstance()->getHeightProfile();

    // iterate through cells
    std::string filename = "cells.csv";
    if (!corPos)
        filename = "cells_noCor.csv";
    QFile cellFile(QString::fromStdString(path_cell + filename));

    if (!cellFile.exists())
        return;

    if (!cellFile.open(QIODevice::ReadOnly)) {
        qDebug() << cellFile.errorString();
        return;
    }

    // read cells file line by line
    // QString cellHeader = cellFile.readLine();

    while (!cellFile.atEnd()) {
        std::vector<std::vector<std::string>> features;
        QByteArray cellLine = cellFile.readLine();
        QList<QByteArray> cellInfo = cellLine.split(',');

        // read cell values
        QString tmp = cellInfo.first();
        int cellId = static_cast<int>(tmp.toDouble());
        double cellLat = cellInfo.at(1).toDouble();
        double cellLon = cellInfo.at(2).toDouble();
        QString bandwidth = cellInfo.at(3);
        QString freq = cellInfo.at(4);
        QString offset = cellInfo.at(7);
        double buildingHeight = cellInfo.at(5).toDouble();
        double cellElevation = cellInfo.at(6).toDouble();

        // check if path exists
        QString path = QString::fromStdString(path_cell) + "from_list/" + QString::number(cellId);
        if (!QDir(path).exists())
            QDir().mkdir(path);
        if (!QDir(path + "/engineered").exists())
            QDir().mkdir(path + "/engineered");
        if (sv && !QDir(path + "/engineered/sv").exists())
            QDir().mkdir(path + "/engineered/sv");
        if (tv && !QDir(path + "/engineered/tv").exists())
            QDir().mkdir(path + "/engineered/tv");
        if (!only_eps && !QDir(path + "/features").exists())
            QDir().mkdir(path + "/features");

        for (unsigned int i = 0; i < size.size(); i++) {
            if (size[i] == -1) {
                if (sv && !QDir(path + "/engineered/sv/full").exists())
                    QDir().mkdir(path + "/engineered/sv/full");
                if (tv && !QDir(path + "/engineered/tv/full").exists())
                    QDir().mkdir(path +"/engineered/tv/full");
            }
            else {
                if (sv && !QDir(path +"/engineered/sv/" + QString::number(size[i]) + "m").exists())
                    QDir().mkdir(path +"/engineered/sv/" + QString::number(size[i]) + "m");
                if (tv && !QDir(path +"/engineered/tv/" + QString::number(size[i]) + "m").exists())
                   QDir().mkdir(path +"/engineered/tv/" + QString::number(size[i]) + "m");
            }
        }

        // calculate transmitter position in 3D space
        Vector3d tx = WGS84::computeOffset(Vector3d(cellLon, cellLat), World::getInstance()->getReference());
        tx.z = cellElevation + buildingHeight + height_eNB;
        if (tx.x < 0.0 && tx.x > -0.01)
            tx.x = 0.0;
        if (tx.y < 0.0 && tx.y > -0.01)
            tx.y = 0.0;

        for (unsigned int i = 0; i < positions.size(); i++) {
            double lat = positions.at(i).first;
            double lon = positions.at(i).second;

            // calculate receiver position in 3D space
            Vector3d rx = WGS84::computeOffset(Vector3d(lon, lat), World::getInstance()->getReference());
            rx.z = hp->getElevation(rx) + height_UE;

            std::vector<std::string> feature;
            QString id = QString::number(i).rightJustified(4, '0');

            // sv
            if (sv) {
                if (only_eps)
                    epsExport(tx, rx, path.toStdString() + "/engineered/", id.toStdString(), size, false, terrain, buildings);
                else
                    epsExport(tx, rx, path.toStdString() + "/engineered/", id.toStdString(), size, true, terrain, buildings);
            }

            // tv
            if (tv) {
                for (unsigned int i = 0; i < size.size(); i++) {
                    int cur_size = size[i];
                    QString _path = path + "/engineered/tv/" + QString::number(cur_size) + "m/" + id + ".eps";
                    if (cur_size == -1)
                        _path = path + "/engineered/tv/full/" + id + ".eps";
                    BoundingBoxExporter box(cur_size);
                    box.snapshot(rx, tx, _path);
                }
            }


            if (!only_eps) {
                // calculate sectional view variables if not done before
                if (!sv)
                    epsExport(tx, rx, path.toStdString() + "/engineered/", id.toStdString(), size, true, terrain, buildings);

                // features
                Vector3d dist = WGS84::computeOffset(Vector3d(lon, lat), Vector3d(cellLon, cellLat));

                feature.push_back("0"); // RSRP
                feature.push_back(std::to_string(rx.z)); // altitude
                feature.push_back(bandwidth.toStdString()); // bandwidth
                feature.push_back(std::to_string(tx.z)); // cell altitude
                feature.push_back(std::to_string(buildingHeight + height_eNB)); // cell height
                feature.push_back(std::to_string(cellLat)); // cell lat
                feature.push_back(std::to_string(cellLon)); // cell lon
                feature.push_back(std::to_string(m_distElem.txrxDist)); // dist 3D
                feature.push_back(freq.toStdString()); // freq
                feature.push_back(std::to_string(height_UE)); // height
                feature.push_back(std::to_string(m_distElem.indoorDist)); // indoor Dist
                feature.push_back(std::to_string(m_intersectionNumbers.first)); // indoor intersections
                feature.push_back(std::to_string(lat)); // lat
                feature.push_back(std::to_string(dist.x)); // lat dist
                feature.push_back(std::to_string(lon)); // lon
                feature.push_back(std::to_string(dist.y)); // lon dist
                feature.push_back(offset.split("\r").first().toStdString()); // offset
                feature.push_back(std::to_string(m_distElem.terrainDist)); // terrain dist
                feature.push_back(std::to_string(m_intersectionNumbers.second)); // terrain intersections
                feature.push_back(id.toStdString()); // sample id

                features.push_back(feature);
            }
        }

        if (!only_eps) {
            QFile file(path + "/features/feature.csv");

            if (file.open(QIODevice::ReadWrite)) {
                QTextStream stream(&file);
                stream<<"RSRP,altitude,bandwidth,cell_altitude,cell_height,cell_lat,cell_lon,dist3d,frequency,height,indoorDist,indoorIntersections,lat,lat_dist,lon,lon_dist,offset,terrainDist,terrainIntersections,id,\n";

                for (unsigned int i = 0; i < features.size(); i++) {
                    for (unsigned int j = 0; j < features[i].size(); j++)
                        stream <<QString::fromStdString(features[i][j])<<",";
                    stream<<"\n";
                }
            }
            file.close();
        }
    }
}


void SectionalView_eps::generateDataset(std::string csvPath, std::string targetFolder, double height_UE, double height_eNB, std::vector<int> size, bool only_eps, bool sv, bool tv, bool corPos, bool terrain, bool buildings)
{
    // get list of all files
    QDir directory(QString::fromStdString(csvPath));
    QStringList files = directory.entryList(QStringList() << "*.csv", QDir::Files);

    std::string filename = "cells.csv";
    if (!corPos)
        filename = "cells_noCor.csv";

    QFile cellFile(QString::fromStdString(csvPath + filename));

    if (!cellFile.exists())
        return;

    if (!cellFile.open(QIODevice::ReadOnly)) {
        qDebug() << cellFile.errorString();
        return;
    }

    // read cells file line by line
    // QString cellHeader = cellFile.readLine();

    while (!cellFile.atEnd()) {
        QByteArray cellLine = cellFile.readLine();
        QList<QByteArray> cellInfo = cellLine.split(',');

        // read cell values
        QString cellId = cellInfo.first();
        double cellLat = cellInfo.at(1).toDouble();
        double cellLon = cellInfo.at(2).toDouble();
        QString bandwidth = cellInfo.at(3);
        QString freq = cellInfo.at(4);
        QString offset = cellInfo.at(7);
        double buildingHeight = cellInfo.at(5).toDouble();
        if (corPos == false && buildingHeight == -1)
            buildingHeight = 15;
        double cellElevation = cellInfo.at(6).toDouble();

        // check if target folders exist
        if (sv && !QDir(QString::fromStdString(targetFolder) +"/sv").exists())
            QDir().mkdir(QString::fromStdString(targetFolder) + "/sv");
        if (tv && !QDir(QString::fromStdString(targetFolder )+"/tv").exists())
            QDir().mkdir(QString::fromStdString(targetFolder) + "/tv");
        if (!only_eps && !QDir(QString::fromStdString(targetFolder) +"/features").exists())
            QDir().mkdir(QString::fromStdString(targetFolder) + "/features");

        for (unsigned int i = 0; i < size.size(); i++) {
            if (sv) {
                if (size[i] == -1) {
                    if (!QDir(QString::fromStdString(targetFolder) +"/sv/full").exists())
                        QDir().mkdir(QString::fromStdString(targetFolder) +"/sv/full");
                }
                else {
                    if (!QDir(QString::fromStdString(targetFolder +"/sv/" + std::to_string(size[i]) + "m")).exists())
                        QDir().mkdir(QString::fromStdString(targetFolder +"/sv/" + std::to_string(size[i]) + "m"));
                }
            }

            if (tv) {
                if (size[i] == -1) {
                    if (!QDir(QString::fromStdString(targetFolder) +"/tv/full").exists())
                        QDir().mkdir(QString::fromStdString(targetFolder) +"/tv/full");
                }
                else {
                    if (!QDir(QString::fromStdString(targetFolder +"/tv/" + std::to_string(size[i]) + "m")).exists())
                       QDir().mkdir(QString::fromStdString(targetFolder +"/tv/" + std::to_string(size[i]) + "m"));
                }
            }
        }

        // read corresponding measurements
        QFile file(QString::fromStdString(csvPath) + cellId + ".csv");

        // check, if file is valid
        if (!file.exists()) {
            qDebug()<<"file of cell with id "<<cellId<<" does not exist";
            continue;
        }
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << file.errorString();
            continue;
        }

        // skip header
        file.readLine();

        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            QList<QByteArray> splittedLine = line.split(',');

            // read measurement values of interest
            QString id = splittedLine.first();

            qDebug()<<id;

            double lat = splittedLine.at(1).toDouble();
            double lon = splittedLine.at(2).toDouble();
            QString rsrp = splittedLine.at(3);

            // eps export
            HeightProfile* hp = World::getInstance()->getHeightProfile();

            // calculate transmitter position in 3D space
            Vector3d tx = WGS84::computeOffset(Vector3d(cellLon, cellLat), World::getInstance()->getReference());
            tx.z = cellElevation + buildingHeight + height_eNB;

            if (tx.x < 0.0 && tx.x > -0.01)
                tx.x = 0.0;
            if (tx.y < 0.0 && tx.y > -0.01)
                tx.y = 0.0;

            // calculate receiver position in 3D space
            Vector3d rx = WGS84::computeOffset(Vector3d(lon, lat), World::getInstance()->getReference());
            rx.z = hp->getElevation(rx) + height_UE;

            // generate side view samples
            if (sv) {
                if (only_eps)
                    epsExport(tx, rx, targetFolder, id.toStdString(), size, false, terrain, buildings);
                else
                    epsExport(tx, rx, targetFolder, id.toStdString(), size, true, terrain, buildings);
            }

            // generate top view samples
            if (tv) {
                for (unsigned int i = 0; i < size.size(); i++) {
                    int cur_size = size[i];
                    QString path = QString::fromStdString(targetFolder + "tv/" + std::to_string(cur_size)) + "m/" + id + ".eps";
                    if (cur_size == -1)
                        path = QString::fromStdString(targetFolder + "tv/full/") + id + ".eps";
                    BoundingBoxExporter box(cur_size);
                    box.snapshot(rx, tx, path);
                }
            }

            if (!only_eps) {
                // calculate sectional view variables if not done before
                if (!sv)
                    epsExport(tx, rx, targetFolder, id.toStdString(), size, true, terrain, buildings);

                Vector3d dist = WGS84::computeOffset(Vector3d(lon, lat), Vector3d(cellLon, cellLat));

                // engineered features
                QJsonObject recordObject;
                recordObject.insert("RSRP", QJsonValue::fromVariant(rsrp.toDouble()));
                recordObject.insert("cell_lat", QJsonValue::fromVariant(cellLat));
                recordObject.insert("cell_lon", QJsonValue::fromVariant(cellLon));
                recordObject.insert("cell_altitude", QJsonValue::fromVariant(tx.z));
                recordObject.insert("cell_height", QJsonValue::fromVariant(buildingHeight + height_eNB));
                recordObject.insert("frequency", QJsonValue::fromVariant(freq.toDouble()));
                recordObject.insert("bandwidth", QJsonValue::fromVariant(bandwidth.toDouble()));
                recordObject.insert("offset", QJsonValue::fromVariant(offset.toDouble()));
                recordObject.insert("lat", QJsonValue::fromVariant(lat));
                recordObject.insert("lon", QJsonValue::fromVariant(lon));
                recordObject.insert("altitude", QJsonValue::fromVariant(rx.z));
                recordObject.insert("height", QJsonValue::fromVariant(height_UE));
                recordObject.insert("lat_dist", QJsonValue::fromVariant(dist.x));
                recordObject.insert("lon_dist", QJsonValue::fromVariant(dist.y));
                recordObject.insert("dist3d", QJsonValue::fromVariant(m_distElem.txrxDist));
                recordObject.insert("indoorDist", QJsonValue::fromVariant(m_distElem.indoorDist));
                recordObject.insert("terrainDist", QJsonValue::fromVariant(m_distElem.terrainDist));
                recordObject.insert("indoorIntersections", QJsonValue::fromVariant(m_intersectionNumbers.first));
                recordObject.insert("terrainIntersections", QJsonValue::fromVariant(m_intersectionNumbers.second));

                QJsonDocument doc(recordObject);
                QFile features(QString::fromStdString(targetFolder) + "features/" + id + ".json");
                features.open(QFile::WriteOnly);
                features.write(doc.toJson());
            }
        }
    }
}


void SectionalView_eps::epsExport(Vector3d _tx, Vector3d _rx, std::string targetFolder, std::string id, std::vector<int> size, bool is_info, bool terrain, bool buildings)
{
    // elevation 50 m below and 50 m above UE
    double min_elevation = _rx.z - 50;
    // double max_elevation = _rx.z + 50;

    // swapped rx and tx, in order that sectional view is of UEs view
    SectionalView sv_complete(_rx, _tx, terrain, buildings);
    sv_complete.run(is_info);

    // save values
    DistanceElement distElem = sv_complete.getDistances();
    m_distElem = distElem;

    m_intersectionNumbers = sv_complete.getIntersectionNumbers();

    for (unsigned int i = 0; i < size.size(); i++) {
        // copy original data
        SectionalView sv(_rx, _tx);
        sv = sv_complete;

        // get current size
        int cur_size = size[i];
        std::string path = targetFolder + "sv/" + std::to_string(cur_size) + "m/" + id + ".eps";
        if (cur_size == -1)
            path = targetFolder + "sv/full/" + id + ".eps";

        // check 2d distance
        if (cur_size != -1 && (_rx-_tx).norm2d() < cur_size) {
            // distance too short -> add range
            Vector3d dir = (_tx-_rx).normed();
            Vector3d goal = _rx + (dir * 500);
            while ((goal-_rx).norm2d() < 500)
                goal = goal + dir;

            if (goal.x < 0 || goal.y < 0) {
                std::cout<<"distance "<<std::to_string(cur_size)<<"m is outside of area"<<std::endl;
                return;
            }

            // update sectional view with new "tx" position
            SectionalView sv_tmp(_rx, goal);
            sv = sv_tmp;
            sv.run(is_info);
        }

        // get vectors
        std::pair<std::vector<double>, std::vector<double>> tmp = sv.getBuildingVectors();
        std::vector<double> b_dist = tmp.first;
        std::vector<double> b_altitude = tmp.second;

        tmp = sv.getElevationVectors();
        std::vector<double> e_dist = tmp.first;
        std::vector<double> elevation = tmp.second;

        // new eps
        EpsDocument eps;
        eps.init(250, 250);
        eps.startPath();

        // set x_scale variable
        if (cur_size == -1)
            cur_size = (_rx-_tx).norm2d();
        double x_scale = (double)cur_size / (double)250;

        // set y_scale variable
        double y_scale = (double)100 / (double)250;

        // draw buildings
        if (!b_dist.empty()) {
            unsigned int j = 0;

            // get first valid index
            while (j < b_dist.size() && b_altitude.at(j) <= sv.getMinimumElevation())
                j++;

            if (j < b_dist.size()) {
                // map value in range
                double y = b_altitude.at(j) - min_elevation < 0 ? 0 : b_altitude.at(j) - min_elevation;
                y /= y_scale;
                if (y > 250)
                    y = 250;
                double x = b_dist.at(j) / x_scale;

                // move to first point
                if (b_dist.at(j) < 0) {
                    eps.moveTo(QPointF(0,0));
                    eps.lineTo(QPointF(0, y));
                }
                else {
                    eps.moveTo(QPointF(x, 0));
                    eps.lineTo(QPointF(x, y));
                }

                bool inBuilding = true;
                j++;

                // iterate through other nodes until limit is reached
                while (j < b_dist.size() && b_dist.at(j) <= cur_size) {
                    if (inBuilding) {
                        y = b_altitude.at(j-1) - min_elevation < 0 ? 0 : b_altitude.at(j-1) - min_elevation;
                        y /= y_scale;
                        if (y > 250)
                            y = 250;
                        x = b_dist.at(j) / x_scale;
                        eps.lineTo(QPointF(x, y));
                        eps.lineTo(QPointF(x, 0));

                        eps.closePath();
                        eps.setColor(Qt::black);
                        eps.fill();

                        inBuilding = false;
                    }
                    else if (b_altitude.at(j) > sv.getMinimumElevation()){
                        y = b_altitude.at(j) - min_elevation < 0 ? 0 : b_altitude.at(j) - min_elevation;
                        y /= y_scale;
                        if (y > 250)
                            y = 250;
                        x = b_dist.at(j) / x_scale;
                        eps.moveTo(QPointF(x, 0));
                        eps.lineTo(QPointF(x, y));

                        inBuilding = true;
                    }

                    j++;
                }

                // check, if building is finished
                if (inBuilding) {
                    eps.lineTo(QPointF(250, y));
                    eps.lineTo(QPointF(250, 0));
                    eps.closePath();
                    eps.setColor(Qt::black);
                    eps.fill();
                }
            }
        }

        // draw elevation profile
        // map value in range
        double y = sv.getElevation(0) - min_elevation;
        y = y < 0 ? 0 : y;
        y /= y_scale;
        if (y > 250)
            y = 250;
        double x = 0;

        // move to first point
        eps.moveTo(QPointF(0,0));
        eps.lineTo(QPointF(x, y));

        unsigned int j = 1;
        while (j < e_dist.size() && e_dist.at(j) <= cur_size) {
            y = elevation.at(j) - min_elevation < 0 ? 0 : elevation.at(j) - min_elevation;
            y /= y_scale;
            if (y > 250)
                y = 250;
            x = e_dist.at(j)/x_scale;
            eps.lineTo(QPointF(x, y));
            j++;
        }

        y = sv.getElevation(cur_size) - min_elevation;
        y = y < 0 ? 0 : y;
        y /= y_scale;
        if (y > 250)
            y = 250;
        eps.lineTo(QPointF(250, y));
        eps.lineTo(QPointF(250, 0));

        eps.closePath();
        eps.setColor(Qt::gray);
        eps.fill();

        // save file
        eps.save(QString::fromStdString(path));
    }
}

}
