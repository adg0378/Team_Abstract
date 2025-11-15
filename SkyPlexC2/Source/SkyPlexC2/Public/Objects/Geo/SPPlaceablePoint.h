// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/SPPlaceable.h"
#include "Delegates/DelegateCombinations.h"
#include "SPPlaceablePoint.generated.h"

// Broadcasted if point is clicked while in click me mode
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FClickedInClickMeDelegate);

/**
 * A basic map point
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPPlaceablePoint : public ASPPlaceable
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FClickedInClickMeDelegate OnClickMeClick;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ToggleClickMeMode(bool IsClickMe);
	void ToggleClickMeMode_Implementation(bool IsClickMe);

	virtual void OnClick(UPrimitiveComponent* touchedComponent, FKey buttonPressed) override;

private:
	bool InClickMeMode = false;
};
