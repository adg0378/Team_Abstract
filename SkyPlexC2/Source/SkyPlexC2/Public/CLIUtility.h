// Copyright (c) 2025 Synetos Aerospace
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "CLIUtility.generated.h"

/*
	UCLIUtility is meant to be managed primarily (if not entirely) by SPDroneManager, hence no teardown functionality.
 */

UCLASS()
class SKYPLEXC2_API UCLIUtility : public UObject
{
	GENERATED_BODY()

public:

	UCLIUtility();
	~UCLIUtility();

	// StdOut pipes also are routing StdErr
	// We aren't even using pipes currently though so these could be omitted when we have time
	struct WSLProc {
		FProcHandle procHandle;
		void* pipeProcStdOut;
		void* pipeProcStdIn;
	};

	struct CLIProcs {
		FProcHandle px4Handle;
		void* pipePX4StdOut;
		void* pipePX4StdIn;

		FProcHandle ros2Handle;
		void* pipeROS2StdOut;
		void* pipeROS2StdIn;
	};

	void StartXRCEAgent(WSLProc& Proc);

	void AddDroneProcesses(CLIProcs& newProcs, int32 droneID, int32 ros2Port, FVector OriginLonLatHeight, FVector HomeLonLatHeight, TFunction<void()> OnReady);

	static void KillWSLProc(WSLProc& Proc);
	static void KillCLIProcs(CLIProcs& Procs);
	static void KillProcHandle(FProcHandle& ProcHandle);

	WSLProc wslProc;
};