// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "CesiumOriginShiftComponent.h"
#include "CesiumSpringArmComponent.generated.h"

/**
 * Custom spring arm component that permits lag with cesium origin shift
 */
UCLASS(ClassGroup = "Cesium", Meta = (BlueprintSpawnableComponent))
class SKYPLEXC2_API UCesiumSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

	/** Updates the desired arm location, calling BlendLocations to do the actual blending if a trace is done */
	void UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime) override;

private:
	/** Temporary variables when using camera lag, to record previous camera position */
	FVector PreviousDesiredLocECEF;

	void ResolveOriginShift();

	// The origin shift attached to the same Actor as this component. Don't
	// save/load or copy this. It is set in BeginPlay and OnRegister.
	UPROPERTY(
		Category = "Cesium",
		EditAnywhere, BlueprintReadWrite,
		Meta = (AllowPrivateAccess))
	UCesiumOriginShiftComponent* OriginShift;

};
