// Copyright (c) 2025 Synetos Aerospace


#include "Util/SPCLIUtility.h"
#include "GenericPlatform/GenericPlatformProcess.h"

USPCLIUtility::USPCLIUtility() {
}

USPCLIUtility::~USPCLIUtility() {
}

void USPCLIUtility::StartXRCEAgent(WSLProc& Proc) {
	FString command = "bash -lc 'MicroXRCEAgent udp4 --port 8888'";
	FString procURL = "wsl.exe";

	Proc.procHandle = FPlatformProcess::CreateProc(
		*procURL, *command,
		true, false, false,
		nullptr, 0, nullptr,
		nullptr, nullptr, nullptr
	);

	if (!Proc.procHandle.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create MicroXRCEAgent process."));
		return;
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("MicroXRCEAgent Process created successfully."));
		return;
	}
}

void USPCLIUtility::AddDroneProcesses(CLIProcs& newProcs, int32 droneID, int32 ros2Port, TFunction<void()> OnReady) {
	FString procURL = TEXT("wsl.exe");
	FString px4Command = FString::Printf(TEXT(
		"bash -lc \"env PX4_SYS_AUTOSTART=4001 PX4_SIM_MODEL=gz_x500 "
		"~/PX4-Autopilot/build/px4_sitl_default/bin/px4 -i %d\""
	), droneID);
	FString ros2Command = FString::Printf(TEXT(
		"bash -lc \"source /opt/ros/humble/setup.bash && "
		"source ~/cc-simulator/ccsim_ws/install/setup.bash && "
		"ros2 run c2comms comms_core -w -p %d\""
	), ros2Port);

	// Starting ROS2 
	newProcs.ros2Handle = FPlatformProcess::CreateProc(
		*procURL, *ros2Command,
		true, false, false,
		nullptr, 0, nullptr,
		nullptr, nullptr, nullptr
	);
	if (!newProcs.ros2Handle.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create ROS2 process."));
		return;
	}
	// Starting PX4 
	newProcs.px4Handle = FPlatformProcess::CreateProc(
		*procURL, *px4Command,
		true, false, false,
		nullptr, 0, nullptr,
		nullptr, nullptr, nullptr
	);
	if (!newProcs.px4Handle.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create PX4 process."));
		return;
	}
	/* No (many) attempts using FPlatformProcess::ReadPipe()
	/* to listen for a PX4 ready signal through stdout have worked so far.
	/* This needs to be revisited. */
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, OnReady]() mutable {
		FPlatformProcess::Sleep(8.0f);
		AsyncTask(ENamedThreads::GameThread, [OnReady]() { OnReady(); }
		);
		});
}

void USPCLIUtility::KillAllWSL() {
	FString procURL = "wsl.exe";
	FString procParams = "--shutdown";

	FProcHandle handle = FPlatformProcess::CreateProc(
		*procURL, *procParams,
		true, false, false,
		nullptr, 0, nullptr,
		nullptr, nullptr, nullptr
	);

	if (!handle.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("Failed to shutdown WSL processes."));
	}
	UE_LOG(LogTemp, Log, TEXT("WSL processes terminated."));
}