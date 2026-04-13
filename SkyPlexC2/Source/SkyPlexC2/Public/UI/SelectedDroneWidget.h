// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "SelectedDroneWidget.generated.h"

/**
 *
 */
UCLASS()
class SKYPLEXC2_API USelectedDroneWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock *DroneNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *SwarmIDText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *LatitudeText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *LongitudeText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *AltitudeText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock *SpeedText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar *BatteryProgressBar;

	UFUNCTION(BlueprintCallable)
	void UpdateViewWithDroneData();
};
