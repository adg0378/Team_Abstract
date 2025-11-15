// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SPAssetUtility.generated.h"

/**
 * 
 */
UCLASS()
class SKYPLEXC2_API USPAssetUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static constexpr ECollisionChannel LandscapeTraceChannel = ECollisionChannel::ECC_GameTraceChannel1;
};
