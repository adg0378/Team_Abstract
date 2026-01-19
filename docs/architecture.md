# SkyPlexC2 Architecture Document

> Generated: 2026-01-19 | Version: 1.0 | Scan Level: Deep

## Executive Summary

**SkyPlexC2** is a specialized Command and Control (C2) visualization system for drone operations, built on Unreal Engine 5.5. It provides real-time 3D geospatial visualization, mission planning, and drone fleet management capabilities.

**Key Characteristics:**
- **Platform:** Unreal Engine 5.5 (C++ / Blueprints)
- **Architecture Pattern:** Manager-based state management with event-driven UI
- **Primary Use Case:** Drone mission planning and real-time visualization
- **External Integrations:** PX4/ROS2 simulation, CC Simulator, Cesium Ion, Google APIs

## Technology Stack

### Core Technologies

| Layer | Technology | Version | Purpose |
|-------|-----------|---------|---------|
| Engine | Unreal Engine | 5.5 | Rendering, physics, input |
| Language | C++ | C++17 | Core application logic |
| Scripting | Blueprints | UE5 | UI, prototyping, configuration |
| Database | SQLite | 3.x | Local persistence |
| Geospatial | Cesium for Unreal | Latest | 3D terrain tiles |
| Graphics | DirectX 11/12 | SM5/SM6 | Rendering API |

### External Dependencies

| Dependency | Protocol | Purpose |
|------------|----------|---------|
| CC Simulator | WebSocket | Drone simulation bridge |
| PX4 Autopilot | DDS (via ROS2) | Flight controller simulation |
| Gazebo | (PX4 bundled) | Physics simulation |
| Cesium Ion | HTTPS | 3D terrain tile streaming |
| Google APIs | HTTPS | Maps, geolocation services |

### Unreal Engine Modules

```cpp
PublicDependencyModuleNames: [
    "CesiumRuntime",      // Geospatial rendering
    "Core", "CoreUObject", "Engine",
    "HTTP",               // REST API calls
    "WebSockets",         // Real-time communication
    "Json", "JsonUtilities",
    "InputCore",
    "ProceduralMeshComponent"
]
```

## Architecture Overview

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                         SkyPlexC2 Application                        │
├─────────────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌────────────┐ │
│  │   Player    │  │    State    │  │   Objects   │  │    UI      │ │
│  │   System    │  │  Management │  │   System    │  │   (HUD)    │ │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘  └─────┬──────┘ │
│         │                │                │                │        │
│         └────────────────┼────────────────┼────────────────┘        │
│                          │                │                          │
│                    ┌─────┴─────┐    ┌─────┴─────┐                   │
│                    │ Persistence│    │  External │                   │
│                    │   Layer    │    │   Comms   │                   │
│                    └─────┬─────┘    └─────┬─────┘                   │
├──────────────────────────┼────────────────┼─────────────────────────┤
│                          │                │                          │
│  ┌───────────────────────┴────────────────┴───────────────────────┐ │
│  │                    Unreal Engine 5.5                            │ │
│  │  (Rendering, Physics, Input, Networking, Asset Management)      │ │
│  └─────────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌────────────┐ │
│  │   SQLite    │  │  WebSocket  │  │    HTTP     │  │  Cesium    │ │
│  │  (Local DB) │  │ (CC Sim)    │  │ (Google)    │  │   Ion      │ │
│  └─────────────┘  └─────────────┘  └─────────────┘  └────────────┘ │
└─────────────────────────────────────────────────────────────────────┘
                                    │
                    ┌───────────────┴───────────────┐
                    │     WSL2 Ubuntu 22.04         │
                    │  ┌─────────┐  ┌───────────┐  │
                    │  │   PX4   │  │   ROS2    │  │
                    │  │  SITL   │  │  Humble   │  │
                    │  └────┬────┘  └─────┬─────┘  │
                    │       └──────┬──────┘        │
                    │        ┌─────┴─────┐         │
                    │        │  Gazebo   │         │
                    │        └───────────┘         │
                    └──────────────────────────────┘
```

### Component Architecture

```
ASPGameState (Central Hub)
    │
    ├── USPStorageManager ──────────┐
    │       ├── UFlyToLocationManager    │
    │       ├── UMissionAssetsManager ◄──┤ SQLite
    │       └── UGeoJsonManager          │
    │                                    │
    ├── USPMissionManager ──────────────►│ Interests, Missions
    │       └── [Delegates: Added/Deleted/Updated]
    │
    ├── USPDroneManager ────────────────►│ Drone Fleet
    │       └── ACCSimDrone (WebSocket)
    │
    ├── USPObstacleManager ─────────────►│ ADSB, FAA DOF
    │
    ├── USPSimulationManager ───────────►│ Simulation State
    │
    ├── UPreferencesManager ────────────►│ User Settings
    │
    ├── USPWorldManager ────────────────►│ Cesium, Environment
    │
    └── USPAuthManager ─────────────────►│ Authentication
