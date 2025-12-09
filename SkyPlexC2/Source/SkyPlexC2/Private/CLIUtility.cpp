// Copyright (c) 2025 Synetos Aerospace

#include "CLIUtility.h"
#include "State/SPGameState.h"
#include "State/PreferencesManager.h"
#include "SPLogger.h"
#include "SPUtility.h"

UCLIUtility::UCLIUtility() {
}

UCLIUtility::~UCLIUtility() {
}

void UCLIUtility::KillProcHandle(FProcHandle& ProcHandle) {
	if (ProcHandle.IsValid()) {
		FPlatformProcess::TerminateProc(ProcHandle, true);
		FPlatformProcess::CloseProc(ProcHandle);
		ProcHandle.Reset();
	}
}

void UCLIUtility::KillWSLProc(WSLProc& Proc) {
	KillProcHandle(Proc.procHandle);
}
void UCLIUtility::KillCLIProcs(CLIProcs& Procs) {
	KillProcHandle(Procs.px4Handle);
	KillProcHandle(Procs.ros2Handle);
}

void UCLIUtility::StartXRCEAgent(WSLProc& Proc) {
	FString command = "bash -lc 'MicroXRCEAgent udp4 --port 8888'";
	FString procURL = "C:/Windows/System32/wsl.exe";
	ASPGameState* GameState = USPUtility::GetSPGameState(this);

	if (!FPlatformProcess::CreatePipe(Proc.pipeProcStdOut, Proc.pipeProcStdIn)) {
		GameState->loggerRef->Error(TEXT("Failed to instantiate MicroXRCEAgent pipes"), TEXT("CliUtility"), true);
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
		GameState->loggerRef->Error(TEXT("Failed to create MicroXRCEAgent process"), TEXT("CliUtility"), true);
		return;
	}
	else {
		GameState->loggerRef->Info(TEXT("MicroXRCEAgent Process created successfully"), TEXT("CliUtility"));
		return;
	}
}

void UCLIUtility::AddDroneProcesses(CLIProcs& newProcs, int32 droneID, int32 ros2Port, FVector OriginLonLatHeight, FVector HomeLonLatHeight, TFunction<void()> OnReady) {
	ASPGameState* GameState = USPUtility::GetSPGameState(this);
	FCCSimPreferencesStruct Prefs = GameState->PreferencesManager->GetPreferences().CCSimPreferences;

	FVector ENU = UGeographicUtility::LatLonToLocal(GameState->CesiumGeoreference, OriginLonLatHeight, HomeLonLatHeight);

	FString Standalone = droneID == 0 ? TEXT("") : TEXT("PX4_GZ_STANDALONE=1 ");

	FString procURL = TEXT("C:/Windows/System32/wsl.exe");
	FString px4Command = FString::Printf(
		TEXT(
			"bash -lc '%s; env %sPX4_GZ_MODEL_POSE=\"%lf %lf 0 0 0 0\" GZ_SIM_PORT_OFFSET=%d EKF2_HOM_LAT=%lf EKF2_HOM_LON=%lf EKF2_HOM_ALT=0.0 PX4_SYS_AUTOSTART=4001 PX4_SIM_MODEL=gz_x500 "
		"%s/build/px4_sitl_default/bin/px4 -i %d'"
		),
		Prefs.PX4PreCommand.IsEmpty() ? *FString(TEXT("echo")) : *Prefs.PX4PreCommand,
		*Standalone,
		ENU.X,
		ENU.Y,
		droneID,
		HomeLonLatHeight.Y,
		HomeLonLatHeight.X,
		*Prefs.PX4AutopilotPath,
		droneID
	);

	UE_LOG(LogTemp, Log, TEXT("Using px4 command: %s"), *px4Command);

	FString ros2Command = FString::Printf(TEXT(
		"bash -lc 'source %s && "
		"source %s/ccsim_ws/install/setup.bash && "
		"ros2 run c2comms comms_core -w -p %d --ros-args --param drone_id:=%d'"
	), *Prefs.ROS2Path, *Prefs.CCSimPath, ros2Port, droneID);

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