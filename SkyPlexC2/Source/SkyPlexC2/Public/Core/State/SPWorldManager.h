// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Core/State/SPManagerBase.h"
#include "Delegates/DelegateCombinations.h"
#include "Core/Player/SPCameraInterface.h"
#include "SPWorldManager.generated.h"

// Broadcasted when camera type index is changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCameraTypeChangedDelegate, uint8, NewIndex);

/**
 * Manages timezone, object culling, camera, window, etc.
 */
UCLASS()
class SKYPLEXC2_API USPWorldManager : public USPManagerBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FCameraTypeChangedDelegate OnCameraTypeChanged;

	void Setup_Implementation() override;
	void ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) override;

	void PostSetup() override;
	void PreTeardown() override;

	UFUNCTION(BlueprintCallable)
	void UpdateTimezone(float Longitude);

	UFUNCTION(BlueprintCallable)
	void SetActiveCamera(const TScriptInterface<USPCameraInterface>& InActiveCamera);

	UFUNCTION(BlueprintCallable)
	const float GetZoomPercentage() const;

	UFUNCTION(BlueprintCallable)
	const FVector GetCameraLocation() const;

	UFUNCTION(BlueprintCallable)
	const FRotator GetCameraRotation() const;

	UFUNCTION(BlueprintCallable)
	void SetCameraTypeFromIndex(uint8 Index);

	UFUNCTION(BlueprintCallable)
	void CullObjects(FVector OriginPosUE = FVector::ZeroVector);

private:
	const FString LOG_ORIGIN = TEXT("WorldManager");

	UPROPERTY()
	TObjectPtr<class ACesiumSunSky> SunSky;

	UPROPERTY()
	TScriptInterface<USPCameraInterface> ActiveCamera;

	UPROPERTY()
	FTimerHandle ObjectDistanceCullTimerHandle;
	const int32 ObjectDistanceCullTime = 6.0f;
	const float MaximumDrawDistance = 5000000.0f;

	void StartObjectDistanceCullTimer();
	void StopObjectDistanceCullTimer();
	void TimerCallCullObjects();
};

