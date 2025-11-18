// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SPCLIUtility.generated.h"

/**
 * 
 */
UCLASS()
class SKYPLEXC2_API USPCLIUtility : public UObject
{
	GENERATED_BODY()
	
public:

	USPCLIUtility();
	~USPCLIUtility();

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

	void AddDroneProcesses(CLIProcs& newProcs, int32 droneID, int32 ros2Port, TFunction<void()> OnReady);

private:

	WSLProc wslProc;
};
