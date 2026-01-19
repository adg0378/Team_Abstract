# SkyPlexC2 Source Tree Analysis

> Generated: 2026-01-19 | Scan Level: Deep

## Repository Structure

```
skyplexc2/                              # Repository root
├── .gitlab-ci.yml                      # GitLab CI/CD pipeline configuration
├── buildme.bat                         # Windows build script (UE Automation Tool)
│
├── docs/                               # Project documentation
│   ├── Setup.md                        # WSL2/development environment setup guide
│   └── [generated docs]                # AI-generated documentation
│
├── _bmad/                              # BMAD workflow system (development tooling)
│   ├── bmm/                            # BMAD module definitions
│   └── core/                           # BMAD core tasks/workflows
│
└── SkyPlexC2/                          # Main Unreal Engine project
    ├── SkyPlexC2.uproject              # UE5.5 project file
    │
    ├── Source/                         # C++ source code
    │   ├── SkyPlexC2/                  # Main game module
    │   │   ├── SkyPlexC2.Build.cs      # Module build configuration
    │   │   ├── SkyPlexC2.h/.cpp        # Module entry point
    │   │   │
    │   │   ├── Public/                 # Public headers (API surface)
    │   │   │   ├── API/                # External API integrations
    │   │   │   │   ├── GoogleAPIUtility.h    # Google Maps/APIs
    │   │   │   │   └── HTTPRequestUtility.h  # HTTP client utilities
    │   │   │   │
    │   │   │   ├── Objects/            # Game object definitions
    │   │   │   │   ├── Drones/         # Drone actors
    │   │   │   │   │   ├── BasicDrone.h      # Base drone class
    │   │   │   │   │   ├── CCSimDrone.h      # CC Simulator drone (WebSocket)
    │   │   │   │   │   ├── DemoSimDrone.h    # Demo/test drone
    │   │   │   │   │   └── DroneTrack.h      # Drone trail visualization
    │   │   │   │   │
    │   │   │   │   ├── Geometry/       # Geometric primitives
    │   │   │   │   │   ├── InterestLines.h   # Mission path lines
    │   │   │   │   │   ├── PlaceablePoint.h  # Map marker base
    │   │   │   │   │   └── Polygon.h         # Area polygon (AOI/GeoFence)
    │   │   │   │   │
    │   │   │   │   ├── Interests/      # Mission interest types
    │   │   │   │   │   ├── Interest.h        # Base interest class
    │   │   │   │   │   ├── InterestPoint.h   # Point interest base
    │   │   │   │   │   ├── POI.h             # Point of Interest
    │   │   │   │   │   ├── AreaOfInterest.h  # Area of Interest (polygon)
    │   │   │   │   │   ├── SPGeoFence.h      # GeoFence (no-fly zone)
    │   │   │   │   │   ├── SPTakeoffPoint.h  # Takeoff location
    │   │   │   │   │   └── SPSimWorldOrigin.h # Simulation origin marker
    │   │   │   │   │
    │   │   │   │   ├── Obstacles/      # Obstacle tracking
    │   │   │   │   │   ├── ADSBObject.h      # ADS-B aircraft transponder
    │   │   │   │   │   └── FAADOFObstacle.h  # FAA Digital Obstacle File
    │   │   │   │   │
    │   │   │   │   ├── Interactable.h        # Base selectable actor
    │   │   │   │   ├── Placeable.h           # Base placeable actor
    │   │   │   │   ├── SimulationData.h      # Simulation data structures
    │   │   │   │   └── StateboundInteractable.h # State-linked interactable
    │   │   │   │
    │   │   │   ├── Player/             # Player/camera system
    │   │   │   │   ├── SPCameraControllerCharacter.h  # Camera pawn
    │   │   │   │   ├── SPPlayerController.h           # Input controller
    │   │   │   │   ├── SPMainHUD.h                    # Main HUD class
    │   │   │   │   ├── SPCameraInterface.h            # Camera mode interface
    │   │   │   │   └── CesiumSpringArmComponent.h     # Geospatial camera arm
    │   │   │   │
    │   │   │   ├── State/              # Application state management
    │   │   │   │   ├── SPGameState.h         # Central game state (manager hub)
    │   │   │   │   ├── SPManager.h           # Base manager class
    │   │   │   │   ├── PreferencesManager.h  # User preferences
    │   │   │   │   ├── SPAuthManager.h       # Authentication
    │   │   │   │   ├── SPWorldManager.h      # World/environment state
    │   │   │   │   │
    │   │   │   │   ├── Missions/       # Mission management
    │   │   │   │   │   ├── SPMissionManager.h   # Mission CRUD, events
    │   │   │   │   │   └── InterestsUtil.h      # Interest helper structs
    │   │   │   │   │
    │   │   │   │   ├── Obstacles/      # Obstacle management
    │   │   │   │   │   ├── SPObstacleManager.h  # Obstacle tracking
    │   │   │   │   │   └── ObstaclesUtil.h      # Obstacle utilities
    │   │   │   │   │
    │   │   │   │   └── Storage/        # Persistence layer
    │   │   │   │       ├── SPStorageManager.h      # Storage coordinator
    │   │   │   │       ├── SQLiteUtility.h         # SQLite helpers
    │   │   │   │       ├── MissionAssetsManager.h  # Mission DB operations
    │   │   │   │       ├── FlyToLocationManager.h  # Saved locations
    │   │   │   │       ├── GeoJsonManager.h        # GeoJSON import/export
    │   │   │   │       └── SPSimulationManager.h   # Simulation state
    │   │   │   │
    │   │   │   └── Util/               # Utilities
    │   │   │       ├── SPUtility.h           # General utilities
    │   │   │       ├── SPLogger.h            # Logging system
    │   │   │       ├── GeographicUtility.h   # Geo calculations
    │   │   │       ├── WebSocketUtility.h    # WebSocket helpers
    │   │   │       ├── CLIUtility.h          # CLI/process management
    │   │   │       └── ClusterUtility.h      # Point clustering
    │   │   │
    │   │   ├── Private/                # Implementation files
    │   │   │   ├── API/                # API implementations
    │   │   │   ├── Objects/            # Object implementations
    │   │   │   ├── Player/             # Player implementations
    │   │   │   ├── State/              # State implementations
    │   │   │   ├── Util/               # Utility implementations
    │   │   │   └── Test/               # Unit tests
    │   │   │       └── SpecExample.spec.cpp  # Spec-based tests
    │   │   │
    │   │   └── ThirdParty/             # Third-party libraries
    │   │       └── SQLite/             # SQLite integration
    │   │           ├── Include/        # sqlite3.h
    │   │           └── Win64/          # sqlite3.dll, sqlite3.lib
    │   │
    │   ├── SkyPlexC2.Target.cs         # Game build target
    │   └── SkyPlexC2Editor.Target.cs   # Editor build target
    │
    ├── Content/                        # Unreal assets (Blueprints, materials)
    │   ├── Basemap.umap                # Main level/map
    │   │
    │   ├── Core/                       # Core game assets
    │   │   ├── SP_GameModeBase         # Game mode blueprint
    │   │   ├── SP_GameState            # Game state blueprint
    │   │   ├── Player/                 # Player blueprints
    │   │   │   ├── SP_CameraControllerCharacter
    │   │   │   ├── SP_PlayerController
    │   │   │   └── Input/              # Enhanced Input mappings
    │   │   │       ├── IMC_Default     # Input mapping context
    │   │   │       └── IA_*            # Input actions
    │   │   └── State/Managers/         # Manager blueprints
    │   │       ├── BP_SPDroneManager
    │   │       ├── BP_SPMissionManager
    │   │       ├── BP_SPObstacleManager
    │   │       └── BP_SimulationManager
    │   │
    │   ├── Objects/                    # Game object assets
    │   │   ├── drone/                  # Drone blueprints
    │   │   │   ├── BP_BasicDrone
    │   │   │   └── BP_CCSimDrone
    │   │   ├── Geometry/               # Geometry blueprints
    │   │   │   ├── BP_Polygon
    │   │   │   └── BP_InterestLines
    │   │   ├── Interests/              # Interest blueprints
    │   │   │   ├── BasicMapPoint (POI)
    │   │   │   ├── AOIPin
    │   │   │   ├── GeoFencePin
    │   │   │   ├── TakeoffMapPoint
    │   │   │   └── BP_SPSimWorldOrigin
    │   │   └── obstacles/              # Obstacle blueprints
    │   │
    │   ├── Materials/                  # Materials and shaders
    │   │   ├── M_Clickable*            # Selection state materials
    │   │   ├── M_Trail                 # Drone trail material
    │   │   ├── M_InterestSplineDecal*  # Interest line materials
    │   │   ├── PostProcesses/          # Post-process effects
    │   │   └── UIMaterials/            # UI-specific materials
    │   │
    │   ├── Meshes/                     # 3D models
    │   │   ├── drone.uasset
    │   │   └── airplane1.uasset
    │   │
    │   ├── Collections/                # Asset collections
    │   ├── CesiumSettings/             # Cesium Ion configuration
    │   └── StarterContent/             # Default UE assets
    │
    ├── Config/                         # Engine configuration
    │   ├── DefaultEngine.ini           # Core engine settings
    │   ├── DefaultGame.ini             # Game-specific settings
    │   ├── DefaultInput.ini            # Input bindings
    │   └── DefaultEditor.ini           # Editor preferences
    │
    ├── Plugins/                        # Custom plugins (empty - uses marketplace)
    │
    ├── Binaries/                       # Compiled binaries (generated)
    ├── Intermediate/                   # Build intermediates (generated)
    ├── Saved/                          # Runtime data, logs (generated)
    ├── DerivedDataCache/               # Asset cache (generated)
    └── SkyPlexUserData/                # User preferences storage
```

