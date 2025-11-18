// Copyright (c) 2025 Synetos Aerospace


#include "Util/SPCLIUtility.h"
#include "GenericPlatform/GenericPlatformProcess.h"

USPCLIUtility::USPCLIUtility() {
	StartXRCEAgent(wslProc);
}

USPCLIUtility::~USPCLIUtility() {
	FPlatformProcess::CloseProc(wslProc.procHandle);
}

void USPCLIUtility::StartXRCEAgent(WSLProc& Proc) {
	FString command = "bash -lc 'MicroXRCEAgent udp4 --port 8888'";
	FString procURL = "C:/Windows/System32/wsl.exe";

	if (!FPlatformProcess::CreatePipe(Proc.pipeProcStdOut, Proc.pipeProcStdIn)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create pipes for MicroXRCEAgent."));
		return;
	}

	Proc.procHandle = FPlatformProcess::CreateProc(
		*procURL, *command,
		true, false, false,
		nullptr, 0, nullptr,
		Proc.pipeProcStdOut,
		Proc.pipeProcStdIn,
		Proc.pipeProcStdOut
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
	FString procURL = TEXT("C:/Windows/System32/wsl.exe");
	FString px4Command = FString::Printf(TEXT(
		"bash -lc 'env PX4_SYS_AUTOSTART=4001 PX4_SIM_MODEL=gz_x500 "
		"~/PX4-Autopilot/build/px4_sitl_default/bin/px4 -i %d'"
	), droneID);
	FString ros2Command = FString::Printf(TEXT(
		"bash -lc 'source /opt/ros/humble/setup.bash && "
		"source ~/cc-simulator/ccsim_ws/install/setup.bash && "
		"ros2 run c2comms comms_core -w -p %d'"
	), ros2Port);

	// Starting ROS2 
	if (!FPlatformProcess::CreatePipe(newProcs.pipeROS2StdOut, newProcs.pipeROS2StdIn)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create pipes for ROS2."));
		return;
	}
	newProcs.ros2Handle = FPlatformProcess::CreateProc(
		*procURL, *ros2Command,
		true, false, false,
		nullptr, 0, nullptr,
		newProcs.pipeROS2StdOut,
		newProcs.pipeROS2StdIn,
		newProcs.pipeROS2StdOut
	);
	if (!newProcs.ros2Handle.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create ROS2 process."));
		return;
	}
	// Starting PX4 
	if (!FPlatformProcess::CreatePipe(newProcs.pipePX4StdOut, newProcs.pipePX4StdIn)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create pipes for PX4."));
		return;
	}
	newProcs.px4Handle = FPlatformProcess::CreateProc(
		*procURL, *px4Command,
		true, false, false,
		nullptr, 0, nullptr,
		newProcs.pipePX4StdOut,
		newProcs.pipePX4StdIn,
		newProcs.pipePX4StdOut
	);
	if (!newProcs.px4Handle.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create PX4 process."));
		return;
	}
	// No (many) attempts using FPlatformProcess::ReadPipe() 
	// to listen for a PX4 ready signal through stdout have worked so far
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, OnReady]() mutable {
		FPlatformProcess::Sleep(8.0f);
		AsyncTask(ENamedThreads::GameThread, [OnReady]() { OnReady(); }
		);
	});
}