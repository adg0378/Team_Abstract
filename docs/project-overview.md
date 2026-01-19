# SkyPlexC2 Project Overview

> Generated: 2026-01-19

## Project Summary

| Attribute | Value |
|-----------|-------|
| **Name** | SkyPlexC2 |
| **Type** | Drone Command & Control (C2) Visualization System |
| **Engine** | Unreal Engine 5.5 |
| **Language** | C++ / Blueprints |
| **Repository** | Monolith |
| **License** | Proprietary (Synetos Aerospace) |

## Purpose

SkyPlexC2 is a specialized visualization and control application for drone operations. Unlike traditional games, it uses Unreal Engine as a rendering platform for:

- **3D Geospatial Visualization** - Real-time terrain and satellite imagery via Cesium
- **Mission Planning** - Create and manage drone flight missions
- **Fleet Management** - Monitor and control multiple drones
- **Obstacle Awareness** - Track ADS-B aircraft and FAA obstacles
- **Simulation Integration** - Connect to PX4/Gazebo for realistic flight simulation

## Key Features

### Mission Management
- Create missions with Points of Interest (POIs)
- Define Areas of Interest (AOIs) as polygons
- Set GeoFences (no-fly zones)
- Designate takeoff/landing points
- Visualize mission paths in 3D

### Drone Control
- Real-time drone telemetry display
- Command interface (arm, takeoff, land, waypoints)
- Multiple drone support (swarms)
- WebSocket communication with simulators

### Visualization
- Cesium 3D terrain tiles
- Multiple camera modes (first-person, isometric, top-down)
- Interactive map markers
- Flight path trails

### Data Persistence
- SQLite database for missions and interests
- GeoJSON import/export
- User preferences storage

## Technology Highlights

| Component | Technology |
|-----------|-----------|
| 3D Engine | Unreal Engine 5.5 |
| Geospatial | Cesium for Unreal |
| Database | SQLite (embedded) |
| Communication | WebSockets, HTTP |
| Simulation | PX4/ROS2/Gazebo (WSL2) |
| Build System | Unreal Build Tool |
| CI/CD | GitLab CI |

## Architecture Style

**Manager-based State Management** with event-driven UI updates:

```
ASPGameState (Hub)
    ├── USPMissionManager → Missions & Interests
    ├── USPDroneManager → Drone Fleet
    ├── USPStorageManager → Persistence
    ├── USPObstacleManager → Obstacles
    └── USPWorldManager → Environment
```

## Target Platforms

| Platform | Status |
|----------|--------|
| Windows 10/11 | Primary |
| macOS | Secondary (in .uproject) |
| Linux | Via Cesium support |

## Quick Links

- [Architecture](./architecture.md) - Detailed system architecture
- [Development Guide](./development-guide.md) - Setup and build instructions
- [Source Tree](./source-tree-analysis.md) - Codebase structure
- [Data Models](./data-models.md) - Database schema
- [Setup Guide](./Setup.md) - Environment setup
