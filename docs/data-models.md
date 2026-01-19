# SkyPlexC2 Data Models

> Generated: 2026-01-19

## Overview

SkyPlexC2 uses SQLite for local persistence with a migration-based schema management system. The database stores missions, interests (waypoints), swarms, and user preferences.

## Database Location

```
SkyPlexC2/SkyPlexUserData/missions.db   (runtime)
```

## Schema

### Version Table

Tracks database schema version for migrations.

```sql
CREATE TABLE tbl_version (
    id_pk INTEGER PRIMARY KEY,
    version INTEGER NOT NULL,
    migrated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Interests Table

Stores all interest types: POIs, AOIs, GeoFences, and Takeoff Points.

```sql
CREATE TABLE interests (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    type INTEGER NOT NULL,
    point_csv TEXT NOT NULL,
    group_id INTEGER,
    params_json TEXT,
    FOREIGN KEY (group_id) REFERENCES missions(id)
);
```

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER | Primary key |
| `name` | TEXT | Display name |
| `type` | INTEGER | Interest type enum |
| `point_csv` | TEXT | Coordinates (see format below) |
| `group_id` | INTEGER | Parent mission ID |
| `params_json` | TEXT | Additional parameters |

**Interest Types (enum):**

| Value | Type | Description |
|-------|------|-------------|
| 0 | POI | Point of Interest |
| 1 | AOI | Area of Interest (polygon) |
| 2 | GeoFence | No-fly zone (polygon) |
| 3 | TakeoffPoint | Launch location |

**Point CSV Format:**
- Single point: `longitude,latitude,altitude`
- Polygon: `lon1,lat1,alt1;lon2,lat2,alt2;lon3,lat3,alt3`

### Missions Table

Stores mission definitions and interest ordering.

```sql
CREATE TABLE missions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    interest_order TEXT
);
```

| Column | Type | Description |
|--------|------|-------------|
| `id` | INTEGER | Primary key |
| `name` | TEXT | Mission name |
| `interest_order` | TEXT | Comma-separated interest IDs |

### Swarms Table

Stores drone group definitions.

```sql
CREATE TABLE swarms (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL
);
```

## C++ Data Structures

### Interest Structures

```cpp
// Point of Interest data
struct FPOIDataStruct {
    int32 ID;
    FString Name;
    FVector Location;      // Lon, Lat, Alt
    int32 MissionID;
    FString ParamsJson;
};

// Area of Interest data
struct FAOIDataStruct {
    int32 ID;
    FString Name;
    TArray<FVector> Points;  // Polygon vertices
    int32 MissionID;
    FString ParamsJson;
};

// GeoFence data
struct FGeoFenceDataStruct {
    int32 ID;
    FString Name;
    TArray<FVector> Points;  // Polygon vertices
    int32 MissionID;
    FString ParamsJson;
};

// Takeoff Point data
struct FTakeoffPointDataStruct {
    int32 ID;
    FString Name;
    FVector Location;      // Lon, Lat, Alt
    int32 MissionID;
    FString ParamsJson;
};
```

### Mission Structures

```cpp
// Mission metadata
struct FSPMissionStruct {
    int32 ID;
    FString Name;
    TArray<int32> InterestOrder;
};

// Interest container (per mission)
struct FSPInterestStruct {
    TMap<int32, UInterest*> Interests;  // ID -> Interest object
};

