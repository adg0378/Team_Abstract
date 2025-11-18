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

	struct WSLProc {
		FProcHandle procHandle;
		void* pipeProcStdOut;
		void* pipeProcStdIn;
	};

	struct CLIProcs {
		FProcHandle px4Handle;
		FProcHandle ros2Handle;
	};

	void StartXRCEAgent(WSLProc& Proc);

	void AddDroneProcesses(CLIProcs& newProcs, int32 droneID, int32 ros2Port, TFunction<void()> OnReady);

	void KillAllWSL();

private:

	WSLProc wslProc;
};
