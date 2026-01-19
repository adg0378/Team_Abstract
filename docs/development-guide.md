# SkyPlexC2 Development Guide

> Generated: 2026-01-19 | Source: Setup.md, build configuration

## Prerequisites

### Windows Host Machine

| Requirement | Version | Purpose |
|-------------|---------|---------|
| **Windows** | 10/11 | Host OS |
| **Unreal Engine** | 5.5.4 | Game engine |
| **Visual Studio** | 2022 Community | IDE and compiler |
| **Epic Games Launcher** | Latest | UE installation |
| **WSL2** | Ubuntu 22.04 | Simulation environment |

### Visual Studio Workloads

Install via Visual Studio Installer:
- Game development with C++
- Desktop development with C++
- .NET desktop development (for UE tools)

See: [Epic's VS Setup Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine)

### Unreal Engine Plugins

Install via Epic Games Launcher or Fab:
- **Cesium for Unreal** - Geospatial 3D tiles
- **Fab UE Plugin** - Asset marketplace
- **VaRest** - REST API with Blueprints
- **Quixel Bridge** - Megascans assets
- **Runtime Svg System** - Vector graphics (optional)

## WSL2 Environment Setup

### Ubuntu 22.04 Installation

```bash
# Install WSL2 with Ubuntu 22.04
wsl --install -d Ubuntu-22.04
```

### ROS2 Humble

Install via Docker or build from source:
- [Docker installation](https://github.com/osrf/docker_images/blob/master/ros/humble/ubuntu/jammy/ros-core/Dockerfile)
- [Build from source](https://docs.ros.org/en/humble/Installation/Alternatives.html)

### Python Packages

```bash
pip install ruff
pip install catkin_pkg
pip install pytest
pip install empy==3.3.4
pip install lark
pip install StrEnum
# Additional packages TBD
```

### Micro-XRCE-DDS Agent

Bridge for PX4 uORB ↔ ROS2 communication:

```bash
# Follow instructions at:
# https://docs.px4.io/main/en/middleware/uxrce_dds.html
```

### PX4 Autopilot + Gazebo

```bash
# Clone PX4 v1.15
git clone https://github.com/PX4/PX4-Autopilot.git --recursive -b release/1.15

# Install developer toolchain
bash ./PX4-Autopilot/Tools/setup/ubuntu.sh
```

**Note:** Configuration file `src/modules/uxrce_dds_client/dds_topics.yaml` defines bridged messages.

Build and run: [PX4 Building Guide](https://docs.px4.io/main/en/dev_setup/building_px4.html)

### QGroundControl

Download from: [qgroundcontrol.com](https://qgroundcontrol.com/)

### CC Simulator

```bash
# Clone and follow PX4 setup guide:
# https://github.com/prius-intelli/cc-simulator/blob/develop/config/PX4Setup.md
```

## Project Setup

### Opening the Project

1. Launch Epic Games Launcher
2. Open Unreal Engine 5.5.4
3. Browse to `SkyPlexC2/SkyPlexC2.uproject`
4. Wait for shader compilation (first launch)

### Generating Visual Studio Solution

```bash
# From project root
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="SkyPlexC2/SkyPlexC2.uproject" -game -rocket -progress
```

Or use: **File → Generate Visual Studio Project** in UE Editor

### Building from IDE

1. Open `SkyPlexC2.sln` in Visual Studio
2. Set configuration: `Development Editor | Win64`
3. Build solution (F7)

## Build Commands

### Local Development Build

```bash
# Generate project files
"C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\GenerateProjectFiles.bat" "SkyPlexC2\SkyPlexC2.uproject" -game

# Build Development Editor
"C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat" SkyPlexC2Editor Win64 Development "SkyPlexC2\SkyPlexC2.uproject"
```

### Shipping Build (CI/CD)

```batch
@echo SkyplexC2 start build

set UPROJECT=%CI_PROJECT_DIR%\SkyPlexC2\SkyPlexC2.uproject
set ARCHIVE_DIR=%CI_PROJECT_DIR%\build

call "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\RunUAT.bat" ^
    BuildCookRun ^
    -project="%UPROJECT%" ^
    -platform=Win64 ^
    -clientconfig=Shipping ^
    -alltimings ^
    -build -cook -stage -package -archive ^
    -archivedirectory="%ARCHIVE_DIR%" ^
    -pak -prereqs -unattended
```

### Build Outputs

| Configuration | Output Location | Purpose |
|---------------|-----------------|---------|
| Development Editor | `Binaries/Win64/` | Local development |
| Shipping | `build/` (CI) | Production release |

## Running the Application

### Editor Mode

1. Open project in UE Editor
2. Press **Play** (Alt+P) or **Play in Standalone** (Alt+S)

### Standalone

```bash
# After packaging
SkyPlexC2/Binaries/Win64/SkyPlexC2.exe
```

### With Simulation Stack

1. **Start WSL2 services:**
   ```bash
   # Terminal 1: Start PX4 SITL
   cd PX4-Autopilot
   make px4_sitl gazebo

   # Terminal 2: Start DDS Agent
   MicroXRCEAgent udp4 -p 8888

   # Terminal 3: Start CC Simulator (if using)
   # Follow CC Simulator instructions
   ```

2. **Launch SkyPlexC2** in Editor or Standalone

3. **Connect QGroundControl** (optional)

## Testing

### Automated Tests

Located in: `Source/SkyPlexC2/Private/Test/`

```bash
# Run from UE Editor: Window → Test Automation
# Or via command line:
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
    "SkyPlexC2\SkyPlexC2.uproject" ^
    -ExecCmds="Automation RunTests SkyPlexC2" ^
    -unattended -nopause -nullrhi
```

### Test Files

| File | Type | Purpose |
|------|------|---------|
| `SpecExample.spec.cpp` | Spec Test | Example unit tests |

## CI/CD Pipeline

### GitLab CI Configuration

```yaml
# .gitlab-ci.yml
build-job:
  stage: build
  script:
    - cmd /c mkdir build
    - .\buildme.bat
  tags:
    - unreal
  artifacts:
    name: skyplexc2
    expose_as: 'SkyplexC2 build'
    paths:
      - build
```

### Requirements

- GitLab Runner with `unreal` tag
- Unreal Engine 5.5 installed on runner
- Windows build environment

### Artifacts

| Artifact | Path | Contents |
|----------|------|----------|
| Build Package | `build/` | Packaged Win64 Shipping build |

## Configuration Files

| File | Purpose |
|------|---------|
| `Config/DefaultEngine.ini` | Engine settings (rendering, physics) |
| `Config/DefaultGame.ini` | Game settings (maps, game mode) |
| `Config/DefaultInput.ini` | Input bindings |
| `Config/DefaultEditor.ini` | Editor preferences |
| `SkyPlexC2.uproject` | Project configuration, plugins |
| `Source/SkyPlexC2/SkyPlexC2.Build.cs` | C++ module dependencies |

## Environment Variables

| Variable | Purpose | Example |
|----------|---------|---------|
| `CI_PROJECT_DIR` | CI build root | Set by GitLab Runner |
| `UE_ROOT` | Unreal Engine install | `C:\Program Files\Epic Games\UE_5.5` |

## Troubleshooting

### Common Issues

| Issue | Solution |
|-------|----------|
| SQLite DLL not found | Ensure `ThirdParty/SQLite/Win64/sqlite3.dll` exists |
| Cesium tiles not loading | Check Cesium Ion token in project settings |
| WebSocket connection failed | Verify CC Simulator is running and port is correct |
| Shader compilation slow | First launch compiles ~10k shaders; be patient |
| PX4 not connecting | Check DDS agent is running, verify `dds_topics.yaml` |

### Logs

| Log Location | Purpose |
|--------------|---------|
| `Saved/Logs/SkyPlexC2.log` | Game runtime logs |
| `Saved/Crashes/` | Crash dumps |
| Custom SPLogger | In-game logging system |
