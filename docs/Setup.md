# Local WSL Environment (Ubuntu 22.04)  

SkyPlexC2 relies on a number of external processes ran in a [WSL2 Ubuntu 22.04](https://documentation.ubuntu.com/wsl/stable/howto/install-ubuntu-wsl2/) system within a Windows 10/11 host machine.

**ROS2 Humble**

Part of the communication with drones in SkyPlexC2 is facilitated via ROS2 workspace using ROS2 Nodes. In order to incorporate all ROS2 functionality, [ROS2 Humble](https://docs.ros.org/en/humble/index.html) installation should be either from
[docker container](https://github.com/osrf/docker_images/blob/master/ros/humble/ubuntu/jammy/ros-core/Dockerfile) or [building from source](https://docs.ros.org/en/humble/Installation/Alternatives.html). 


**Python Packages**

```
pip install ruff
pip install catkin_pkg
pip install pytest
pip install empy==3.3.4
pip install lark
pip install StrEnum
...TBD
```
Several integration dependencies must be set up as well in addition to the list of packages above.

**uXRCE-DDS Bridge**

This bridge exchanges internal PX4 uORB messages with external ROS2 publishers and subscribers via a client-agent architecture. 

Build the [Micro-XRCE-DDS-Agent](https://docs.px4.io/main/en/middleware/uxrce_dds.html) according to the instructions.

**PX4 and Gazebo Simulator**

The Gazebo simulator is included with the PX4 software.

Clone the v1.15 branch of the px4 repository (recursively, as there are submodules).

``` 
git clone https://github.com/PX4/PX4-Autopilot.git --recursive -b release/1.15
```

NOTE: Make sure to install the PX4 developer toolchain after cloning the repository:

```
bash ./PX4-Autopilot/Tools/setup/ubuntu.sh
```

NOTE: The configuration file src/modules/uxrce_dds_client/dds_topics.yaml describes which messages are bridged to DDS

Build and run [PX4/Gazebo](https://docs.px4.io/main/en/dev_setup/building_px4.html) according to its instructions. 

**QGroundControl**

QGroundControl is the default flight planning and flight control for PX4.

Download [QGroundControl](https://qgroundcontrol.com/) according to instructions.

**CC Simulator**

CC Simulator communicates with SkyPlexC2 via web sockets for commanding and telemetry reading.

SkyPlexC2 currently only supports PX4 whereas CC Simulator has a PX4 and ArduPilot setup process. Clone it and follow the [CC Simulator PX4 Setup Guide](https://github.com/prius-intelli/cc-simulator/blob/develop/config/PX4Setup.md) per its instructions. Many of its steps have already been completed above.

# For Development

In addition to setting up the local WSL environment detailed above, developers will need Unreal Engine 5 and Visual Studio 2022 installed.

**Unreal Engine 5.5.4**

To install Unreal Engine, you first must install the Epic Games Launcher. Follow [the guide](https://www.unrealengine.com/en-US/download) and make sure version 5.5.4 is installed. 

In addition, several plugins must be installed:

* Cesium for Unreal
* Fab UE Plugin
* VaRest - Rest API with Blueprints
* Quixel Bridge
* Runtime Svg System

**Visual Studio Community 2022**

By default, Unreal Engine projects are configured to use Visual Studio for the IDE. 

In addition to the IDE, several tools and workloads must be installed as well. Follow [the guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine) for a complete Visual Studio - Unreal Engine 5 set up.
