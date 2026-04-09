// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SelectedDroneWidget.generated.h"

/**
 *
 */
UCLASS()
class SKYPLEXC2_API USelectedDroneWidget : public UUserWidget
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	void UpdateViewWithDroneData();
};
