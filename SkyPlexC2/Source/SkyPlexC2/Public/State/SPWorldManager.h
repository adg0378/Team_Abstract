// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "State/SPManager.h"
#include "Player/SPCameraControllerCharacter.h"
#include "Player/SPCameraInterface.h"
#include "SPWorldManager.generated.h"

// forward declarations
class ACesiumSunSky;

/**
 * Manages world parameters like sun and sky position
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API USPWorldManager : public USPManager
{
	GENERATED_BODY()

public:
	void Setup_Implementation(bool& outSuccess) override;
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
	const float GetDevicePitch() const;

	UFUNCTION(BlueprintCallable)
	const float GetDeviceRoll() const;

	UFUNCTION(BlueprintCallable)
	const float GetDeviceYaw() const;

	UFUNCTION(BlueprintCallable)
	const FString GetDeviceName() const;

	UFUNCTION(BlueprintCallable)
	void CullObjects(FVector OriginPosUE = FVector::ZeroVector, bool FromFlyTo = false);

private:
	const FString LOG_ORIGIN = TEXT("WorldManager");

	UPROPERTY()
	TObjectPtr<ACesiumSunSky> SunSky;

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