```

## State Management

### Manager Pattern

All state managers inherit from `USPManager` base class:

```cpp
UCLASS()
class USPManager : public UObject {
    virtual void Setup_Implementation(bool& outSuccess);
    virtual void Teardown_Implementation();
    virtual void PreTeardown();
    virtual void CullRelatedObjects(float MaxDistance, const FVector& Origin);
};
```

### Game State Initialization

```cpp
// ASPGameState::BeginPlay()
1. Spawn USPLogger
2. Spawn USPStorageManager → Initialize SQLite
3. Spawn USPDroneManager
4. Spawn USPMissionManager → Load missions from DB
5. Spawn USPObstacleManager
6. Spawn USPSimulationManager
7. Spawn UPreferencesManager
8. Spawn USPWorldManager → Initialize Cesium
9. Spawn USPAuthManager
10. Broadcast OnGameStateFullyInitialized
```

### Event System

Dynamic multicast delegates for decoupled communication:

| Delegate | Trigger | Consumers |
|----------|---------|-----------|
| `FInterestAddedDelegate` | Interest created | UI, Mission Lines |
| `FInterestDeletedDelegate` | Interest removed | UI, Mission Lines |
| `FInterestUpdatedDelegate` | Interest modified | UI, Interaction Box |
| `FMissionAddedDelegate` | Mission created | UI, Mission List |
| `FMissionDeletedDelegate` | Mission removed | UI, Mission List |
| `FMissionFinishedDelegate` | Mission completed | UI, Drone Manager |
| `FOnGameStateFullyInitialized` | All managers ready | UI, Player |

## Data Architecture

### SQLite Schema

```sql
-- Version tracking for migrations
CREATE TABLE tbl_version (
    id_pk INTEGER PRIMARY KEY,
    version INTEGER NOT NULL,
    migrated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Interests (POI, AOI, GeoFence, TakeoffPoint)
CREATE TABLE interests (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    type INTEGER NOT NULL,      -- 0=POI, 1=AOI, 2=GeoFence, 3=TakeoffPoint
    point_csv TEXT NOT NULL,    -- "lon,lat,alt" or "lon1,lat1,alt1;lon2,..."
    group_id INTEGER,           -- Mission ID
    params_json TEXT            -- Additional parameters
);

-- Missions
CREATE TABLE missions (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    interest_order TEXT         -- Comma-separated interest IDs
);

-- Swarms (Drone groups)
CREATE TABLE swarms (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL
);
```

### Interest Type Hierarchy

```
UInterest (Base)
    │
    ├── UInterestPoint (Point-based)
    │       ├── UPOI (Point of Interest)
    │       └── USPTakeoffPoint (Launch location)
    │
    └── UAreaOfInterest (Polygon-based)
            ├── [AOI] Area of Interest
            └── USPGeoFence (No-fly zone)
```

### Data Flow

```
User Action → SPPlayerController
                    │
                    ▼
            SPMissionManager
                    │
         ┌──────────┼──────────┐
         ▼          ▼          ▼
    Add Interest  Update    Delete
         │          │          │
         └──────────┼──────────┘
                    │
                    ▼
         MissionAssetsManager
                    │
                    ▼
              SQLite DB
                    │
                    ▼
           Broadcast Delegate
                    │
                    ▼
              UI Updates
```

## Object System

### Actor Hierarchy

```
AActor
    │
    ├── AInteractable ──────────────── Selectable base
    │       │
    │       ├── APlaceable ─────────── Placeable on terrain
    │       │       │
    │       │       └── APlaceablePoint ── Map markers
    │       │               ├── POI Pin
    │       │               ├── AOI Pin
    │       │               ├── GeoFence Pin
    │       │               └── Takeoff Pin
    │       │
    │       └── APolygon ───────────── Area polygons
    │               ├── AOI Polygon
    │               └── GeoFence Polygon
    │
    └── ABasicDrone ────────────────── Drone base
            │
            ├── ACCSimDrone ────────── CC Simulator drone
            └── ADemoSimDrone ──────── Demo/test drone
```

### Drone Communication

```cpp
// ACCSimDrone WebSocket Commands
void Arm();
void Disarm();
void Takeoff(float Lat, float Lon, float Altitude);
void Land();
void Return();
void Orbit(float Radius, float Velocity, float Lat, float Lon, float Alt);
void Reposition(float Velocity, float Lon, float Lat, float Alt);
void SetWaypoint(...);
void UploadMission(const FString& Mission);
```

## Player System

### Input Handling

Enhanced Input System with Input Mapping Context:

| Action | Binding | Purpose |
|--------|---------|---------|
| `IA_Movement` | WASD | Camera movement |
| `IA_CameraOrbit` | RMB + Drag | Orbit camera |
| `IA_CameraZoomIn/Out` | Scroll | Zoom |
| `IA_DoAction` | LMB | Select/place |
| `IA_Delete` | Delete | Remove selected |
| `IA_Deselect` | Escape | Clear selection |
| `IA_MultipleSelect` | Ctrl | Multi-select modifier |
| `IA_SwapToFirstPersonCamera` | 1 | First-person view |
| `IA_SwapToIsometricCamera` | 2 | Isometric view |
| `IA_SwapToTopdownCamera` | 3 | Top-down view |

### Camera System

```cpp
// Camera modes via SPCameraInterface
- First Person: Free-fly camera
- Isometric: Fixed angle overhead
- Top-down: Bird's eye view
- Follow Drone: Track selected drone

// CesiumSpringArmComponent
- Geospatial-aware camera positioning
- Terrain collision avoidance
- Smooth transitions between modes
```

## External Integrations

### CC Simulator (WebSocket)

```
SkyPlexC2                          CC Simulator
    │                                   │
    │──── Connect (ws://host:port) ────►│
    │                                   │
    │◄──── Status Updates ─────────────│
    │◄──── Position Updates ───────────│
    │                                   │
    │──── Command (Arm/Takeoff/etc) ──►│
    │                                   │
    │◄──── Command Response ───────────│
```

### Cesium Ion

- **Terrain:** 3D tiles streaming
- **Imagery:** Satellite/aerial imagery
- **Georeference:** WGS84 coordinate system
- **Height Sampling:** Terrain elevation queries

### Google APIs

- Maps API for additional geospatial services
- HTTP utility class for REST calls

## Deployment Architecture

### Build Configurations

| Config | Use Case | Features |
|--------|----------|----------|
| Development Editor | Local dev | Hot reload, debugging |
| Development | Testing | Debug symbols, logging |
| Shipping | Production | Optimized, no debug |

### CI/CD Pipeline

```
GitLab Runner (unreal tag)
         │
         ▼
    buildme.bat
         │
         ▼
    RunUAT.bat BuildCookRun
         │
         ├── Build (compile C++)
         ├── Cook (process assets)
         ├── Stage (prepare files)
         ├── Package (create executable)
         └── Archive (zip artifacts)
         │
         ▼
    build/ directory
         │
         ▼
    GitLab Artifacts
```

### Runtime Dependencies

| Component | Required | Notes |
|-----------|----------|-------|
| Windows 10/11 | Yes | Primary platform |
| DirectX 11+ | Yes | Rendering |
| Visual C++ Redistributable | Yes | Runtime libraries |
| WSL2 + Ubuntu | For simulation | PX4/ROS2 stack |
| Network access | For Cesium | Tile streaming |

## Security Considerations

### Authentication

- `USPAuthManager` handles user authentication
- Session-based state management

### Data Security

- Local SQLite database (no network exposure)
- WebSocket communication (consider TLS for production)
- Cesium Ion token management

### Input Validation

- Coordinate validation for geospatial data
- Mission parameter validation
- WebSocket message parsing

## Performance Considerations

### Rendering

- Ray tracing enabled (configurable)
- Lumen Global Illumination
- Virtual Shadow Maps
- LOD for distant objects

### Memory Management

- Culling system for distant objects (`CullRelatedObjects`)
- Cesium tile streaming with cache
- SQLite connection pooling

### Network

- WebSocket keep-alive for simulator connection
- HTTP request queuing for API calls
- Cesium tile request caching

## Extension Points

### Adding New Interest Types

1. Create class inheriting from `UInterest` or `UInterestPoint`
2. Add enum value to `DBInterestType`
3. Implement insert/load methods in `UMissionAssetsManager`
4. Create Blueprint/widget for visualization
5. Register with `USPMissionManager`

### Adding New Drone Types

1. Create class inheriting from `ABasicDrone`
2. Implement communication protocol (WebSocket, ROS2, etc.)
3. Register with `USPDroneManager`
4. Create Blueprint for visual representation

### Adding New Managers

1. Create class inheriting from `USPManager`
2. Add spawn configuration to `ASPGameState`
3. Implement `Setup_Implementation` and `Teardown_Implementation`
4. Access via `ASPGameState` reference

## Related Documentation

- [Source Tree Analysis](./source-tree-analysis.md)
- [Development Guide](./development-guide.md)
- [Setup Guide](./Setup.md)
