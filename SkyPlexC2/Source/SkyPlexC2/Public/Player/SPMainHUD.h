// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SPMainHUD.generated.h"

/**
 * Main HUD for SkyPlex
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPMainHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SpawnHUD();
	void SpawnHUD_Implementation();
};
