# SkyPlexC2 Component Inventory

> Generated: 2026-01-19

## Overview

This document catalogs the UI components, Blueprints, and visual assets used in SkyPlexC2.

## Player & Camera Components

### C++ Classes

| Class | File | Purpose |
|-------|------|---------|
| `ASPPlayerController` | `Player/SPPlayerController.h` | Input handling, selection management |
| `ASPCameraControllerCharacter` | `Player/SPCameraControllerCharacter.h` | Camera pawn with movement |
| `ASPMainHUD` | `Player/SPMainHUD.h` | Main HUD class (Blueprint-extensible) |
| `USPCameraInterface` | `Player/SPCameraInterface.h` | Camera mode interface |
| `UCesiumSpringArmComponent` | `Player/CesiumSpringArmComponent.h` | Geospatial camera arm |

### Blueprints

| Blueprint | Location | Purpose |
|-----------|----------|---------|
| `SP_PlayerController` | `Content/Core/Player/` | Player controller instance |
| `SP_CameraControllerCharacter` | `Content/Core/Player/` | Camera pawn instance |

## Input System

### Input Mapping Context

| Asset | Location | Purpose |
|-------|----------|---------|
| `IMC_Default` | `Content/Core/Player/Input/` | Default input mapping |
| `PMIC` | `Content/Core/Player/Input/` | Player mapping input context |

### Input Actions

| Action | Binding | Purpose |
|--------|---------|---------|
| `IA_Movement` | WASD | Camera translation |
| `IA_CameraOrbit` | RMB + Drag | Orbit around point |
| `IA_CameraPitch` | Middle Mouse | Pitch adjustment |
| `IA_CameraZoomIn` | Scroll Up | Zoom in |
| `IA_CameraZoomOut` | Scroll Down | Zoom out |
| `IA_DoAction` | LMB | Primary action (select/place) |
| `IA_Delete` | Delete | Delete selected |
| `IA_Deselect` | Escape | Clear selection |
| `IA_MultipleSelect` | Ctrl | Multi-select modifier |
| `IA_Shift` | Shift | Modifier key |
| `IA_FreeCamera` | - | Free camera mode |
| `IA_SwapToFirstPersonCamera` | 1 | First-person view |
| `IA_SwapToIsometricCamera` | 2 | Isometric view |
| `IA_SwapToTopdownCamera` | 3 | Top-down view |
| `IA_ToggleControlsMenu` | ? | Show controls help |
| `IA_ClickOutsideWidget` | - | Deselect on empty click |
| `IA_ShowCursorInFirstPerson` | - | Cursor visibility toggle |
| `IA_MouseX` | Mouse X | Mouse X axis |
| `IA_MouseY` | Mouse Y | Mouse Y axis |
| `IA_ZeroNumKey` | 0 | Numpad zero |

## Game State Components

### Manager Blueprints

| Blueprint | Location | C++ Parent | Purpose |
|-----------|----------|------------|---------|
| `SP_GameState` | `Content/Core/State/` | `ASPGameState` | Central game state |
| `BP_SPDroneManager` | `Content/Core/State/Managers/` | `USPDroneManager` | Drone fleet management |
| `BP_SPMissionManager` | `Content/Core/State/Managers/` | `USPMissionManager` | Mission management |
| `BP_SPObstacleManager` | `Content/Core/State/Managers/` | `USPObstacleManager` | Obstacle tracking |
| `BP_SimulationManager` | `Content/Core/State/Managers/` | `USPSimulationManager` | Simulation state |

### Core Blueprints

| Blueprint | Location | Purpose |
|-----------|----------|---------|
| `SP_GameModeBase` | `Content/Core/` | Game mode with manager spawning |
| `SP_GameModeBaseUtil` | `Content/Core/` | Game mode utilities |
| `SP_CesiumUtil` | `Content/Core/` | Cesium helper functions |

## Object Components

### Drone Blueprints

| Blueprint | Location | C++ Parent | Purpose |
|-----------|----------|------------|---------|
| `BP_BasicDrone` | `Content/Objects/drone/` | `ABasicDrone` | Base drone actor |
| `BP_CCSimDrone` | `Content/Objects/drone/` | `ACCSimDrone` | CC Simulator drone |

### Interest Blueprints

| Blueprint | Location | Purpose |
|-----------|----------|---------|
| `BasicMapPoint` | `Content/Objects/Interests/` | POI map marker |
| `AOIPin` | `Content/Objects/Interests/` | AOI vertex marker |
| `GeoFencePin` | `Content/Objects/Interests/` | GeoFence vertex marker |
| `TakeoffMapPoint` | `Content/Objects/Interests/` | Takeoff point marker |
| `BP_SPSimWorldOrigin` | `Content/Objects/Interests/` | Simulation origin marker |
| `FloatingMonolith` | `Content/Objects/Interests/` | Visual marker asset |

### Geometry Blueprints