## Critical Entry Points

| Entry Point | Path | Purpose |
|-------------|------|---------|
| **Game Mode** | `Content/Core/SP_GameModeBase` | Game initialization, manager spawning |
| **Game State** | `Content/Core/State/SP_GameState` | Central state hub |
| **Player Controller** | `Content/Core/Player/SP_PlayerController` | Input handling |
| **Main Level** | `Content/Basemap.umap` | Default map with Cesium terrain |
| **C++ Module** | `Source/SkyPlexC2/SkyPlexC2.cpp` | Module registration |

## Architecture Layers

```
┌─────────────────────────────────────────────────────────────┐
│                     UI / HUD Layer                          │
│  (Blueprints: Widgets, HUD, Input Actions)                  │
├─────────────────────────────────────────────────────────────┤
│                   Player Layer                              │
│  (SPPlayerController, SPCameraControllerCharacter)          │
├─────────────────────────────────────────────────────────────┤
│                 State Management Layer                      │
│  (SPGameState → Managers: Mission, Drone, Storage, etc.)    │
├─────────────────────────────────────────────────────────────┤
│                   Objects Layer                             │
│  (Drones, Interests, Obstacles, Geometry)                   │
├─────────────────────────────────────────────────────────────┤
│                   Persistence Layer                         │
│  (SQLiteUtility, MissionAssetsManager, GeoJsonManager)      │
├─────────────────────────────────────────────────────────────┤
│              External Communication Layer                   │
│  (WebSocket → CC Simulator, HTTP → Google APIs)             │
├─────────────────────────────────────────────────────────────┤
│                  Cesium / Unreal Engine                     │
│  (3D Tiles, Georeference, Rendering)                        │
└─────────────────────────────────────────────────────────────┘
```

## External Integration Points

| Integration | Protocol | Direction | Purpose |
|-------------|----------|-----------|---------|
| CC Simulator | WebSocket | Bidirectional | Drone commands & telemetry |
| Google APIs | HTTP/REST | Outbound | Maps, geolocation services |
| Cesium Ion | HTTP | Inbound | 3D terrain tiles |
| PX4/ROS2 (WSL) | DDS | Bidirectional | Flight simulation |