// Mission progress tracking
struct FSPMissionProgress {
    int32 MissionID;
    int32 CurrentInterestIndex;
    float CompletionPercent;
};
```

## Data Access Layer

### SQLiteUtility

Low-level database operations:

```cpp
class SQLiteUtility {
    static sqlite3* OpenDatabase(USPLogger* log, const FString& dbPath);
    static int GetDatabaseVersion(USPLogger* log, sqlite3* connection);
    static int MigrateDatabase(USPLogger* log, sqlite3* connection, const MigrationMap migrations);
    static int ExecuteSQL(USPLogger* log, sqlite3* connection, const std::string& sqlQuery);
    static int PrepareSQLStatement(USPLogger* log, sqlite3* connection, const char* query, sqlite3_stmt** stmt);
};
```

### MissionAssetsManager

High-level CRUD operations:

```cpp
class UMissionAssetsManager : public USPManager {
    // Interests
    void InsertPOI(FString name, FVector location, int32 groupID, FString& params, int32& outID);
    void InsertAOI(FString name, TArray<FVector> points, int32 groupID, FString& params, int32& outID);
    void InsertGeoFence(FString name, TArray<FVector> points, int32 groupID, FString& params, int32& outID);
    void InsertTakeoffPoint(FString name, FVector location, int32 missionID, FString& params, int32& outID);
    void DeleteInterest(int32 id);
    void RenameInterest(int32 id, FString newName);
    void ReassignInterest(int32 id, int32 newGroupID);
    void UpdateInterestParams(int32 id, FString& params);

    // Missions
    void InsertMission(FString name, int32& outID);
    void RenameMission(int32 id, FString newName);
    void UpdateMissionOrder(int32 id, TArray<int32>& interestOrder);
    void DeleteMission(int32 id);

    // Swarms
    void InsertSwarm(FString name, int32& outID);
    void RenameSwarm(int32 id, FString newName);
    void DeleteSwarm(int32 id);

    // Loading
    void LoadMissions(TMap<int32, FSPMissionStruct>& outMissions);
    void LoadInterests(TArray<FPOIDataStruct>&, TArray<FAOIDataStruct>&,
                       TArray<FGeoFenceDataStruct>&, TArray<FTakeoffPointDataStruct>&);
    void LoadSwarms(TMap<int32, FString>& outSwarms);
};
```

## GeoJSON Support

### Import/Export

`UGeoJsonManager` handles GeoJSON file operations:

```cpp
class UGeoJsonManager : public USPManager {
    void ImportGeoJson(const FString& filePath, TArray<FPOIDataStruct>& pois, TArray<FAOIDataStruct>& aois);
    void ExportMissionToGeoJson(int32 missionID, const FString& filePath);
};
```

### GeoJSON Format

```json
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "geometry": {
        "type": "Point",
        "coordinates": [-122.4194, 37.7749, 100]
      },
      "properties": {
        "name": "Waypoint 1",
        "type": "POI"
      }
    },
    {
      "type": "Feature",
      "geometry": {
        "type": "Polygon",
        "coordinates": [[
          [-122.42, 37.78, 0],
          [-122.41, 37.78, 0],
          [-122.41, 37.77, 0],
          [-122.42, 37.77, 0],
          [-122.42, 37.78, 0]
        ]]
      },
      "properties": {
        "name": "Survey Area",
        "type": "AOI"
      }
    }
  ]
}
```

## Migration System

### Migration Map

```cpp
using MigrationMap = std::map<int, std::vector<std::string>>;

// Example migrations
MigrationMap migrations = {
    {1, {
        "CREATE TABLE interests (...)",
        "CREATE TABLE missions (...)"
    }},
    {2, {
        "ALTER TABLE interests ADD COLUMN params_json TEXT",
        "CREATE TABLE swarms (...)"
    }}
};
```

### Migration Execution

```cpp
// On database open
int currentVersion = SQLiteUtility::GetDatabaseVersion(log, connection);
SQLiteUtility::MigrateDatabase(log, connection, migrations);
// Applies all migrations > currentVersion
```

## Coordinate System

| Context | Format | Example |
|---------|--------|---------|
| Database | Lon, Lat, Alt (meters) | `-122.4194, 37.7749, 100` |
| Unreal | X, Y, Z (centimeters) | Transformed via Cesium |
| Display | Lat, Lon, Alt | `37.7749°N, 122.4194°W, 100m` |

### Coordinate Transformation

```cpp
// Database → Unreal (via Cesium Georeference)
ACesiumGeoreference* Geo = ACesiumGeoreference::GetDefaultGeoreference(World);
FVector UnrealPos = Geo->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(Lon, Lat, Alt));

// Unreal → Database
FVector LonLatAlt = Geo->TransformUnrealPositionToLongitudeLatitudeHeight(UnrealPos);
```