| Blueprint | Location | C++ Parent | Purpose |
|-----------|----------|------------|---------|
| `BP_Polygon` | `Content/Objects/Geometry/` | `APolygon` | Area polygon |
| `Red_Polygon` | `Content/Objects/Geometry/` | `APolygon` | GeoFence polygon |
| `BP_InterestLines` | `Content/Objects/Geometry/` | `AInterestLines` | Mission path lines |

### Widget Blueprints

| Widget | Location | Purpose |
|--------|----------|---------|
| `WB_MapPoint` | `Content/Objects/Interests/` | Map marker UI |
| `WB_TakeoffMapPoint` | `Content/Objects/Interests/` | Takeoff marker UI |
| `WB_SimWorldOrigin` | `Content/Objects/Interests/` | Sim origin marker UI |

## Materials

### Interaction Materials

| Material | Purpose | States |
|----------|---------|--------|
| `M_Clickable` | Default clickable state | Base |
| `M_ClickableHover` | Mouse hover state | Highlighted |
| `M_ClickableSelected` | Selected state | Blue tint |
| `M_ClickableInvalid` | Invalid placement | Red tint |
| `M_ClickablePaused` | Paused state | Gray |
| `M_ClickableRunning` | Active/running | Green |
| `M_ClickableUnassigned` | Unassigned to mission | Yellow |

### Interest Materials

| Material | Purpose |
|----------|---------|
| `M_InterestSplineDecalBase` | Base interest line |
| `M_InterestSplineDecalAOI` | AOI boundary line |
| `M_InterestSplineDecalGeoFence` | GeoFence boundary line |
| `MI_AOI_Tint` | AOI area fill |
| `MLB_AOI_Tint_Pulsing` | Pulsing AOI effect |
| `MLB_AOI_Tint_Waves` | Wave AOI effect |
| `MLB_NoFly_Tint_Pulsing` | GeoFence pulsing |
| `MLB_POI_Tint_Pulsing` | POI pulsing effect |

### Drone Materials

| Material | Purpose |
|----------|---------|
| `M_Trail` | Drone flight trail |
| `M_Circle` | Circular markers |

### UI Materials

| Material | Location | Purpose |
|----------|----------|---------|
| `M_ButtonTint` | `Materials/UIMaterials/` | Button styling |
| `MI_ButtonTint` | `Materials/UIMaterials/` | Button instance |
| `M_UIGradient` | `Materials/UIMaterials/` | Gradient backgrounds |
| `M_Grid` | `Materials/UIMaterials/` | Grid overlay |
| `M_GridSmall` | `Materials/UIMaterials/` | Small grid |
| `M_OutlineGradient` | `Materials/UIMaterials/` | Outline effects |
| `M_CircularClip` | `Materials/UIMaterials/` | Circular mask |
| `M_circularMask` | `Materials/UIMaterials/` | Circular mask alt |
| `M_SimNameBackground` | `Materials/UIMaterials/` | Simulation name BG |
| `M_CurvedHUDFX` | `Materials/UIMaterials/` | HUD curve effect |
| `Radial` | `Materials/UIMaterials/` | Radial gradient |

### Post-Process Materials

| Material | Location | Purpose |
|----------|----------|---------|
| `SP_HighlightMaterial` | `Materials/PostProcesses/` | Selection highlight |

## Meshes

| Mesh | Location | Purpose |
|------|----------|---------|
| `drone` | `Content/Meshes/` | Drone 3D model |
| `airplane1` | `Content/Meshes/` | Aircraft 3D model |

## Configuration Assets

### Cesium

| Asset | Location | Purpose |
|-------|----------|---------|
| `CesiumIonSaaS` | `Content/CesiumSettings/CesiumIonServers/` | Cesium Ion server config |

### Collections

| Collection | Purpose |
|------------|---------|
| `Content/Collections/` | Asset organization |

## Interactable Hierarchy

```
AInteractable (Base selectable)
    │
    ├── APlaceable (Terrain-placeable)
    │       │
    │       └── APlaceablePoint (Single point)
    │               ├── BasicMapPoint (POI)
    │               ├── AOIPin (AOI vertex)
    │               ├── GeoFencePin (GeoFence vertex)
    │               └── TakeoffMapPoint (Takeoff)
    │
    ├── APolygon (Multi-point area)
    │       ├── BP_Polygon (AOI)
    │       └── Red_Polygon (GeoFence)
    │
    └── AStateboundInteractable (State-linked)

ABasicDrone (Drone base)
    ├── ACCSimDrone (CC Simulator)
    └── ADemoSimDrone (Demo/test)
```

## Selection States

| State | Material | Color | Trigger |
|-------|----------|-------|---------|
| Default | `M_Clickable` | White | Initial |
| Hover | `M_ClickableHover` | Light Blue | Mouse over |
| Selected | `M_ClickableSelected` | Blue | Click |
| Invalid | `M_ClickableInvalid` | Red | Invalid action |
| Running | `M_ClickableRunning` | Green | Active mission |
| Paused | `M_ClickablePaused` | Gray | Paused |
| Unassigned | `M_ClickableUnassigned` | Yellow | No mission |
