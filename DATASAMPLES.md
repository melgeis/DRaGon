# Using LIMoSim for data sample generation
The folder *LIMoSim* holds a scaled-down version of the **LIMoSim** framework, which only contains the classes needed for building the scenario's environment and the ones needed for exporting the images.
To use this framework *Qt C++* is required.

## Required Geographical Data
**LIMoSim** takes building information as **Open Street Map** data and elevation information as a numeric matrix. Both sources have to cover the same area.

### Open Street Map File
The building information is given as an *osm* file, which is then parsed by **LIMoSim**. To include building height information of other sources, the corresponding building must be additionally tagged in the *osm* file using the tag key *mapped_height*:
```xml
<tag k="mapped_height" v="23.94"/></way>
```

### Elevation Profile
The elevation profile is given as a *csv* file holding a matrix with the elevation values.
The file has to contain a header line in the following format:
```csv
ref_lat, 51.475, ref_lon, 7.373, cell_size_m, 10
```
Where *ref_lat* and *ref_lon* specify the lower left corner in **WGS84** of the rectangular area covered by the elevation profile matrix.
*cell_size_m* specifies the distance between two neighboring matrix elements in meters.


## Generating dataset samples
The main function holds three different functions that can be used for the dataset generation.

### generateDataset
This function is used, when no REM is to be generated.
Therefore, the function requires a path to a folder, where a *cells.csv* holding the transmitter information and the corresponding receiver informations are.
The folder must contain one *csv* file per transmitter.
The *cells.csv* file must be in the following format, where each row specifies one transmitter:
```csv
id, lat, lon, bandwidth, frequency, building_height, elevation, offset
```
- *id* is the transmitter's identification number
- *lat* and *lon* specify the transmitter's position in **WGS84**
- *building_height* specifies the height of the transmitter
- *elevation* specifies the elevation at the transmitter's position
- *offset* aggregates antenna gains and coupling losses

For each *id* the folder must contain a *csv* file of the format:
```csv
id, lat, lon, rsrp_dbm
```
- *id* is the samples id, which is also the name of the generated data sample later
- *lat* and *lon* specify the receiver's position in **WGS84**
- *rsrp_dbm* specifies the measured **RSRP** in dBm

```c++
void generateDataset(std::string _osmPath, std::string _elevationPath, std::string _pathToFolder, std::string _targetPath, std::vector<int> _sizes, double _UE_height = 1.5, double _eNB_height = 0, bool _sv = true, bool _tv = true, bool _only_eps = false, bool _terrain = true, bool _buildings = true)
```
- *_osmPath* specifies the path to the *osm* file
- *_elevationPath* specifies the path to the elevation matrix
- *_pathToFolder* specifies the folder, where the data is
- *_targetPath* specifies the folder, where the generated data should be saved
- *_sizes* holds the ranges in meter, for which the image samples should be generated (-1 means the full range there)
- *_UE_height* specifies the receiver height above the ground
- *_eNB_height* specifies the transmitter height, which is added to the height specified in the *cells.csv*
- *_sv* indicates whether the side view samples should be generated or not
- *_tv* indicates whether the top view samples should be generated or not
- *_only_eps* indicates whether the features should be saved or not
- *_terrain* indicates whether the elevation profile should be used or not
- *_buildings* indicates whether the building information should be used or not

(Here one *json* file holding the features is generated per transmitter receiver pair.)

### generateDatasetREM
This function is used to generate a REM based on a specified bounding box.
The function requires a path to a folder, where a *cells.csv* lies holding the transmitter information. (For each specified transmitter one REM is to be generated.)
```c++
void generateDatasetREM(std::string _osmPath, std::string _elevationPath, std::string _pathToFolder, std::string _targetPath, std::vector<int> _sizes, double cell_size_m, double minlat, double minlon, double maxlat, double maxlon, double _UE_height = 1.5, double _eNB_height = 0, bool _sv = true, bool _tv = true, bool _only_eps = false, bool _terrain = true, bool _buildings = true)
```
- *cell_size_m* specifies the REM's cell size in meters
- *minlat*, *minlon*, *maxlat*, and *maxlon* specify the REM's bounding box in **WGS84**

(Here one *csv* file holding the features is generated.)

### generateDatasetFromListREM
This function is used to generate a REM based on a specified list of positions.
The function requires a path to a folder, where a *cells.csv* lies holding the transmitter information. (For each specified transmitter one REM is to be generated.)
Additionally the path to the folder holding the *rem.csv* including the list of positions is required:
```csv
lat, lon
```
(Each row specifies a latititude longitude pair.)

```c++
void generateDatasetFromListREM(std::string _osmPath, std::string _elevationPath, std::string _pathToCell, std::string _pathToPos, std::vector<int> _sizes, double _UE_height = 1.5, double _eNB_height = 0, bool _sv = true, bool _tv = true, bool _only_eps = false, bool _terrain = true, bool _buildings = true)

```
- *_pathToCell* specifies the path to the folder where the *cells.csv* lies
- *_pathToPos* specifies the paht to the folder where the *rem.csv* lies and is additionally the target folder where the generated samples will be saved

(Here one *csv* file holding the features is generated.)
